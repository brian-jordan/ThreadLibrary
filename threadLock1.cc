#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
using namespace std;

//locking when it already has the lock

unsigned int lock;
int friends;


void child(void *a){
  thread_lock(lock);
  cout << "lock1\n";
  friends=friends+1;

  thread_lock(lock);
  cout << "lock2\n";
  //should throw an error
  thread_unlock(lock);
  cout << "unlock1\n";

}



void start(void *a) {
    if (thread_create((thread_startfunc_t) child, (void *) NULL)) {
      cout << "thread_create failed\n";
      exit(1);
    }
   }

int main() {
  friends=0;
  lock=0;


  if (thread_libinit( (thread_startfunc_t) start, (void *) 4) ){
    cout << "thread_libinit failed\n";
    //should end with 5 yogurts
    exit(1);
  }
}