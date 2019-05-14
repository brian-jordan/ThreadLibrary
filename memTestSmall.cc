#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <map>
using namespace std;



// Maker Method for making orders

void newThread(void* arg) {
	printf("hello");

}

void threadCreator(void* arg){

	for(int i = 0; i < 10; i++){
		if (thread_create(((thread_startfunc_t) newThread), (void*) NULL)){
			cout << "thread_create failed \n";
			exit(1);
		}
	}

}

int main(){

	// Initiates maker thread with no argument
	if (thread_libinit(((thread_startfunc_t) threadCreator), (void*) NULL)){
		cout << "thread_libinit failed \n";
		exit(1);
	}

	


}
