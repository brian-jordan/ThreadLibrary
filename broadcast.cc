#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
using namespace std;

unsigned int TAJohnLock;
unsigned int waitingStudents;
int teacherHere;


void John(void *a){
  thread_lock(TAJohnLock);
  teacherHere=teacherHere+1;
  thread_broadcast(TAJohnLock, waitingStudents);
  cout << "teacher here" << endl;
  thread_unlock(TAJohnLock);

}


void student(void *a){
  thread_lock(TAJohnLock);
  if (teacherHere==0){
    thread_wait(TAJohnLock, waitingStudents);
  }

  cout << "I'm in class! " << endl;
  thread_unlock(TAJohnLock);
}


void start(void *a) {

   for(int i=0; i<3; i++){
    if (thread_create((thread_startfunc_t) student, (void *) NULL)) {
      cout << "thread_create failed\n";
      exit(1);
    }
  }

  if (thread_create((thread_startfunc_t) John, (void *) NULL)) {
      cout << "thread_create failed\n";
      exit(1);
      }


   }

int main() {
  TAJohnLock=0;
  waitingStudents=0;
  teacherHere= 0;


  if (thread_libinit( (thread_startfunc_t) start, (void *) 4) ){
    cout << "thread_libinit failed\n";
    //should end with 5 yogurts
    exit(1);
  }
}
