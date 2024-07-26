#include <getopt.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>


#include "server.h"
#include "data_structures.h"
#include "protocol.h"
#include "helpers.h"

#include <pthread.h>
#include <semaphore.h>
#include <math.h>

pthread_t tid;

list_t* auction_list;
list_t* user_list;
list_t* job_queue;
int job_num;
int auction_id = 1;

//const char exit_str[] = "exit";

char buffer[BUFFER_SIZE];
pthread_mutex_t buffer_lock;

// Lock for the auction ids
pthread_mutex_t auction_id_lock;

// Read/write lock for the user list
pthread_mutex_t user_w_lock;
int user_readcnt;
pthread_mutex_t user_r_lock;

// Read/write lock for the auction list
pthread_mutex_t auction_r_lock;
int auction_readcnt;
pthread_mutex_t auction_w_lock;
 
// Producer/consumer semaphore for the jobs
sem_t job_sem;
pthread_mutex_t job_lock;  

//int total_num_msg = 0;

int listen_fd;

void sigint_handler(int sig)
{
    deleteList(auction_list);
    deleteList(user_list);
    deleteList(job_queue);

    
    printf("\nShutting down server\n");
    close(listen_fd);
    exit(0);
}

int server_init(int server_port){
    int sockfd;
    struct sockaddr_in servaddr;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("Socket successfully created\n");

    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(server_port);

    int opt = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, (char *)&opt, sizeof(opt))<0)
    {
	perror("setsockopt");exit(EXIT_FAILURE);
    }

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("Socket successfully binded\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 1)) != 0) {
        printf("Listen failed\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("Server listening on port: %d.. Waiting for connection\n", server_port);

    return sockfd;
}

//Function running in thread
void *process_client(void* clientfd_ptr){
    int client_fd = *(int *)clientfd_ptr;
    free(clientfd_ptr);
    int received_size;
    fd_set read_fds;

    int retval;
    while(1){
        FD_ZERO(&read_fds);
        FD_SET(client_fd, &read_fds);
        retval = select(client_fd + 1, &read_fds, NULL, NULL, NULL);
        if (retval!=1 && !FD_ISSET(client_fd, &read_fds)){
            printf("Error with select() function\n");
            break;
        }
        
        // Wait for commant input
        petr_header* p_head = (petr_header*) malloc(sizeof(petr_header));
        int rd = rd_msgheader(client_fd, p_head);
        if (rd < 0)
        {
            free(p_head);
            break;
            //close(client_fd);
            //exit(EXIT_FAILURE);
        }

        else if (rd != 0)
        {
            free(p_head);
            break;
            //close(client_fd);
            //exit(EXIT_FAILURE);
        }

        else if (p_head->msg_type == LOGOUT)
        {
            free(p_head);

            petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
            w_head->msg_len = 0;
            w_head->msg_type = OK;
            wr_msg(client_fd, w_head, "");
            free(w_head);
            break;
        }


        if (p_head->msg_len != 0)
        {
            char buff [p_head->msg_len]; 
            bzero(buff, p_head->msg_len);
            read(client_fd, buff, p_head->msg_len);

            job_t* p_job = (job_t*) malloc(sizeof(job_t));
            p_job->msg_type = p_head->msg_type;
            p_job->message = (char*) calloc(p_head->msg_len, 1);

        
            //printf("%s", buff);
            strcpy(p_job->message, buff);
            //printf("%s\n", p_job->message);

            // char* item_name = strtok(p_job->message, "\r\n");
            // char* duration = strtok(NULL,"\r\n");
            // char* bin_price = strtok(NULL,"\r\n");

            // int d = atoi(duration);

            //printf("%s%s%s\n", item_name, duration, bin_price);

            p_job->client_fd = client_fd;
            //p_job->p_head = p_head;

            pthread_mutex_lock(&job_lock);
            InsertAtTail(job_queue, p_job);
            pthread_mutex_unlock(&job_lock);
            sem_post(&job_sem);
            //free(buff);

        }

        else
        {
            job_t* p_job = (job_t*) malloc(sizeof(job_t));
            p_job->msg_type = p_head->msg_type;
            p_job->message = NULL;
            p_job->client_fd = client_fd;
            //p_job->p_head = p_head;

            pthread_mutex_lock(&job_lock);
            InsertAtTail(job_queue, p_job);
            pthread_mutex_unlock(&job_lock);
            sem_post(&job_sem);
        }
        

        free(p_head);

    }
    // Close the socket at the end
    pthread_mutex_lock(&user_w_lock);

    // find the user with the matching client_id
    node_t* usr_node = user_list->head;

    while (usr_node != NULL)
    {
        if (((user_t*)(usr_node->data))->file_descriptor == client_fd)
        {

            printf("Logged out username: %s\n", ((user_t*)(usr_node->data))->username);
            printf("Logged out password: %s\n", ((user_t*)(usr_node->data))->password);
            ((user_t*)(usr_node->data))->isActive = 0;

            //handle watching auctions
            if (((user_t*)(usr_node->data))->auctions_watching->length == 0)
            {
                break;
            }

            pthread_mutex_lock(&auction_w_lock);

            while (((user_t*)(usr_node->data))->auctions_watching->length != 0)
            {
                auction_t* auc = removeFront(((user_t*)(usr_node->data))->auctions_watching);

                node_t* usr_in_auc = auc->user_list->head;
                int user_i = 0;

                while (usr_in_auc != NULL)
                {
                    if (((user_t*)(usr_in_auc->data))->file_descriptor == client_fd)
                    {
                        removeByIndex(auc->user_list, user_i);
                        auc->num_watchers--;
                        break;
                    }
                    usr_in_auc = usr_in_auc->next;
                    user_i++;
                }

            }
            pthread_mutex_unlock(&auction_w_lock);

            break;
        }
        usr_node = usr_node->next;
    }

    pthread_mutex_unlock(&user_w_lock);
    printf("Client logged out\n");
    close(client_fd);
    return NULL;
}

// JOBS
void *job_threads(void *p){

    while(1)
    {
        //printf("mmmmmmm");
        // sleep(1);

        sem_wait(&job_sem);
        pthread_mutex_lock(&job_lock);
        job_t* curr_job = (job_t*) (removeFront(job_queue)); 
        pthread_mutex_unlock(&job_lock);
        

        // //do job stuff
        if (curr_job->msg_type == LOGOUT)
        {       

            pthread_mutex_lock(&user_w_lock);

            // find the user with the matching client_id
            node_t* usr_node = user_list->head;

            while (usr_node != NULL)
            {
                if (((user_t*)(usr_node->data))->file_descriptor == curr_job->client_fd)
                {

                    printf("Logged out username: %s\n", ((user_t*)(usr_node->data))->username);
                    printf("Logged out password: %s\n", ((user_t*)(usr_node->data))->password);
                    ((user_t*)(usr_node->data))->isActive = 0;

                    //handle watching auctions
                    if (((user_t*)(usr_node->data))->auctions_watching->length == 0)
                    {
                        break;
                    }

                     pthread_mutex_lock(&auction_w_lock);

                    while (((user_t*)(usr_node->data))->auctions_watching->length != 0)
                    {
                        auction_t* auc = removeFront(((user_t*)(usr_node->data))->auctions_watching);

                        node_t* usr_in_auc = auc->user_list->head;
                        int user_i = 0;

                        while (usr_in_auc != NULL)
                        {
                            if (((user_t*)(usr_in_auc->data))->file_descriptor == curr_job->client_fd)
                            {
                                removeByIndex(auc->user_list, user_i);
                                auc->num_watchers--;

                                break;
                            }
                            usr_in_auc = usr_in_auc->next;
                            user_i++;
                        }
                        

                    }
                    pthread_mutex_unlock(&auction_w_lock);

                    break;
                }
                usr_node = usr_node->next;
            }

            pthread_mutex_unlock(&user_w_lock);


            //write ok
            petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
            w_head->msg_len = 0;
            w_head->msg_type = OK;
            wr_msg(curr_job->client_fd, w_head, "");
            free(w_head);
            close(curr_job->client_fd);
        }

        else if (curr_job->msg_type == ANCREATE)
        {
            //printf("");
            //printf("%s\n", curr_job->message);

            //parsing the message
            char* item_name = strtok(curr_job->message, "\r\n");
            int duration = atoi(strtok(NULL,"\r\n"));
            int bin_price = atoi(strtok(NULL,"\r\n"));

            if(duration < 1 || bin_price < 0 || strcmp("", item_name) == 0)
            {
                petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
                w_head->msg_len = 0;
                w_head->msg_type = EINVALIDARG;
                wr_msg(curr_job->client_fd, w_head, "");
                free(w_head);
                continue;
            }
            
            printf("auction info: %s, %d, %d\n", item_name, duration, bin_price);

            //lock auction number
            pthread_mutex_lock(&auction_id_lock);

            printf("Auction id: %d\n", auction_id);

            pthread_mutex_lock(&user_w_lock);

            // find the user with the matching client_id
            node_t* usr_node = user_list->head;

            while (usr_node != NULL)
            {
                if (((user_t*)(usr_node->data))->file_descriptor == curr_job->client_fd)
                {

                    printf("Auction Creator: %s\n", ((user_t*)(usr_node->data))->username);

                    //create auction
                    auction_t* auction = (auction_t*)malloc(sizeof(auction_t));
                    auction->creator = ((user_t*)(usr_node->data))->username;
                    auction->isActive = 1;
                    auction->currentBid = 0;
                    auction->bin = bin_price;
                    auction->winner = NULL;
                    auction->item_name = item_name;
                    auction->auction_id = auction_id;
                    auction->remaining_ticks = duration;
                    auction->user_list = CreateList(auction_tDeleter);


                    // insert user into auction list and auction into created user list
                    //InsertAtTail(auction->user_list, (user_t*)(usr_node->data));
                    InsertAtTail(((user_t*)(usr_node->data))->auctions_created, auction);

                    // insert auction into the auction list
                    pthread_mutex_lock(&auction_w_lock);
                    InsertAtTail(auction_list, auction);
                    pthread_mutex_unlock(&auction_w_lock);

                    // send message
                    char msg[getIntLen(auction_id)]; 
                    sprintf(msg, "%d",auction_id);

                    petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
                    w_head->msg_len = getIntLen(auction_id);
                    w_head->msg_type = ANCREATE;
                    wr_msg(curr_job->client_fd, w_head, msg);

                    free(w_head);

                    auction_id++;


                    break;
                }
                usr_node = usr_node->next;
            }

            pthread_mutex_unlock(&user_w_lock);
            pthread_mutex_unlock(&auction_id_lock);

        }


        else if (curr_job->msg_type == ANWATCH)
        {
            int curr_id = atoi(curr_job->message);

            pthread_mutex_lock(&auction_w_lock);

            int auction_found = 0;

            //get auction node
            node_t* auc_node = auction_list->head;

            while (auc_node != NULL)
            {
                if (((auction_t*)(auc_node->data))->auction_id == curr_id)
                {
                    
                    auction_found = 1;

                    break;
                }
                auc_node = auc_node->next;
            }

            if (auction_found ==0 || ((auction_t*)(auc_node->data))->isActive == 0)
            {
                pthread_mutex_unlock(&auction_w_lock);
                petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
                w_head->msg_len = 0;
                w_head->msg_type = EANNOTFOUND;
                wr_msg(curr_job->client_fd, w_head, "");
                free(w_head);
                continue;

            }

            //get user node
            pthread_mutex_lock(&user_w_lock);
            node_t* usr_node = user_list->head;

            while (usr_node != NULL)
            {
                if (((user_t*)(usr_node->data))->file_descriptor == curr_job->client_fd)
                {


                    break;
                }
                usr_node = usr_node->next;
            }

            InsertAtTail(((user_t*)(usr_node->data))->auctions_watching, ((auction_t*)(auc_node->data)));
            InsertAtTail(((auction_t*)(auc_node->data))->user_list, ((user_t*)(usr_node->data)));

            char msg[] = "";

            strncat(msg, ((auction_t*)(auc_node->data))->item_name, (int)strlen(((auction_t*)(auc_node->data))->item_name));
            strcat(msg, "\r\n");
            char bin_msg[getIntLen(((auction_t*)(auc_node->data))->bin)]; 
            sprintf(bin_msg, "%d",((auction_t*)(auc_node->data))->bin);
            strncat(msg, bin_msg, (int)strlen(bin_msg));
            strcat(msg, "\0");
            char* message = (char*)malloc((int)strlen(msg));
            strcpy(message, msg);
            
            ((auction_t*)(auc_node->data))->num_watchers++;

            petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
            w_head->msg_len = (int)strlen(message) + 1;
            w_head->msg_type = ANWATCH;
            wr_msg(curr_job->client_fd, w_head, message);
            free(w_head);
            free(message);

            pthread_mutex_unlock(&auction_w_lock);
            pthread_mutex_unlock(&user_w_lock);

        }

        else if (curr_job->msg_type == ANLEAVE)
        {
            int curr_id = atoi(curr_job->message);

            pthread_mutex_lock(&auction_w_lock);

            int auction_found = 0;

            //get auction node
            node_t* auc_node = auction_list->head;

            while (auc_node != NULL)
            {
                if (((auction_t*)(auc_node->data))->auction_id == curr_id)
                {
                    
                    auction_found = 1;

                    break;
                }
                auc_node = auc_node->next;
            }

            if (auction_found ==0 )
            {
                pthread_mutex_unlock(&auction_w_lock);
                petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
                w_head->msg_len = 0;
                w_head->msg_type = EANNOTFOUND;
                wr_msg(curr_job->client_fd, w_head, "");
                free(w_head);
                continue;

            }

            pthread_mutex_lock(&user_w_lock);
            node_t* usr_node = user_list->head;

            while (usr_node != NULL)
            {
                if (((user_t*)(usr_node->data))->file_descriptor == curr_job->client_fd)
                {


                    break;
                }
                usr_node = usr_node->next;
            }


            node_t* usr_in_auc = ((auction_t*)(auc_node->data))->user_list->head;
            int user_i = 0;

            while (usr_in_auc != NULL)
            {
                if (((user_t*)(usr_in_auc->data))->file_descriptor == curr_job->client_fd)
                {
                    //printf("%d\n", user_i);
                    removeByIndex(((auction_t*)(auc_node->data))->user_list, user_i);

                    break;

                }
                usr_in_auc = usr_in_auc->next;
                user_i++;
            }

            node_t* auc_in_usr = ((user_t*)(usr_node->data))->auctions_watching->head;
            int auc_i = 0;

            while (auc_in_usr != NULL)
            {
                if (((auction_t*)(auc_in_usr->data))->auction_id == curr_id)
                {   
                    //printf("%d\n", auc_i);
                    removeByIndex(((user_t*)(usr_node->data))->auctions_watching, auc_i);

                    break;
                }
                auc_in_usr = auc_in_usr->next;
                auc_i++;
            }

            ((auction_t*)(auc_node->data))->num_watchers--;

            petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
            w_head->msg_len = 0;
            w_head->msg_type = OK;
            wr_msg(curr_job->client_fd, w_head, "");
            free(w_head);

            pthread_mutex_unlock(&auction_w_lock);
            pthread_mutex_unlock(&user_w_lock);

        }

        else if (curr_job->msg_type == ANBID)
        {

            int curr_id= atoi(strtok(curr_job->message, "\r\n"));
            int bid = atoi(strtok(NULL,"\r\n"));

            pthread_mutex_lock(&auction_w_lock);

            int auction_found = 0;

            //get auction node
            node_t* auc_node = auction_list->head;

            while (auc_node != NULL)
            {
                if (((auction_t*)(auc_node->data))->auction_id == curr_id)
                {
                    auction_found = 1;

                    break;
                }
                auc_node = auc_node->next;
            }

            // If the auctions isn't found or is closed
            if (auction_found ==0 || ((auction_t*)(auc_node->data))->isActive == 0)
            {
                pthread_mutex_unlock(&auction_w_lock);
                petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
                w_head->msg_len = 0;
                w_head->msg_type = EANNOTFOUND;
                wr_msg(curr_job->client_fd, w_head, "");
                free(w_head);
                continue;

            }

            // get user node
            pthread_mutex_lock(&user_r_lock);
            user_readcnt++;
            if (user_readcnt == 1)
            {
                pthread_mutex_lock(&user_w_lock);
            }
            pthread_mutex_unlock(&user_r_lock);

            node_t* usr_node = user_list->head;

            while (usr_node != NULL)
            {
                if (((user_t*)(usr_node->data))->file_descriptor == curr_job->client_fd)
                {


                    break;
                }
                usr_node = usr_node->next;
            }
           
           // if user created auction
            if (((auction_t*)(auc_node->data))->creator != NULL && strcmp(((user_t*)(usr_node->data))->username, ((auction_t*)(auc_node->data))->creator) == 0)
            {
                petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
                w_head->msg_len = 0;
                w_head->msg_type = EANDENIED;
                wr_msg(curr_job->client_fd, w_head, "");
                free(w_head);

                pthread_mutex_lock(&user_r_lock);
                user_readcnt--;
                if (user_readcnt == 0)
                {
                    pthread_mutex_unlock(&user_w_lock);
                }
                pthread_mutex_unlock(&user_r_lock);
                pthread_mutex_unlock(&auction_w_lock);
                continue;
            }

            //if user is not watching auction (should not happen)
            int watcher_found = 0;
            node_t* w_node = ((auction_t*)(auc_node->data))->user_list->head;
            while (w_node != NULL)
            {
                if (((user_t*)(w_node->data))->file_descriptor == curr_job->client_fd)
                {

                    watcher_found = 1;
                    break;
                }

                w_node = w_node->next;
            }
            if (watcher_found == 0)
            {
                petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
                w_head->msg_len = 0;
                w_head->msg_type = EANDENIED;
                wr_msg(curr_job->client_fd, w_head, "");
                free(w_head);

                pthread_mutex_lock(&user_r_lock);
                user_readcnt--;
                if (user_readcnt == 0)
                {
                    pthread_mutex_unlock(&user_w_lock);
                }
                pthread_mutex_unlock(&user_r_lock);
                pthread_mutex_unlock(&auction_w_lock);
                continue;
            }

            // check if bid is too low
            if (bid <= ((auction_t*)(auc_node->data))->currentBid)
            {
                petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
                w_head->msg_len = 0;
                w_head->msg_type = EBIDLOW;
                wr_msg(curr_job->client_fd, w_head, "");
                free(w_head);

                pthread_mutex_lock(&user_r_lock);
                user_readcnt--;
                if (user_readcnt == 0)
                {
                    pthread_mutex_unlock(&user_w_lock);
                }
                pthread_mutex_unlock(&user_r_lock);
                pthread_mutex_unlock(&auction_w_lock);
                continue;
            }

            //updating the bid
            ((auction_t*)(auc_node->data))->currentBid = bid;
            ((auction_t*)(auc_node->data))->winner = ((user_t*)(usr_node->data))->username;

            //send ok to user
            petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
            w_head->msg_len = 0;
            w_head->msg_type = OK;
            wr_msg(curr_job->client_fd, w_head, "");
            free(w_head);

            if (((auction_t*)(auc_node->data))->bin != 0 && bid >= ((auction_t*)(auc_node->data))->bin)
            {
                bid = ((auction_t*)(auc_node->data))->bin;
            }


            // send update to watchers
            // Make the message
            char msg[] = "";

            char id_msg[getIntLen(((auction_t*)(auc_node->data))->auction_id)]; 
            sprintf(id_msg, "%d",((auction_t*)(auc_node->data))->auction_id);
            strncat(msg, id_msg, (int)strlen(id_msg));
            strcat(msg, "\r\n");

            strncat(msg, ((auction_t*)(auc_node->data))->item_name, (int)strlen(((auction_t*)(auc_node->data))->item_name));
            strcat(msg, "\r\n");

            strncat(msg, ((user_t*)(usr_node->data))->username, (int)strlen(((user_t*)(usr_node->data))->username));
            strcat(msg, "\r\n");

            char bid_msg[getIntLen(bid)]; 
            sprintf(bid_msg, "%d",bid);
            strncat(msg, bid_msg, (int)strlen(bid_msg));
            strcat(msg, "\0");

            char* message = (char*)malloc((int)strlen(msg));
            strcpy(message, msg);

            //send the update
            node_t* watching_node = ((auction_t*)(auc_node->data))->user_list->head;
            while (watching_node != NULL)
            {
                petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
                w_head->msg_len = (int)strlen(message) + 1;
                w_head->msg_type = ANUPDATE;
                wr_msg(((user_t*)(watching_node->data))->file_descriptor, w_head, message);
                free(w_head);

                watching_node = watching_node->next;
            }

            // Stop reading and unlock the auction writing
            pthread_mutex_lock(&user_r_lock);
            user_readcnt--;
            if (user_readcnt == 0)
            {
                pthread_mutex_unlock(&user_w_lock);
            }
            pthread_mutex_unlock(&user_r_lock);
            pthread_mutex_unlock(&auction_w_lock);

            free(message);

            //if the bid surpassed the bin
            if (((auction_t*)(auc_node->data))->bin != 0 && bid >= ((auction_t*)(auc_node->data))->bin)
            {
                pthread_mutex_lock(&user_w_lock);
                pthread_mutex_lock(&auction_w_lock);
                
                // Finish the auction and set the current bid to bin
                ((auction_t*)(auc_node->data))->isActive = 0;
                ((auction_t*)(auc_node->data))->currentBid = ((auction_t*)(auc_node->data))->bin;

                InsertAtTail(((user_t*)(usr_node->data))->auctions_won, ((auction_t*)(auc_node->data)));

                //write the winning message
                char win_msg[] = "";

                char id_msg[getIntLen(((auction_t*)(auc_node->data))->auction_id)]; 
                sprintf(id_msg, "%d",((auction_t*)(auc_node->data))->auction_id);
                strncat(win_msg, id_msg, (int)strlen(id_msg));
                strcat(win_msg, "\r\n");

                strncat(win_msg, ((user_t*)(usr_node->data))->username, (int)strlen(((user_t*)(usr_node->data))->username));
                strcat(win_msg, "\r\n");

                
                char bid_msg[getIntLen(bid)]; 
                sprintf(bid_msg, "%d",bid);
                strncat(win_msg, bid_msg, (int)strlen(bid_msg));
                strcat(win_msg, "\0");

                char* win_message = (char*)malloc((int)strlen(win_msg));
                strcpy(win_message, win_msg);

                //Sending the message the all the watching users
                node_t* wwatching_node = ((auction_t*)(auc_node->data))->user_list->head;
                while (wwatching_node != NULL)
                {
                    petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
                    w_head->msg_len = (int)strlen(win_message) + 1;
                    w_head->msg_type = ANCLOSED;
                    wr_msg(((user_t*)(wwatching_node->data))->file_descriptor, w_head, win_message);
                    free(w_head);

                    wwatching_node = wwatching_node->next;
                }

                free(win_message);

                pthread_mutex_unlock(&user_w_lock);
                pthread_mutex_unlock(&auction_w_lock);

            }

        }


        else if (curr_job->msg_type == USRLIST){
          	char usr_list[] = "";
          	//int names = 0;
          	petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));

            // lock the reading
            pthread_mutex_lock(&user_r_lock);
            user_readcnt++;
            if (user_readcnt == 1)
            {
                pthread_mutex_lock(&user_w_lock);
            }
            pthread_mutex_unlock(&user_r_lock);


          	//pthread_mutex_lock(&user_r_lock);
          	node_t* usr_node = user_list->head;
            while(usr_node != NULL){
          			if ((((user_t*)(usr_node->data))->isActive == 1) && (((user_t*)(usr_node->data))->file_descriptor != curr_job->client_fd)){
                		//names++;
                		char temp[(int)strlen(((user_t*)(usr_node->data))->username)];
                		int i = 0;
                		for(i; i < (int)strlen(((user_t*)(usr_node->data))->username); i++){
                    		temp[i] = (char)*(((user_t*)(usr_node->data))->username + i);
                		}
                        //strcpy(temp, ((user_t*)(usr_node->data))->username);
                		temp[i] = '\0';
                		strncat(usr_list, temp, (int)strlen(temp));
                		strcat(usr_list, "\n");

          			}
          			usr_node = usr_node->next;
            }
            //pthread_mutex_unlock(&user_r_lock);

            pthread_mutex_lock(&user_r_lock);
            user_readcnt--;
            if (user_readcnt == 0)
            {
                pthread_mutex_unlock(&user_w_lock);
            }
            pthread_mutex_unlock(&user_r_lock);



            char* compare = "";
            if(strncmp(usr_list, compare, 1) == 0){
            		w_head->msg_len = 0;
            		w_head->msg_type = USRLIST;
            		wr_msg(curr_job->client_fd, w_head, "");
            }
            else{
                strcat(usr_list, "\0");
                char* userlist = (char*)malloc((int)strlen(usr_list));
                strcpy(userlist, usr_list);
                w_head->msg_len = (uint32_t)((int)strlen(userlist) + 1);
                w_head->msg_type = USRLIST;
                wr_msg(curr_job->client_fd, w_head, userlist);
                free(userlist);
                }
            free(w_head);
        }

        else if (curr_job->msg_type == ANLIST){
            char auctionList[] = "";
            petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));

            pthread_mutex_lock(&auction_r_lock);
            user_readcnt++;
            if (user_readcnt == 1)
            {
                pthread_mutex_lock(&auction_w_lock);
            }
            pthread_mutex_unlock(&auction_r_lock);

            node_t* auction = auction_list->head;
            while(auction != NULL){
                if((((auction_t*)(auction->data))->isActive == 1)){
                    int len = 0;
                    int test = ((auction_t*)(auction->data))->auction_id;
                    int num = 0;
                    while(test != 0){
                        test = test/10;
                        num++;
                    }
                    int test5 = ((auction_t*)(auction->data))->bin;
                    int num5 = 0;
                    if(test5 == 0){
                        num5 = 1;
                    }
                    else{
                        while(test5 != 0){
                                test5 = test5/10;
                                num5++;
                            }
                    }
                    int test2 = ((auction_t*)(auction->data))->currentBid;
                    int num2 = 0;
                    if(test2 == 0){
                        num2 = 1;
                    }
                    else{
                        while(test2 != 0){
                        test2 = test2/10;
                        num2++;
                        }
                    }
                    int test3 = ((auction_t*)(auction->data))->num_watchers;
                    int num3 = 0;
                    if(test3 == 0){
                    num3 = 1;
                    }
                    else{
                        while(test3 != 0){
                        test3 = test3/10;
                        num3++;
                        }
                    }
                    int test4 = ((auction_t*)(auction->data))->remaining_ticks;
                    int num4 = 0;
                    while(test4 != 0){
                        test4 = test4/10;
                        num4++;
                    }
                    char auction_id[num];
                    sprintf(auction_id, "%d", ((auction_t*)(auction->data))->auction_id);
                    char currentBid[num2];
                    sprintf(currentBid, "%d", ((auction_t*)(auction->data))->currentBid);
                    char num_watchers[num3];
                    sprintf(num_watchers, "%d", ((auction_t*)(auction->data))->num_watchers);
                    char duration[num4];
                    char bin[num5];
                                sprintf(bin, "%d", ((auction_t*)(auction->data))->bin);
                    
                    sprintf(duration, "%d", ((auction_t*)(auction->data))->remaining_ticks);
                    len += (int)(strlen(auction_id)) + 1;
                    len += (int)strlen(((auction_t*)(auction->data))->item_name) + 1;
                    len += (int)(strlen(bin) + 1);
                    len += (int)(strlen(currentBid)) + 1;
                    len += (int)(strlen(num_watchers)) + 1;
                    len += (int)(strlen(duration));

                    char temp[len];
                    int i = 0;
                    for(i; i < (int)strlen(auction_id); i++){
                        temp[i] = *(auction_id + i);
                    }
                    temp[i] = ';';
                    i++;
                    int j = 0;
                    for(j; j < (int)strlen(((auction_t*)(auction->data))->item_name); j++){
                        temp[i] = *(((auction_t*)(auction->data))->item_name + j);
                        i++;
                    }
                    temp[i] = ';';
                    i++;
                    int n = 0;
                                for(n; n < (int)strlen(bin); n++){
                                    temp[i] = *(bin + n);
                                    i++;
                                }
                        temp[i] = ';';
                    i++;
                    int k = 0;
                    for(k; k < (int)strlen(currentBid); k++){
                        temp[i] = *(currentBid + k);
                        i++;
                    }
                    temp[i] = ';';
                    i++;
                    int l = 0;
                    for(l; l < (int)strlen(num_watchers); l++){
                        temp[i] = *(num_watchers + l);
                        i++;
                    }
                    temp[i] = ';';
                    i++;
                    int m = 0;
                    for(m; m < (int)strlen(duration); m++){
                        temp[i] = *(duration + m);
                        i++;
                    }
                    temp[i] = '\0';
                    strncat(auctionList, temp, (int)strlen(temp));
                    strcat(auctionList, "\n");
                }
                auction = auction->next;
                }

            pthread_mutex_lock(&auction_r_lock);
            user_readcnt--;
            if (user_readcnt == 0)
            {
                pthread_mutex_unlock(&auction_w_lock);
            }
            pthread_mutex_unlock(&auction_r_lock);

            char* compare = "";
            if(strncmp(auctionList, compare, 1) == 0){
                w_head->msg_len = 0;
                w_head->msg_type = ANLIST;
                wr_msg(curr_job->client_fd, w_head, "");
            }
            else{
                strcat(auctionList, "\0");
                //printf("%s\n", auctionList);
                char* auctionlist = (char*)malloc((int)strlen(auctionList));
                strcpy(auctionlist, auctionList);
                w_head->msg_len = (uint32_t)((int)strlen(auctionList) + 1);
                w_head->msg_type = ANLIST;
                wr_msg(curr_job->client_fd, w_head, auctionlist);
                free(auctionlist);
                }
            free(w_head);
            }


            else if (curr_job->msg_type == USRWINS){
                char auctionList[] = "";
                petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
            
                pthread_mutex_lock(&user_r_lock);
                    user_readcnt++;
                    if (user_readcnt == 1)
                    {
                    pthread_mutex_lock(&user_w_lock);
                    }
                    pthread_mutex_unlock(&user_r_lock);

                node_t* usr_node = user_list->head;
                while(usr_node != NULL){
                            if (((user_t*)(usr_node->data))->file_descriptor == curr_job->client_fd){
                                    break;
                            }
                            usr_node = usr_node->next;
                }
            
                pthread_mutex_lock(&auction_r_lock);
                auction_readcnt++;
                if (auction_readcnt == 1)
                {
                    pthread_mutex_lock(&auction_w_lock);
                }
                pthread_mutex_unlock(&auction_r_lock);
                
                node_t* auction = ((user_t*)(usr_node->data))->auctions_won->head;
                while(auction != NULL){
                int len = 0;
                int test = ((auction_t*)(auction->data))->auction_id;
                int num = 0;
                while(test != 0){
                        test = test/10;
                        num++;
                }
                int test2 = ((auction_t*)(auction->data))->currentBid;
                int num2 = 0;
                while(test2 != 0){
                        test2 = test2/10;
                        num2++;
                }
                char auction_id[num];
                sprintf(auction_id, "%d", ((auction_t*)(auction->data))->auction_id);
                char currentBid[num2];
                sprintf(currentBid, "%d", ((auction_t*)(auction->data))->currentBid);
                
                len += (int)num + 1;
                len += (int)strlen(((auction_t*)(auction->data))->item_name) + 1;
                len += (int)strlen(currentBid);
                char temp[len];
                int i = 0;
                for(i; i < (int)strlen(auction_id); i++){
                    temp[i] = *(auction_id + i);
                }
                temp[i] = ';';
                i++;
                int j = 0;
                for(j; j < (int)strlen(((auction_t*)(auction->data))->item_name); j++){
                    temp[i] = *(((auction_t*)(auction->data))->item_name + j);
                    i++;
                }
                temp[i] = ';';
                i++;
                int k = 0;
                for(k; k < (int)strlen(currentBid); k++){
                    temp[i] = *(currentBid + k);
                    i++;
                }
                temp[i] = '\0';
                strncat(auctionList, temp, (int)strlen(temp));
                strcat(auctionList, "\n");
                auction = auction->next;
                }
                
                pthread_mutex_lock(&user_r_lock);
                user_readcnt--;
                if (user_readcnt == 0)
                {
                    pthread_mutex_unlock(&user_w_lock);
                }
                pthread_mutex_unlock(&user_r_lock);
                
                pthread_mutex_lock(&auction_r_lock);
                auction_readcnt--;
                if (auction_readcnt == 0)
                {
                    pthread_mutex_unlock(&auction_w_lock);
                }
                pthread_mutex_unlock(&auction_r_lock);
                
                char* compare = "";
                if(strncmp(auctionList, compare, 1) == 0){
                        w_head->msg_len = 0;
                    w_head->msg_type = USRWINS;
                        wr_msg(curr_job->client_fd, w_head, "");
                    }
                    else{
                                strcat(auctionList, "\0");
                                char* auctionlist = (char*)malloc((int)strlen(auctionList));
                    strcpy(auctionlist, auctionList);
                                w_head->msg_len = (uint32_t)((int)strlen(auctionlist) + 1);
                            w_head->msg_type = USRWINS;
                            wr_msg(curr_job->client_fd, w_head, auctionlist);
                    free(auctionlist);
                    }
                free(w_head);
            }

            else if (curr_job->msg_type == USRSALES){
            char salesList[] = "";
            int names = 0;
            petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));

            pthread_mutex_lock(&user_r_lock);
            user_readcnt++;
            if (user_readcnt == 1)
            {
                pthread_mutex_lock(&user_w_lock);
            }
            pthread_mutex_unlock(&user_r_lock);

            node_t* usr_node = user_list->head;
            while(usr_node != NULL){
            if (((user_t*)(usr_node->data))->file_descriptor == curr_job->client_fd){
                break;
            }
            usr_node = usr_node->next;
            }
        
            pthread_mutex_lock(&auction_r_lock);
            auction_readcnt++;
            if (auction_readcnt == 1)
            {
                pthread_mutex_lock(&auction_w_lock);
            }
            pthread_mutex_unlock(&auction_r_lock);
        
            node_t* auction = ((user_t*)(usr_node->data))->auctions_created->head;
            while(auction != NULL){
                        if((((auction_t*)(auction->data))->isActive == 0) && (((auction_t*)(auction->data))->winner != NULL)){
                int len = 0;
                int test = ((auction_t*)(auction->data))->auction_id;
                int num = 0;
                while(test != 0){
                    test = test/10;
                    num++;
                }
                int test2 = ((auction_t*)(auction->data))->currentBid;
                int num2 = 0;
                while(test2 != 0){
                    test2 = test2/10;
                    num2++;
                }
                char auction_id[num];
                sprintf(auction_id, "%d", ((auction_t*)(auction->data))->auction_id);
                char currentBid[num2];
                sprintf(currentBid, "%d", ((auction_t*)(auction->data))->currentBid);
                
                len += (int)(strlen(auction_id)) + 1;
                len += (int)strlen(((auction_t*)(auction->data))->winner) + 1;
                len += (int)strlen(((auction_t*)(auction->data))->item_name) + 1;
                len += (int)strlen(((auction_t*)(auction->data))->winner) + 1;
                len += (int)(strlen(currentBid));

                char temp[len];
                int i = 0;
                for(i; i < (int)strlen(auction_id); i++){
                    temp[i] = *(auction_id + i);
                }
                temp[i] = ';';
                i++;
                //printf("awefa\n");
                int j = 0;
                for(j; j < (int)strlen(((auction_t*)(auction->data))->item_name); j++){
                    temp[i] = *(((auction_t*)(auction->data))->item_name + j);
                    i++;
                }
                temp[i] = ';';
                i++;
                int k = 0;
                for(k; k < (int)strlen(((auction_t*)(auction->data))->winner); k++){
                    temp[i] = *(((auction_t*)(auction->data))->winner + k);
                    i++;
                }
                temp[i] = ';';
                i++;
                int l = 0;
                for(l; l < (int)strlen(currentBid); l++){
                    temp[i] = *(currentBid + l);
                    i++;
                }
                temp[i] = '\0';
                strncat(salesList, temp, (int)strlen(temp));
                strcat(salesList, "\n");
                }
                auction = auction->next;
            }

            pthread_mutex_lock(&user_r_lock);
            user_readcnt--;
            if (user_readcnt == 0)
            {
                pthread_mutex_unlock(&user_w_lock);
            }
            pthread_mutex_unlock(&user_r_lock);
        
            pthread_mutex_lock(&auction_r_lock);
            auction_readcnt--;
            if (auction_readcnt == 0)
            {
                    pthread_mutex_unlock(&auction_w_lock);
            }
            pthread_mutex_unlock(&auction_r_lock);

            char* compare = "";
            if(strncmp(salesList, compare, 1) == 0){
            w_head->msg_len = 0;
            w_head->msg_type = USRSALES;
            wr_msg(curr_job->client_fd, w_head, "");
            }
            else{
            strcat(salesList, "\0");
            char* saleslist = (char*)malloc((int)strlen(salesList));
            strcpy(saleslist, salesList);
            w_head->msg_len = (uint32_t)((int)strlen(salesList) + 1);
            w_head->msg_type = USRSALES;
            wr_msg(curr_job->client_fd, w_head, saleslist);
            free(saleslist);
            }
            free(w_head);
        }

        else if(curr_job->msg_type == USRBLNC){
			pthread_mutex_lock(&user_r_lock);
            user_readcnt++;
            if (user_readcnt == 1)
            {
                pthread_mutex_lock(&user_w_lock);
            }
            pthread_mutex_unlock(&user_r_lock);

			int total = 0;
 
			node_t* usr_node = user_list->head;

            // get user node
			petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
			while(usr_node != NULL){
            if (((user_t*)(usr_node->data))->file_descriptor == curr_job->client_fd){
                break;
                }
            usr_node = usr_node->next;
			}

            pthread_mutex_lock(&auction_r_lock);
            auction_readcnt++;
            if (auction_readcnt == 1)
            {
                pthread_mutex_lock(&auction_w_lock);
            }
            pthread_mutex_unlock(&auction_r_lock);

			node_t* auctions_won = ((user_t*)(usr_node->data))->auctions_won->head;
			node_t* auctions_created = ((user_t*)(usr_node->data))->auctions_created->head;
			
			while(auctions_won != NULL){
                if(((auction_t*)(auctions_won->data))->isActive == 0){
            	total -= ((auction_t*)(auctions_won->data))->currentBid;
                }
                auctions_won = auctions_won->next;
			}

			while(auctions_created != NULL){
                if(((auction_t*)(auctions_created->data))->isActive == 0){
            	total += ((auction_t*)(auctions_created->data))->currentBid;
                }
                auctions_created = auctions_created->next;
			}

			pthread_mutex_lock(&auction_r_lock);
			auction_readcnt--;
			if (auction_readcnt == 0)
			{
		  		pthread_mutex_unlock(&auction_w_lock);
			}
			pthread_mutex_unlock(&auction_r_lock);

			char* balance = (char*)(malloc(getIntLen(total)));
			sprintf(balance, "%d", total);
			w_head->msg_len = (uint32_t)((int)strlen(balance) + 1);
			w_head->msg_type = USRBLNC;
			wr_msg(curr_job->client_fd, w_head, balance);
			free(w_head);
			free(balance);

            pthread_mutex_lock(&user_r_lock);
			user_readcnt--;
			if (user_readcnt == 0)
			{
		  		pthread_mutex_unlock(&user_w_lock);
			}
			pthread_mutex_unlock(&user_r_lock);

	    }

        

        // done with job
        
        job_tDeleter(curr_job);

    }
    return NULL;

}

void* tick_thread(void* tick){
	int seconds = *(int*) tick;
	free(tick);
	if(seconds == 0){
      while(1){

          // reading from the input
          char *line = NULL;
          size_t len = 0;
          ssize_t read;
          if((read = getline(&line, &len, stdin)) != -1){
              printf("TICK\n");
              pthread_mutex_lock(&auction_w_lock);
              

              // looping through the auctions
              node_t* auction = auction_list->head;
              while(auction != NULL){
              
              		if(((auction_t*)(auction->data))->isActive == 1){
                      
                      if(((auction_t*)(auction->data))->remaining_ticks > 0){
                        	((auction_t*)(auction->data))->remaining_ticks--;
                      }
                    
                    //if the auction is out of ticks
                    if(((auction_t*)(auction->data))->remaining_ticks <= 0){
                        ((auction_t*)(auction->data))->isActive = 0;
                        
                        //start creating the message
                        char win_msg[] = "";

                        char id_msg[getIntLen(((auction_t*)(auction->data))->auction_id)]; 
                        sprintf(id_msg, "%d",((auction_t*)(auction->data))->auction_id);
                        strncat(win_msg, id_msg, (int)strlen(id_msg));
                        strcat(win_msg, "\r\n");

                        if (((auction_t*)(auction->data))->winner != NULL)
                        {
                            // find the winning user
                            pthread_mutex_lock(&user_w_lock);
                            node_t* usr_node = user_list->head;
                            while (usr_node != NULL)
                            {
                                if (strcmp(((user_t*)(usr_node->data))->username, ((auction_t*)(auction->data))->winner)==0)
                                {


                                    break;
                                }
                                usr_node = usr_node->next;
                            }

                            // Add the winner to the auctions won
                            InsertAtTail(((user_t*)(usr_node->data))->auctions_won, ((auction_t*)(auction->data)));

                            strncat(win_msg, ((user_t*)(usr_node->data))->username, (int)strlen(((user_t*)(usr_node->data))->username));
                            strcat(win_msg, "\r\n");

                            char bid_msg[getIntLen(((auction_t*)(auction->data))->currentBid)]; 
                            sprintf(bid_msg, "%d",((auction_t*)(auction->data))->currentBid);
                            strncat(win_msg, bid_msg, (int)strlen(bid_msg));
                            strcat(win_msg, "\0");

                            pthread_mutex_unlock(&user_w_lock);

                        }

                        else
                        {
                            strcat(win_msg, "\r\n\0");
                        }

                        char* win_message = (char*)malloc((int)strlen(win_msg));
                        strcpy(win_message, win_msg);

                        //lock reading
                        pthread_mutex_lock(&user_r_lock);
                        user_readcnt++;
                        if (user_readcnt == 1)
                        {
                            pthread_mutex_lock(&user_w_lock);
                        }
                        pthread_mutex_unlock(&user_r_lock);

                        // Send ANCLOSED to all users watching
                        node_t* wwatching_node = ((auction_t*)(auction->data))->user_list->head;
                        while (wwatching_node != NULL)
                        {
                            petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
                            w_head->msg_len = (int)strlen(win_message) + 1;
                            w_head->msg_type = ANCLOSED;
                            wr_msg(((user_t*)(wwatching_node->data))->file_descriptor, w_head, win_message);
                            free(w_head);

                            wwatching_node = wwatching_node->next;
                        }

                        free(win_message);


                        //unlock reading
                        pthread_mutex_lock(&user_r_lock);
                        user_readcnt--;
                        if (user_readcnt == 0)
                        {
                            pthread_mutex_unlock(&user_w_lock);
                        }
                        pthread_mutex_unlock(&user_r_lock);
                        

                      }
                  }
              		auction = auction->next;
              }


              pthread_mutex_unlock(&auction_w_lock);
          }
      }
	}
	else{
      while(1){
          sleep(seconds);
          printf("TICK\n");
          pthread_mutex_lock(&auction_w_lock);
          node_t* auction = auction_list->head;
          while(auction != NULL){
              if(((auction_t*)(auction->data))->isActive == 1){
                  if(((auction_t*)(auction->data))->remaining_ticks > 0){
                    	((auction_t*)(auction->data))->remaining_ticks--;
                  }
                  if(((auction_t*)(auction->data))->remaining_ticks <= 0){
                      ((auction_t*)(auction->data))->isActive = 0;
                        
                        //start creating the message
                        char win_msg[] = "";

                        char id_msg[getIntLen(((auction_t*)(auction->data))->auction_id)]; 
                        sprintf(id_msg, "%d",((auction_t*)(auction->data))->auction_id);
                        strncat(win_msg, id_msg, (int)strlen(id_msg));
                        strcat(win_msg, "\r\n");

                        if (((auction_t*)(auction->data))->winner != NULL)
                        {
                            // find the winning user
                            pthread_mutex_lock(&user_w_lock);
                            node_t* usr_node = user_list->head;
                            while (usr_node != NULL)
                            {
                                if (strcmp(((user_t*)(usr_node->data))->username, ((auction_t*)(auction->data))->winner)==0)
                                {


                                    break;
                                }
                                usr_node = usr_node->next;
                            }

                            // Add the winner to the auctions won
                            InsertAtTail(((user_t*)(usr_node->data))->auctions_won, ((auction_t*)(auction->data)));

                            strncat(win_msg, ((user_t*)(usr_node->data))->username, (int)strlen(((user_t*)(usr_node->data))->username));
                            strcat(win_msg, "\r\n");

                            char bid_msg[getIntLen(((auction_t*)(auction->data))->currentBid)]; 
                            sprintf(bid_msg, "%d",((auction_t*)(auction->data))->currentBid);
                            strncat(win_msg, bid_msg, (int)strlen(bid_msg));
                            strcat(win_msg, "\0");

                            pthread_mutex_unlock(&user_w_lock);

                        }

                        else
                        {
                            strcat(win_msg, "\r\n\0");
                        }

                        char* win_message = (char*)malloc((int)strlen(win_msg));
                        strcpy(win_message, win_msg);

                        //lock reading
                        pthread_mutex_lock(&user_r_lock);
                        user_readcnt++;
                        if (user_readcnt == 1)
                        {
                            pthread_mutex_lock(&user_w_lock);
                        }
                        pthread_mutex_unlock(&user_r_lock);

                        // Send ANCLOSED to all users watching
                        node_t* wwatching_node = ((auction_t*)(auction->data))->user_list->head;
                        while (wwatching_node != NULL)
                        {
                            petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
                            w_head->msg_len = (int)strlen(win_message) + 1;
                            w_head->msg_type = ANCLOSED;
                            wr_msg(((user_t*)(wwatching_node->data))->file_descriptor, w_head, win_message);
                            free(w_head);

                            wwatching_node = wwatching_node->next;
                        }

                        free(win_message);


                        //unlock reading
                        pthread_mutex_lock(&user_r_lock);
                        user_readcnt--;
                        if (user_readcnt == 0)
                        {
                            pthread_mutex_unlock(&user_w_lock);
                        }
                        pthread_mutex_unlock(&user_r_lock);
                  }
              }
              auction = auction->next;
          }
          pthread_mutex_unlock(&auction_w_lock);
      }
	}
	return NULL;
}

// Main thread
void run_server(int server_port){
    listen_fd = server_init(server_port); // Initiate server and start listening on specified port
    int client_fd;
    struct sockaddr_in client_addr;
    int client_addr_len = sizeof(client_addr);

    //pthread_t tid;

    while(1){
        // Wait and Accept the connection from client
        printf("Wait for new client connection\n");
        int* client_fd = malloc(sizeof(int));
        *client_fd = accept(listen_fd, (SA*)&client_addr, (socklen_t*)&client_addr_len);
        if (*client_fd < 0) {
            printf("server acccept failed\n");
            exit(EXIT_FAILURE);
        }
        else{


            petr_header* p_head = (petr_header*) malloc(sizeof(petr_header));
            int rd = rd_msgheader(*client_fd, p_head);
            if (rd < 0)
            {
                // close(client_fd);
                // exit(EXIT_FAILURE);
            }

            else if (rd != 0)
            {
                continue;
            }


            if (p_head->msg_type == LOGIN)
            {   

                // Read from the client
                char buff [p_head->msg_len]; 
                bzero(buff, p_head->msg_len);
                read(*client_fd, buff, p_head->msg_len); 

                char* message = (char*)calloc(p_head->msg_len, 1);
                strcpy(message, buff);
                
                // use strtok to get the username and password
                char* username = strtok(message, "\r\n");
                printf("Username is: %s\n", username); // printing for good measure
                char* password = strtok(NULL,"\r\n");
                printf("Password is: %s\n", password);

                // Run checks and stuff.
                int user_found = 0;
                pthread_mutex_lock(&user_w_lock);
                node_t* usr_node = user_list->head;

                while (usr_node != NULL)
                {   
                    
                    if (strcmp(((user_t*)(usr_node->data))->username, username) == 0)
                    {   
                        user_found = 1;
                        printf("Username found!\n");

                        if (strcmp(((user_t*)(usr_node->data))->password, password) !=0)
                        {
                            petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
                            w_head->msg_len = 0;
                            w_head->msg_type = EWRNGPWD;
                            wr_msg(*client_fd, w_head, "");
                            free(w_head);
                            break;
                        }
                        
                        if (((user_t*)(usr_node->data))->isActive == 1)
                        {
                            petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
                            w_head->msg_len = 0;
                            w_head->msg_type = EUSRLGDIN;
                            wr_msg(*client_fd, w_head, "");
                            free(w_head);
                            break;
                        }

                        ((user_t*)(usr_node->data))->isActive = 1;
                        ((user_t*)(usr_node->data))->file_descriptor = *client_fd;

                        petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
                        w_head->msg_len = 0;
                        w_head->msg_type = OK;
                        wr_msg(*client_fd, w_head, "");
                        free(w_head);
                        printf("Client connetion accepted\n");
                        pthread_create(&tid, NULL, process_client, (void *)client_fd); 
                        break;
                    }
                    usr_node = usr_node->next;
                }

                pthread_mutex_unlock(&user_w_lock);


                if (user_found == 1) {
                    free(p_head);
                    continue;
                    }

                // create the new user
                user_t* new_user = (user_t*)malloc(sizeof(user_t));
                new_user->username = username;
                new_user->password = password;
                new_user->isActive = 1;
                new_user->file_descriptor = *client_fd;
                new_user-> auctions_watching = CreateList(auction_tDeleter);
                new_user-> auctions_won = CreateList(auction_tDeleter);
                new_user-> auctions_created = CreateList(auction_tDeleter);

                // lock auction list, put new user in the list, unlock auction list
                pthread_mutex_lock(&user_w_lock);
                InsertAtTail(user_list, new_user);
                pthread_mutex_unlock(&user_w_lock);

                // Write OK to the client.
                petr_header* w_head = (petr_header*) malloc(sizeof(petr_header));
                w_head->msg_len = 0;
                w_head->msg_type = OK;
                wr_msg(*client_fd, w_head, "");
                free(w_head);
                free(p_head);
                printf("Client connetion accepted\n");
                pthread_create(&tid, NULL, process_client, (void *)client_fd); 


            }
        }
    }
    bzero(buffer, BUFFER_SIZE);
    close(listen_fd);
    return;
}


int main(int argc, char* argv[]) {
    // You got this!

    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
    perror("Failed to set signal handler");
    exit(EXIT_FAILURE);
	}

    job_num = 2;
    int* tick_seconds = (int*)(malloc(sizeof(int)));
    sem_init(&job_sem, 0, 0);
    *tick_seconds = 0;

    int opt;

    unsigned int port = 0;
    while ((opt = getopt(argc, argv, "h:j:t:")) != -1) {
        switch (opt) {
        case 'h':
            printf("%s", USAGE_TEXT);
            return EXIT_SUCCESS;
            break;
        case 'j':
                //printf("%s\n", optarg);
        		job_num = atoi(optarg);
                break;
        case 't':
                *tick_seconds = atoi(optarg);
        		break;
        default: /* '?' */
            fprintf(stderr, "Server Application Usage: %s -p <port_number>\n",
                    argv[0]);
            exit(1);
        }
    }
    
    port = atoi(argv[argc - 2]);
    char *c = argv[argc - 1];
    

    if (port == 0){
        fprintf(stderr, "ERROR: Port number for server to listen is not given\n");
        fprintf(stderr, "Server Application Usage: %s -p <port_number>\n",
                    argv[0]);
        exit(1);
    }


    auction_list = CreateList(auction_tDeleter);
    user_list = CreateList(user_tDeleter);
    job_queue = CreateList(job_tDeleter);

    // input all the actions from the txt file
    char *filename = argv[argc - 1];

    char *line = NULL;
    size_t length = 0;
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("BAD AUCTION FILE\n");
        exit(1);
    }
    ssize_t read;
    while ((read = getline(&line, &length, file)) != -1){
      char* item_name = (char*)(malloc((int)(strlen(line)) - 2));
      strncpy(item_name, line, (int)(strlen(line)) - 2);
      auction_t* auction = (auction_t*)malloc(sizeof(auction_t));
      auction->item_name = item_name;
      auction->creator = NULL;
      auction->isActive = 1;
      auction->currentBid = 0;

      read = getline(&line, &length, file);
      int duration = atoi(line);
      auction->remaining_ticks = duration;

      read = getline(&line, &length, file);
      int bin = atoi(line);
      auction->bin = bin;
      auction->winner = NULL;

      pthread_mutex_lock(&auction_id_lock);
      auction->auction_id = auction_id;
      auction_id++;
      pthread_mutex_unlock(&auction_id_lock);
      auction->user_list = CreateList(auction_tDeleter);

      pthread_mutex_lock(&auction_w_lock);
      InsertAtTail(auction_list, (void*)auction);
      pthread_mutex_unlock(&auction_w_lock);

      read = getline(&line, &length, file);
    }
    fclose(file);

    //pthread_t tid;

    for(int i = 0; i<job_num; i++)
    {   
        //printf("asdf");
        pthread_create(&tid, NULL, job_threads, NULL); 
        //pthread_detach(tid);
    }

    // Make tick thread
    pthread_create(&tid, NULL, tick_thread, (void*) tick_seconds);
    
    run_server(port);

    
    return 0;
}