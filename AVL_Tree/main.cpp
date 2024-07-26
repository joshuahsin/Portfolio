/*
 * main.cpp
 *
 *  Created on: Feb 20, 2020
 *      Author: joshuahsin
 */
#include "proj4schindler.cpp"
#include <string>
int main(){
	MyAVLTree<int, std::string> a;
	std::cout << a.isEmpty() << std::endl;
	a.insert(2, "c");
	std::cout << a.contains(3) << std::endl;
	std::cout << a.rootNode->key << std::endl;
	std::cout << a.isEmpty() << std::endl;
	a.insert(1, "b");
	a.insert(3, "a");
	std::cout << a.contains(3) << std::endl;
	std::cout << a.find(3) << std::endl;
	a.insert(4, "a");
	a.insert(5, "a");
	a.inOrderTraversal(a.rootNode);
	a.preOrder(a.rootNode);
	a.postOrder(a.rootNode);
	return 0;
}




