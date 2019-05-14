#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
using namespace std;



// Maker Method for making orders


unsigned int totalYogurt;
int yogurtCount;


void MollyEats(void *a) {
  thread_yield();
  //lock the total yogurt 
  //add the total yogurt 
  //release the lock 
  thread_lock(totalYogurt);
  thread_yield();
  yogurtCount=yogurtCount+1;
  thread_yield();
  cout << "Molly ate a yogurt" << endl;
  cout << "Yogurt count" << yogurtCount << endl;

  thread_unlock(totalYogurt);

}

void BryanWantsToKnow(void *a){
  thread_yield();
  thread_lock(totalYogurt);
  cout << "Bryan Asks Molly how many yogurts she's eaten. She says:" << yogurtCount << endl;
  thread_yield();
  thread_unlock(totalYogurt);

}


void start(void *a) {
  if (thread_create((thread_startfunc_t) MollyEats, (void *) NULL)) {
      cout << "thread_create failed\n";
      }
  if (thread_create((thread_startfunc_t) MollyEats, (void *) NULL)) {
      cout << "thread_create failed\n";
      }
  if (thread_create((thread_startfunc_t) BryanWantsToKnow, (void *) NULL)) {
        cout << "thread_create failed\n";
      }
  if (thread_create((thread_startfunc_t) MollyEats, (void *) NULL)) {
      cout << "thread_create failed\n";
      }
  if (thread_create((thread_startfunc_t) BryanWantsToKnow, (void *) NULL)) {
        cout << "thread_create failed\n";
      }

}

int main() {
  totalYogurt=0;
  yogurtCount=0;

  if (thread_libinit( (thread_startfunc_t) start, (void *) 5)) {
    cout << "thread_libinit failed\n";
    //should end with 5 yogurts
    exit(1);
  }
}
