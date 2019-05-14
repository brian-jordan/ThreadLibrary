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

	if (thread_libinit(((thread_startfunc_t) thread1), (void*) NULL) == -1){
		cout << "thread_libinit failed\n";
		exit(1);
	}
	else exit(0);

}

int main(){

	cv = 1;

	lock = 0;

	if (thread_lock(lock) == -1){
		cout << "thread_lock failed\n";
		exit(1);
	}

	if (thread_unlock(lock) == -1){
		cout << "thread_unlock failed\n";
		exit(1);
	}

	if (thread_signal(lock,cv) == -1){
		cout << "thread_signal failed\n";
		exit(1);
	}

	if (thread_broadcast(lock,cv) == -1){
		cout << "thread_broadcast failed\n";
		exit(1);
	}

	if (thread_wait(lock,cv) == -1){
		cout << "thread_wait failed\n";
		exit(1);
	}

	if (thread_yield(lock,cv) == -1){
		cout << "thread_yield failed\n";
		exit(1);
	}


	if (thread_create(((thread_startfunc_t) thread1), (void*) NULL) == -1){
		cout << "thread_create failed\n";
		exit(1);
	}

	// Initiates maker thread with no argument
	if (thread_libinit(((thread_startfunc_t) thread1), (void*) NULL)){
		cout << "thread_libinit failed\n";
		exit(1);
	}
}