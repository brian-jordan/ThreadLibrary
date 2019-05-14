#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
using namespace std;

//calling thread libiinit within itself 

unsigned int lock;
int friends;


void child(void *a){
  thread_lock(lock);
  friends=friends+1;
  thread_unlock(lock);

}



void start(void *a) {
  if (thread_libinit( (thread_startfunc_t) child, (void *) 4) ){
    cout << "thread_libinit failed because called twice\n";
    //should end with 5 yogurts
  }
  cout << "Should not get here\n";

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
