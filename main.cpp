#include <iostream>
#include "skiplist.h"

int main() {

    Skiplist<int, std::string> Skiplist(6);
	Skiplist.insert_element(1, "a"); 
	Skiplist.insert_element(3, "b"); 
	Skiplist.insert_element(7, "c"); 
	Skiplist.insert_element(8, "d"); 
	Skiplist.insert_element(9, "e"); 
	Skiplist.insert_element(19, "f"); 
	Skiplist.insert_element(19, "g"); 

    std::cout << "Skiplist size:" << Skiplist.get_size() << std::endl;

    Skiplist.search_element(9);
    Skiplist.search_element(18);

    Skiplist.dump_file();

    Skiplist.display_list();

    Skiplist.delete_element(3);
    Skiplist.delete_element(7);

    std::cout << "Skiplist size:" << Skiplist.get_size() << std::endl;

    Skiplist.display_list();

    return 0;
}