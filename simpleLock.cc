#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
using namespace std;



// Maker Method for making orders


unsigned int totalYogurt;


void MollyEats(void *a) {
  //lock the total yogurt 
  //add the total yogurt 
  //release the lock 
  thread_lock(totalYogurt);
  totalYogurt=totalYogurt+1;
  cout << "Molly ate a yogurt" << endl;
  thread_unlock(totalYogurt);

}

void BryanWantsToKnow(void *a){
  thread_lock(totalYogurt);
  totalYogurt=totalYogurt;
  cout << "Bryan Asks Molly how many yogurts she's eaten. She says:" << totalYogurt << endl;
  thread_unlock(totalYogurt);

}


void start(void *a) {
  int arg;
  arg = (long int) a;

  cout << "start called with arg " << arg << endl;

  for(int i=0; i<arg; i++){
    if (thread_create((thread_startfunc_t) MollyEats, (void *) NULL)) {
      cout << "thread_create failed\n";
      exit(1);
      }
    if(i==2){
      if (thread_create((thread_startfunc_t) BryanWantsToKnow, (void *) NULL)) {
        cout << "thread_create failed\n";
        exit(1);
      }
    }
  }
  BryanWantsToKnow( (void *) NULL);

}

int main() {
  totalYogurt=0;

  if (thread_libinit( (thread_startfunc_t) start, (void *) 5)) {
    cout << "thread_libinit failed\n";
    //should end with 5 yogurts
    exit(1);
  }
}
