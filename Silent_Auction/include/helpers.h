#ifndef HELP_H
#define HELP_H
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "protocol.h"

#define USAGE_TEXT "./bin/zbid_server [-h] [-j N] [-t M] PORT_NUMBER AUCTION_FILENAME\n\
\n-h                 Displays this help menu, and returns EXIT_SUCCESS.\
\n-j N               Number of job thread. If option not specified, default to 2.\
\n-t M               M seconds between time ticks. If option not specified, defualt is to wait on\
\n                   input from stdin to indicate a tick.\
\nPORT_NUMBER        Port number to listen on.\
\nAUCTION_FILENAME   File to read auction item information from the start of the server.\n"

int getIntLen(int x)
{   
    int i =1;
    if (x<0) {i++;}

    for (x; x >0; x/=10)
    {
        i++;
    }
    return i;
}





#endif