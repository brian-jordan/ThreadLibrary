#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
using namespace std;

//waiting on wrong long

unsigned int lock;
int friends;
unsigned int cond;
unsigned int lock2;
unsigned int cond2;


void child(void *a){
  if (thread_lock(lock)) {
      cout << "thread_lock failed\n";
    }
  cout << "acquired the lock and about to yeild\n";

  if (thread_yield()) {
      cout << "thread_yeild failed\n";
    }
  //thread_yield();
    cout << "back to child1 but should fail before this\n";
  if (thread_unlock(lock)) {
      cout << "thread_unlock failed\n";
    }
  //cout << "unlock1\n";

}

void child2(void *a){
  cout << "child 2 turn, don't have lock but about to call wait holding wrong lock\n";
  if (thread_lock(lock2)) {
      cout << "thread_lock failed\n";
    }
    if (thread_wait(lock, cond)) {
      cout << "thread_wait failed\n";
    }
  if (thread_unlock(lock2)) {
      cout << "thread_lock failed\n";
    }

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
  friends=0;
  lock=0;
  cond=0;
  lock2=1;
  cond2=1;


  if (thread_libinit( (thread_startfunc_t) start, (void *) 4) ){
    cout << "thread_libinit failed\n";
    //should end with 5 yogurts
  }
}
