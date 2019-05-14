#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
using namespace std;

//broadcast test

unsigned int TAJohnLock;
unsigned int waitingStudents;
int teacherHere;
int studentsInside;


void John(void *a){
  if(thread_lock(TAJohnLock)){
    cout << "thread_lock failed\n";
  }
 cout << "teacher here!!!!" << endl;
  teacherHere=teacherHere+1;
  if(thread_broadcast(TAJohnLock, waitingStudents)){
    cout << "thread_broadcast failed" << endl;
  }
  cout << "teacher broadcasted" << endl;
  if(thread_yield()){
    cout << "thread_yeild failed\n";
  }
  cout << "teacher should still run next cause its holding the lock" << endl;
  if(thread_unlock(TAJohnLock)){
    cout << "thread unlock failed" << endl;
  }

}


void student(void *a){
  if(thread_lock(TAJohnLock)){
    cout << "thread lock failed" << endl;
  }
  cout << "acquired lock" << endl;
  if (teacherHere==0){
    cout << "no teachers so waiting" << endl;
    thread_wait(TAJohnLock, waitingStudents);
    cout << "out of wait" << endl;
  }
  studentsInside=studentsInside+1;
  cout << "number of students inside" << studentsInside << endl;

  cout << "I'm inside\n";
  thread_unlock(TAJohnLock);
}


void start(void *a) {

   for(int i=0; i<3; i++){
    if (thread_create((thread_startfunc_t) student, (void *) NULL)) {
      cout << "thread_create failed\n";
    }
  }

  if (thread_create((thread_startfunc_t) John, (void *) NULL)) {
      cout << "thread_create failed\n";
      }


   }

int main() {
  TAJohnLock=0;
  waitingStudents=0;
  teacherHere= 0;
  studentsInside=0;
  cout << studentsInside;


  if (thread_libinit( (thread_startfunc_t) start, (void *) 4) ){
    cout << "thread_libinit failed\n";
    //should end with 5 yogurts
  }


}
