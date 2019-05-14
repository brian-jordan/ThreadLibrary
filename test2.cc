#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
using namespace std;

//not initalising libinit first 

unsigned int lock;
int friends;


void child(void *a){
  if (thread_lock(lock)) {
      cout << "thread_lock failed\n";
    }

  friends=friends+1;
  //should throw an error
  if (thread_unlock(lock)) {
      cout << "thread_unlock failed\n";
    }

}

int main() {
  friends=0;
  lock=0;
  cout << "calling thread create without libinit d\n";
  if (thread_create((thread_startfunc_t) child, (void *) NULL)) {
      cout << "FAILED LIKE SUPPOSED TO!\n";
    }
  }
