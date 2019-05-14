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
  if (thread_lock(lock)) {
      cout << "thread_lock failed\n";
    }
  cout << "lock1\n";
  friends=friends+1;

  if (thread_lock(lock)) {
      cout << "thread_lock failed\n";
    }
  cout << "lock2\n";

  if (thread_unlock(lock)) {
      cout << "thread_unlock failed\n";
    }
  cout << "unlocking\n";
  //cout << "unlock1\n";

}



void start(void *a) {
    if (thread_create((thread_startfunc_t) child, (void *) NULL)) {
      cout << "thread_create failed\n";
    }
   }

int main() {
  friends=0;
  lock=0;


  if (thread_libinit( (thread_startfunc_t) start, (void *) 4) ){
    cout << "thread_libinit failed\n";
    //should end with 5 yogurts
  }
}
