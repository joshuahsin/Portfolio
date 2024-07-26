

#include <string>
#include <vector>
#include <iostream>
#include <typeinfo>

template<typename Key, typename Value>
class MyAVLTree
{
private:
	struct node{			//Each node in AVL Tree has a height, key - which it is sorted by, value, parent node, left child node, and right child node
		int height;
		Key key;
		Value value;
		node* parent;
		node* leftchild;
		node* rightchild;
	};
    size_t count; 	//number of nodes
    size_t height;		//overall tree height
    std::vector<node*> added_nodes;	//vector of all added nodes

public:
    node* rootNode;
    MyAVLTree();
    ~MyAVLTree()
    {
    }

    size_t size() const; //returns the number of distinct keys in the tree.
    bool isEmpty() const; //returns true if tree has no nodes
    bool contains(const Key & k) const;	//returns tree if node with key is in tree
    bool checkBalence(node* currentNode);	//check if left and right side of current node have the same height +/- 1
    Value & find(const Key & k);
    const Value & find(const Key & k) const;		//returns value of node with given key

    void insert(const Key & k, const Value & v);	//insert a node with the given key and value
    void Rotate(node* startingNode);	//Rotate nodes to balance tree
    void increment(const Key & k);		//increment value of node with a certain key

    //print nodes in a certain order by key
    void inOrderTraversal(node* node) const;
    void preOrder(node* node) const;
    void postOrder(node* node) const;

   };

	template<typename Key, typename Value>
	MyAVLTree<Key, Value>::MyAVLTree()
	{
		count = 0;
		height = 0;
		rootNode = new node;
		rootNode->height = -1;
		rootNode->parent = nullptr;		//set count, height, right_height, and left_height to 0
										//create a new empty rootNode with a height -1 and null parent
	}


   template<typename Key, typename Value>
   size_t MyAVLTree<Key, Value>::size() const
   {
       return count; //return size
   }

   template<typename Key, typename Value>
   bool MyAVLTree<Key, Value>::isEmpty() const
   {
       if(rootNode->height == -1){
    	   return true;
       }
       return false;	//if tree empty return tree, else, return false
   }



   template<typename Key, typename Value>
   bool MyAVLTree<Key, Value>::contains(const Key & k) const
   {
       for(int i = 0; i < added_nodes.size(); i++)
       {
           if(added_nodes[i]->key == k)
           {
               return true;
           }
       }
       return false;	//If tree contains key, return tree, else, return false
   }

   template<typename Key, typename Value>
   const Value & MyAVLTree<Key, Value>::find(const Key & k) const
   {
		for(int i = 0; i < added_nodes.size(); i++){
			if(added_nodes[i]->key == k){
				return added_nodes[i]->value;
			}
		}
		std::cout << "none" << std::endl;
		exit(EXIT_FAILURE);	//return value of node with given key if possible, else exit
   }

   template<typename Key, typename Value>
   Value & MyAVLTree<Key, Value>::find(const Key & k)
   {
		for(int i = 0; i < added_nodes.size(); i++){
			if(added_nodes[i]->key == k){
				return added_nodes[i]->value;
			}
		}
		std::cout << "none" << std::endl;
		exit(EXIT_FAILURE);	//return value of node with given key if possible, else exit
   }

   template<typename Key, typename Value>
   void MyAVLTree<Key, Value>::insert(const Key & k, const Value & v)
   {
	   node* insert_node;
	   insert_node = new node;
	   insert_node->height = 0;
	   insert_node->key = k;
	   insert_node->value = v;				//create a new node with given key and value
	   node* right_child = new node;		//create empty right child
	   right_child->height = -1;			//set right child height to -1
	   right_child->parent = insert_node;	//set parent of right child to node being inserted
	   node* left_child = new node;			//create empty left child
	   left_child->height = -1;				//set left child height to -1
	   left_child->parent = insert_node;	//set parent of left child to node being inserted
	   insert_node->rightchild = right_child;
	   insert_node->leftchild = left_child;		//set children of node being inserted

	   if(count == 0){
		   rootNode = insert_node;			//if first node being inserted, set it equal to root node
		   rootNode->parent = nullptr;		//set parent of root node to nullptr
	   }
	   else{
		   node* tracking_node = new node;
		   tracking_node = rootNode;		//create a new tracking node and set it equal to root node
		   while(tracking_node->height != -1){					//while height of tracking node is != -1 (not at bottom of tree)
			   if(insert_node->key < tracking_node->key){		//if key of insert node is less than current tracking node, move tracking node left
				   tracking_node = tracking_node->leftchild;
			   }
			   else if(insert_node->key > tracking_node->key){	//if key of insert node is more than current tracking node, move tracking node right
				   tracking_node = tracking_node->rightchild;
			   }
		   }
		   if(tracking_node->parent->key < insert_node->key){	//if insert node key is greater than parent key
			   tracking_node->parent->rightchild = insert_node;		//set right child of parent to insert node
			   insert_node->parent = tracking_node->parent;			//set parent of insert node to parent
		   }
		   else if(tracking_node->parent->key > insert_node->key){	//if insert node key is less than parent key
			   tracking_node->parent->leftchild = insert_node;		//set left child of parent to insert node
			   insert_node->parent = tracking_node->parent;			//set parent of insert node to parent
		   }
		   //set height of inserted node
		   node* current_node;
		   current_node = new node;
		   current_node = insert_node;
		   while(current_node->parent != nullptr){					//if inserted node is not root node
			   current_node = current_node->parent;					//set current node to parent of inserted node
			   if(current_node->rightchild && current_node->leftchild){
				   current_node->height = std::max(current_node->rightchild->height, current_node->leftchild->height) + 1;
			   }//if current node has rightchild and leftchild, height of current node is equal to the max of the heights of the left and right child plus 1
			   else if(!current_node->rightchild && current_node->leftchild){
				   current_node->height = current_node->leftchild->height + 1;
			   }//else if current node has no right child, set height of current node to the height of the node's left child + 1
			   else{
				   current_node->height = current_node->rightchild->height + 1;
			   }//else if current node has no left child, set height of current node to the height of the node's right child + 1
		   }
		   }
		   if(!checkBalence(rootNode)){
			   std::cout << "Not balenced, rotating" << std::endl;
			   Rotate(insert_node);//if tree is not balanced, rotate tree at appropriate node
		   }
		   added_nodes.push_back(insert_node);//add inserted node to vector of all nodes
		   count++;//increment node count
		   return;
		}

	template<typename Key, typename Value>
	void MyAVLTree<Key, Value>::inOrderTraversal(node* node) const
	{
	  if(node->height == -1){
		  return;
	  }

	  inOrderTraversal(node->leftchild);

	  std::cout <<"node "<< node->key << std::endl;

	  inOrderTraversal(node->rightchild);
	}//prints tree nodes in order of key - left, root, right


	template<typename Key, typename Value>
	void MyAVLTree<Key, Value>::preOrder(node* node) const
	{
	  if(node->height == -1){
		  return;
	  }

	  std::cout << "node " << node->key << std::endl;

	  preOrder(node->leftchild);

	  preOrder(node->rightchild);
	}//prints tree nodes in order of root, left, right

	template<typename Key, typename Value>
	void MyAVLTree<Key, Value>::postOrder(node* node) const
	{
		if(node->height == -1){
		  return;
		}

		postOrder(node->leftchild);

		postOrder(node->rightchild);

		std::cout << "node " << node->key << std::endl;
	}//prints tree nodes in order of left, right, root

	template<typename Key, typename Value>
	void MyAVLTree<Key, Value>::Rotate(node* startingNode)
	{
	node* X;
	X = new node;
	X = startingNode;
	node* Y;
	Y = new node;
	Y = startingNode->parent;
	node* Z;
	Z = new node;
	Z = startingNode->parent->parent;		//Create nodes X - starting node, Y - parent of starting node, Z - parent of Y

	bool stop = false;
	while(!stop){
		if(Z->parent == nullptr){			//If Z's parent if a nullpointer stop
			stop = true;
		}
		else if(abs(Z->rightchild->height - Z->leftchild->height) <= 1){
			Z = Z->parent;
			X = X->parent;
			Y = Y->parent;
		}//if different in height of Z's right and left child is greater than one, both X, Y, and Z up the tree
		else{
			stop = true;					//else stop
		}
	}
	if(Y->key < X->key && X->key < Z->key){	//if key of Y is less than X and key of Z is greater than X
		node* tree0;
		tree0 = new node;
		tree0 = Y->leftchild;
		node* tree1;
		tree1 = new node;
		tree1 = X->leftchild;
		node* tree2;
		tree2 = new node;
		tree2 = X->rightchild;
		node* tree3;
		tree3 = new node;
		tree3 = Z->rightchild;		//Create 4 trees with the root nodes being the left child of Y, the left child of X, right child of X, and the right child of Z

		if(Z->parent != nullptr){						//if Z is not the root node
		  if(Z->parent->rightchild->key == Z->key){		//if Z is the right child of Z's parent
			  X->parent = Z->parent;					//Set X's parent to Z's parent
			  Z->parent->rightchild = X;				//Set Z's parents right child to X
		  }
		  else if(Z->parent->leftchild->key == Z->key){	//if Z is the left child of Z's parent
			  X->parent = Z->parent;					//Set X's parent to Z's parent
			  Z->parent->leftchild = X;					//Set Z's left child to X
		  }
		}
		else{											//else if Z is root node
		  rootNode = X;									//Set the root node to X
		  X->parent = nullptr;
		}
		X->leftchild = Y;
		Y->parent = X;
		Y->leftchild = tree0;
		Y->rightchild = tree1;							//Set Y as the left child of X and set children of Y to appropriate trees
		X->rightchild = Z;
		Z->parent = X;
		Z->leftchild = tree2;
		Z->rightchild = tree3;							//Set Z as the right child of Z and children of Z to appropriate trees

		X->height += 1;
		Y->height -= 1;
		Z->height -= 2;									//update heights
		node* current_node;
		current_node = new node;
		current_node = X;
		while(current_node->parent != nullptr){			//update heights of everything above X
		   current_node->parent->height -= 1;
		   current_node = current_node->parent;
		}
	}
	else if(X->key < Y->key && Y->key < Z->key){		//if key of X is less than Y and key of Y is greater than Z
		node* tree0;
		tree0 = new node;
		tree0 = X->leftchild;
		node* tree1;
		tree1 = new node;
		tree1 = X->rightchild;
		node* tree2;
		tree2 = new node;
		tree2 = Y->rightchild;
		node* tree3;
		tree3 = new node;
		tree3 = Z->rightchild;							//Create 4 trees with the root nodes being the left child of X, the right child of X, right child of Y, and the right child of Z
		if(Z->parent != nullptr){						//if Z is not root node
		  if(Z->parent->rightchild->key == Z->key){		//if Z is the right child of its parent
			  Y->parent = Z->parent;					//Set Y's parent to Z's parent
			  Z->parent->rightchild = Y;				//Set Z's parent's right child to Y
		  }
		  else if(Z->parent->leftchild->key == Z->key){	//if Z is the left child of its parent
			  Y->parent = Z->parent;					//Set Y's parent to Z's parent
			  Z->parent->leftchild = Y;					//Set Z's parent's right child to Y
		  }
		}
		else{											//else if Z is root node
		  rootNode = Y;									//set root node to Y
		  Y->parent = nullptr;
		}

		Y->leftchild = X;
		X->parent = Y;
		X->leftchild = tree0;
		tree0->parent = X;
		X->rightchild = tree1;
		tree1->parent = X;								//Set X as the left child of Y and set children of X to appropriate trees
		Y->rightchild = Z;
		Z->parent = Y;
		Z->leftchild = tree2;
		tree2->parent = Z;
		Z->rightchild = tree3;
		tree3->parent = Z;								//Set Z as the right child of Y and set children of Z to appropriate trees

		Z->height -= 2;									//update heights
		node* current_node;
		current_node = new node;
		current_node = Y;
		while(current_node->parent != nullptr){			//update heights of everything above Y
		 current_node->parent->height -= 1;
		 current_node = current_node->parent;
		}
	}
	else if(Z->key < X->key && X->key < Y->key){		//if key of Z is less than X and key of X is greater than Y
		node* tree0;
		tree0 = new node;
		tree0 = Z->leftchild;
		node* tree1;
		tree1 = new node;
		tree1 = X->leftchild;
		node* tree2;
		tree2 = new node;
		tree2 = X->rightchild;
		node* tree3;
		tree3 = new node;
		tree3 = Y->rightchild;							//Create 4 trees with the root nodes being the left child of Z, the left child of X, right child of X, and the right child of Y
		if(Z->parent != nullptr){						//if Z is not root node
		  if(Z->parent->rightchild->key == Z->key){		//if Z is the right child of its parent
			  X->parent = Z->parent;					//Set parent of X to parent of Z
			  Z->parent->rightchild = X;				//Set right child of Z's parent to X
		  }
		  else if(Z->parent->leftchild->key == Z->key){	//if Z is the left child of its parent
			  X->parent = Z->parent;					//Set parent of X to parent of Z
			  Z->parent->leftchild = X;					//Set left child of Z's parent to X
		  }
		}
		else{											//if Z is the root node
		  rootNode = X;									//Set the root node equal to X
		  X->parent = nullptr;
		}
		X->leftchild = Z;
		Z->parent = X;
		Z->leftchild = tree0;
		tree0->parent = Z;
		Z->rightchild = tree1;
		tree1->parent = Z;								//Set Z as the left child of X and set children of Z to appropriate trees
		X->rightchild = Y;
		Y->parent = X;
		Y->leftchild = tree2;
		tree2->parent = Y;
		Y->rightchild = tree3;
		tree3->parent = Y;								//Set Y as the right child of X and set children of X to appropriate trees

		X->height += 1;
		Y->height -= 1;
		Z->height -= 2;									//update heights

		node* current_node;
		current_node = new node;
		current_node = X;
		while(current_node->parent != nullptr){
		 current_node->parent->height -= 1;
		 current_node = current_node->parent;
		}												//update heights of everything above X
	}
	else if(Z->key < Y->key && Y->key < X->key){		//if key of Z is less than Y and key of X is greater than Y
		node* tree0;
		tree0 = new node;
		tree0 = Z->leftchild;
		node* tree1;
		tree1 = new node;
		tree1 = Y->leftchild;
		node* tree2;
		tree2 = new node;
		tree2 = X->leftchild;
		node* tree3;
		tree3 = new node;
		tree3 = X->rightchild;							//Create 4 trees with the root nodes being the left child of Z, the left child of Y, right child of X, and the left child of X
		if(Z->parent != nullptr){						//if Z is not root node
		  if(Z->parent->rightchild->key == Z->key){		//if Z is the right child of its parent
			  Y->parent = Z->parent;					//Set parent of Y to parent of Z
			  Z->parent->rightchild = Y;				//Set right child of Z's parent to Y
		  }
		  else if(Z->parent->leftchild->key == Z->key){	//if Z is the left child of its parent
			  Y->parent = Z->parent;					//Set parent of Y to parent of Z
			  Z->parent->leftchild = Y;					//Set left child of Z's parent to Y
		  }
		}
		else{											//else if Z is root node
		  rootNode = Y;									//Set root node equal to Y
		  Y->parent = nullptr;
		}
		Y->leftchild = Z;
		Z->parent = Y;
		Z->leftchild = tree0;
		tree0->parent = Z;
		Z->rightchild = tree1;
		tree1->parent = Z;								//Set Z as the left child of Y and set children of Z to appropriate trees
		Y->rightchild = X;
		X->parent = Y;
		X->leftchild = tree2;
		tree2->parent = X;
		X->rightchild = tree3;
		tree3->parent = X;								//Set X as the right child of Y and set children of X to appropriate trees

		Z->height -= 2;									//update heights
		node* current_node;
		current_node = new node;
		current_node = Y;
		while(current_node->parent != nullptr){
		 current_node->parent->height -= 1;
		 current_node = current_node->parent;
		}												//update heights of everything above Y
	}
	}

	template<typename Key, typename Value>
	bool MyAVLTree<Key, Value>::checkBalence(node* currentNode)
	{
		if(currentNode->height == -1){
			return true;																				//return true if you reach the bottom
		}
		else{
			if(abs(currentNode->rightchild->height - currentNode->leftchild->height) > 1){
				return false;																			//if height of left and right child differ by more than one, tree is unbalanced
			}
			else{
				return checkBalence(currentNode->rightchild) && checkBalence(currentNode->leftchild);	//recursively check balance until you find unbalanced children or reach the bottom
			}
		}
	}

	template<typename Key, typename Value>
	void MyAVLTree<Key, Value>::increment(const Key & k)												//increment the value of a node with a certain key
	{
		for(int i = 0; i < added_nodes.size();i++){
			if(added_nodes[i]->key == k){
				if(!std::strcmp(typeid(added_nodes[i]->key).name(), "i")){
					added_nodes[i]->value += 1;															//if value is type integer, add 1
				}else{
					std::cout << "Cannot increment " << typeid(Value).name() << " type" << std::endl;	//else output error
				}
			}
		}
	}









