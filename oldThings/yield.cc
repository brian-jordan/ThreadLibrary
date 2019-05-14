#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>


//readyList = my ready queue. It is linked list of structs. Each struct contains a TCB struct and a next pointer. 
	//should have next pointer


struct readyStruct{
	ucontext_t* readyTCB_ptr;
	struct readyStruct* next;
}

readyStruct* nxtReadyNode;
readyStruct* readyList =NULL;

int thread_yield(){
	//1. put current TCB on back of ready list
	//a. initialize a readylist node that will contain a next pointer to next node and the ucurrent_ptr (stored as readyTCB_ptr)
	readyStruct* currTCBNode;
	getcontext(currTCBNode->readyTCB_ptr); //same as: currTCBNode->readyTCB_ptr = ucontext_ptr aka our current TCB
	currTCBNode->next=NULL;
	//b.push that node to end of readylist queue
	push(currTCBNode); 			//how pass in?
	
	//2. ***START OF SWITCH***
	//if there are threads are ready
	readyList.pop(); //get next ready node (could just be you if you are the only thread left)
	swapcontext(getcontext(ucontext_ptr), nxtReadyNode->readyTCB_ptr);
	//**END OF SWITCH***
}

//add TCB onto end of readyList
void push(nodeToAdd){  //check how to pass
	if(readyList==NULL){ //if ready queue is empty
		readyList=nodeToAdd;
	}else{ 
		readyStruct* temp = readyList; 
		while(temp->next!=NULL){ //iterate to end of ready queue and add at end
			temp = temp->next;
		}
		temp->next=nodeToAdd;
	}

}

//get next TCB from front of ready list (already checked readylist not null if here)
void pop(){
	nxtReadyNode = readyList;		//next ready node is the front of list node
	readyList = readyList->next;  //update front of readyList to be next node
}

