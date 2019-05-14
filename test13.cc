#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
using namespace std;

//deadlock

unsigned int lock1;
unsigned int lock2;



void child(void *a){
  if(thread_lock(lock1)){
    cout << "thread_lock failed\n";
  }
  cout << "acquired resrouce 1\n";
  if(thread_yield()){
    cout << "thread_yield failed\n";
  }
  //thread_yield();
  if(thread_lock(lock2)){
    cout << "thread_lock failed\n";
  }
  cout << "acquired resource 2\n";

  if(thread_unlock(lock2)){
    cout << "thread_unlock failed\n";
  }
  cout << "finished resource 2\n";

  if(thread_unlock(lock1)){
    cout << "thread_unlock failed\n";
  }
  cout << "finished resource 1\n";
  //cout << "unlock1\n";

}
void child2(void *a){
  if(thread_lock(lock2)){
    cout << "thread_lock failed\n";
  }
  cout << "acquired resource 2\n";
  if(thread_yield()){
    cout << "thread_yeild failed\n";
  }
  //thread_yield();
  if(thread_lock(lock1)){
    cout << "thread_lock failed\n";
  }
  cout << "acquired resource 1\n";

  if(thread_unlock(lock1)){
    cout << "thread_unlock failed\n";
  }
  cout << "finished resource 1\n";

  if(thread_unlock(lock2)){
    cout << "thread_unlock failed\n";

  }
  cout << "finished resrouce 2\n";

}



void start(void *a) {
    if (thread_create((thread_startfunc_t) child, (void *) NULL)) {
      cout << "thread_create failed\n";
    }
    if (thread_create((thread_startfunc_t) child2, (void *) NULL)) {
      cout << "thread_create failed\n";
    }
   }

int main() {
  lock1=0;
  lock2=1;


  if (thread_libinit( (thread_startfunc_t) start, (void *) 4) ){
    cout << "thread_libinit failed\n";
    //should end with 5 yogurts
  }
}
