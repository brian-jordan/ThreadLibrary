#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
using namespace std;

unsigned int lock;
unsigned int cv;



// Maker Method for making orders

void thread1(void* arg) {

	if (thread_unlock(lock) == -1){
		cout << "thread_unlock failed\n";
		exit(1);
	}

	else exit(0);

}

int main(){

	cv = 1;

	lock = 0;

	// Initiates maker thread with no argument
	if (thread_libinit(((thread_startfunc_t) thread1), (void*) NULL)){
		cout << "thread_libinit failed\n";
		exit(1);
	}
}