#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
using namespace std;

unsigned int lock;
unsigned int cv;

int i;



// Maker Method for making orders

void thread1(void* arg) {

	fprintf(stderr, "In thread1\n");

	i = 0;

	thread_lock(lock);

	fprintf(stderr, "Locked in thread1\n");

	//thread_signal(lock,cv);

	thread_unlock(lock);

}

void thread2(void* arg){

	fprintf(stderr, "waiting in thread2\n");

	while(int i=0){
		thread_wait(lock,cv);
		i++;
	}

	fprintf(stderr, "done waiting in thread2\n");

	thread_unlock(lock);
}

void threadCreator(void* arg){

	fprintf(stderr, "Creating threads\n");

	if (thread_create(((thread_startfunc_t) thread1), ((void*) NULL))){
		cout << "thread_create failed \n";
		exit(1);
	}

	if (thread_create(((thread_startfunc_t) thread2), ((void*) NULL))){
		cout << "thread_create failed \n";
		exit(1);
	}

	thread_lock(lock);

	fprintf(stderr, "creator: yielding for first time\n");

	thread_yield();

	fprintf(stderr, "creator: calling signal\n");

	thread_signal(lock, cv);

	fprintf(stderr, "creator: calling unlock\n");

	thread_unlock(lock);

}

int main(){

	cv = 1;

	lock = 0;

	// Initiates maker thread with no argument
	if (thread_libinit(((thread_startfunc_t) threadCreator), (void*) NULL)){
		cout << "thread_libinit failed \n";
		exit(1);
	}

	


}
