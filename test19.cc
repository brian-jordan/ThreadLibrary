#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
using namespace std;

unsigned int TAJohnLock;
unsigned int waitingStudent;
unsigned int waitingJohn;

int questionBank; //has room only space for 1


void John(void *a){
  thread_yield();
  if(thread_lock(TAJohnLock)){
    cout << "thread_lock failed\n";
  }
  //take question out 

  while(questionBank==0){
    cout << "no questions John must wait\n";
  	if(thread_wait(TAJohnLock,waitingJohn)){
        cout << "thread_wait failed\n";

    }
  }
  cout << "number of questions" << questionBank << endl;
  questionBank=questionBank-1;
  thread_yield();
  cout << "John has answered a question";
  if(thread_signal(TAJohnLock, waitingStudent)){
      cout << "thread_signal failed\n";
  }
  cout << "signaled waiting students" << endl;

  thread_yield();
   cout << "John has unlocked" << endl;
  if(thread_unlock(TAJohnLock)){
      cout << "thread_unlock failed\n";
  }

}


void student(void *a){
  if(thread_lock(TAJohnLock)){
    cout << "thread_lock failed\n";
  }
  thread_yield();
  cout << "student has acquired a lock to ask a question" << questionBank << endl;
  //put question in 
  while(questionBank==1){
    //he's busy 
    cout << "student waits because john is busy" << questionBank << endl;
    if(thread_wait(TAJohnLock,waitingStudent)){
        cout << "thread_wait failed\n";

    } 
     }
    thread_yield();

  cout << "number of questions" << questionBank << endl;
  questionBank=questionBank+1;
  cout << "Student has asked a question:" << endl;
  if(thread_signal(TAJohnLock, waitingJohn)){
     cout << "thread_signal failed\n";
  }
  cout << "student has signaled john\n";

  thread_unlock(TAJohnLock);

}


void start(void *a) {

  if (thread_create((thread_startfunc_t) John, (void *) NULL)) {
      cout << "thread_create failed\n";
      }
  if (thread_create((thread_startfunc_t) student, (void *) NULL)) {
        cout << "thread_create failed\n";
      }
  if (thread_create((thread_startfunc_t) student, (void *) NULL)) {
        cout << "thread_create failed\n";
      }


}

int main() {
	TAJohnLock=0;
	waitingStudent=0;
	waitingJohn=1;
	questionBank=0; //has room only space for 1



  if (thread_libinit( (thread_startfunc_t) start, (void *) 4)) {
    cout << "thread_libinit failed\n";
    //should end with 5 yogurts
    exit(1);
  }

}
