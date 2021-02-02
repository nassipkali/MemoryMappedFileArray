#include "MemoryMappedFileArray.hpp"
#include <iostream>
#include <ctime>
#include <vector>

int main() {
    srand(time(0));
    std::cout << "Insertion Test" << std::endl;
    std::vector <int> arr;
    for(int i = 0; i < 10000; i++) {
	int num = rand() % RAND_MAX;
	arr.push_back(num);
    }
    MemoryMappedFileArray <int> mmfa("itest");


    bool test1 = true;
    bool test2 = true;

    for(int i = 0; i < 1000; i++) {
	mmfa.Add(arr[i]);
    }

    for(int i = 0; i < 1000; i++) {
	if(mmfa[i] != arr[i]) {
	    test1 = false;
	    break;
	}
    }
    



    if(test1) {
	std::cout << "Test #1: PASS" << std::endl;
    }
    else {
	std::cout << "Test #1: FAILED" << std::endl;
    }

    for(int i = 1000; i < 10000; i++) {
	mmfa.Add(arr[i]);
    }
    for(int i = 1000; i < 10000; i++) {
	if(mmfa[i] != arr[i]) {
	    test2 = false;
	    break;
	}
    }
    

    if(test2) {
	std::cout << "Test #2: PASS" << std::endl;
    }
    else {
	std::cout << "Test #2: FAILED" << std::endl;
    }

    mmfa.Resize(0);
}
