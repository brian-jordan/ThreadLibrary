#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
using namespace std;


int maxOrdersOnBoard;
int numOrdersOnBoard;
int prevMadeSandwich;
int numCashiers;
unsigned int deliLock; //main lock
unsigned int makerCV; //maker condition variable
bool* arr;
unsigned int initialNumCashiers;


struct BoardListNode {
	unsigned int cashierNumber; //unsigned int for lock
	int sandwichNumber;
	struct BoardListNode* next;
};

struct CashierInfo {
	FILE* file_ptr;
	unsigned int id;
};

BoardListNode* startBoardList;
BoardListNode* endBoardList;

void print_freelist() {
	BoardListNode* freelist_head = startBoardList;
	while(freelist_head != NULL) {
		fprintf(stderr, "\tFreelist cashier numb:%d, sandwich Number:%d, Next:%p\t \n",
			freelist_head->cashierNumber,
			freelist_head->sandwichNumber,
			freelist_head->next);
		freelist_head = freelist_head->next;
	}
}

int checkSandwhich(BoardListNode* start, unsigned int ID) {
    BoardListNode* temp = start;

	while(temp->sandwichNumber!=1001){
		if(temp->cashierNumber==ID){
			return 1;
		}
		else{
			temp=temp->next;
		}
	}
	return 0;
}

bool canSignalThisCashier(unsigned int ID) {
	return arr[ID];
}

void turnCashierToFalse(unsigned int ID){
	arr[ID] = false;
}
void turnCashierToTrue(unsigned int ID){
	arr[ID] = true;
}


//get sandwhich take it off the board 
void take_off() {
    //BoardListNode* temp = ((struct BoardListNode*)(malloc(sizeof(struct BoardListNode))));
    BoardListNode* temp = startBoardList->next;
    //BoardListNode* removeTempNode = temp;
	//temp->cashierNumber = 0; //************change from NULL
	//temp->sandwichNumber = 0;
	//temp->next = startBoardList;

	// fprintf(stderr, "take_off: entering takeoff method\n");
	// fprintf(stderr, "take_off: this prevMadeSandwich value is %d\n", prevMadeSandwich);
	// fprintf(stderr, "take_off: this is the boardlist before the takeoff\n");
	// print_freelist();
	
	BoardListNode* removeNode=startBoardList->next;
	int minDiff=10000;
	BoardListNode* before= startBoardList;
	BoardListNode* finalBefore;

	while(temp->sandwichNumber!=1001){
		//fprintf(stderr, "take off: entering the while loop\n");
		int diff=abs((temp->sandwichNumber) - prevMadeSandwich);
		//fprintf(stderr, "take off: diff is %d\n",diff);
		
		if(diff<minDiff){
			//fprintf(stderr, "take off: if: diff (%d) is less than minDiff (%d) \n",diff, minDiff);
			minDiff=diff;
			removeNode=temp;
			finalBefore=before;
		
			// fprintf(stderr, "take off: if: finalbefore is \n \tcashier numb:%d, sandwich Number:%d, Next:%p\t \n", 
			// 		finalBefore->cashierNumber,
			// 		finalBefore->sandwichNumber,
			// 		finalBefore->next);
			// fprintf(stderr, "take off: if: removeNode is \n \tcashier numb:%d, sandwich Number:%d, Next:%p\t \n", 
			// 		removeNode->cashierNumber,
			// 		removeNode->sandwichNumber,
			// 		removeNode->next);

		}
		before=before->next;
		temp = temp->next;
	}
	// fprintf(stderr, "take off: exited while loop and these are my node values \n");
	// fprintf(stderr, "take off: if: finalbefore is \n \tcashier numb:%d, sandwich Number:%d, Next:%p\t \n", 
	// 				finalBefore->cashierNumber,
	// 				finalBefore->sandwichNumber,
	// 				finalBefore->next);
	// fprintf(stderr, "take off: if: removeNode is \n \tcashier numb:%d, sandwich Number:%d, Next:%p\t \n", 
	// 				removeNode->cashierNumber,
	// 				removeNode->sandwichNumber,
	// 				removeNode->next);

	finalBefore->next=removeNode->next;

	// fprintf(stderr, "take off: removed the node and so this is my list now\n");
	// print_freelist();

	unsigned int madeCashierID = removeNode->cashierNumber;
	int madeSandwichNumber = removeNode->sandwichNumber;

	prevMadeSandwich = madeSandwichNumber;
	numOrdersOnBoard--;

	// fprintf(stderr, "take_off: after the takeoff the prevMadeSandwich value is %d\n", prevMadeSandwich);
	// fprintf(stderr, "take_off: after the takeoff the numOrdersOnBoard value is %d\n", numOrdersOnBoard);
	// fprintf(stderr, "take_off: after the takeoff the madeCashierID/cashier that will be signalled is %d\n", madeCashierID);
	
	cout << "READY: cashier " << madeCashierID << " sandwich " << madeSandwichNumber << std::endl;
	


	free(removeNode);
	//free(removeTempNode);

	// Print Made Sandwich info here

	thread_signal(deliLock, madeCashierID);

}



int submitOrder(CashierInfo* info){
	//fprintf(stderr, "submitOrder: just entered submit order function\n");
	// fprintf(stderr, "Beginning of submitOrder method \n");
	int fileReadResult;

	int done = fscanf(info->file_ptr, "%d", &fileReadResult);
	// fprintf(stderr, "File was read from \n");
	// fprintf(stderr, "submitOrder: the done value is %d\n", done);
	if(done == EOF){ //if there is no more sandwhiches to read, KILL IT
		//fprintf(stderr, "submitOrder: if: entering first if DONE ==EOF %d\n", EOF);
		numCashiers--; //decrement number of cashiers
		//fprintf(stderr, "submitOrder: if: numCashiers is %d\n", numCashiers);


 		//fprintf(stderr, "submitOrder: if: returning from first if statement to kill thread\n");

		int killThread=1;
		return killThread;

 	}
 	else{ //more sandwhiches to read from this cashier so its NOT done
 		//add sandwhich to board
 		//fprintf(stderr, "submitOrder: else: entering else. cashier has more sandwiches\n");

 		BoardListNode* newOrder = ((struct BoardListNode*)(malloc(sizeof(struct BoardListNode))));
 		newOrder->cashierNumber = info->id;
 		newOrder->sandwichNumber = fileReadResult;
 		// fprintf(stderr, "submitOrder: this is the new order to submite \n");
 		// fprintf(stderr, "submitOrder: newOrder is \n \tcashier numb:%d, sandwich Number:%d, Next:%p\t \n", 
			// 		newOrder->cashierNumber,
			// 		newOrder->sandwichNumber,
			// 		newOrder->next);


 		BoardListNode* temp = startBoardList;
 		//fprintf(stderr, "submitOrder: else: while: entering while\n");
 		while(temp->sandwichNumber!=1001){ //last temp will be 1001 //check this through


 			if(temp->next->sandwichNumber>=fileReadResult){
 				// fprintf(stderr, "submitOrder: else: while: if: the sandwhich should go here in the list\n");
 				
 				// fprintf(stderr, "submitOrder: else: while: if: this is the prev node aka temp\n");
	 			// fprintf(stderr, "submitOrder: temp is\n \tcashier numb:%d, sandwich Number:%d, Next:%p\t \n", 
					// temp->cashierNumber,
					// temp->sandwichNumber,
					// temp->next);

 				// fprintf(stderr, "submitOrder: else: while: if: this is the next node aka temp->next\n");
	 			// fprintf(stderr, "submitOrder: temp->next is\n \tcashier numb:%d, sandwich Number:%d, Next:%p\t \n", 
					// temp->next->cashierNumber,
					// temp->next->sandwichNumber,
					// temp->next->next);

	 			newOrder->next = temp->next;
	 			temp->next = newOrder;
	 			break;
	 		}
	 		temp = temp->next;
	 	}
 		
 	 	// fprintf(stderr, "Sandwich was added to board \n");
 		numOrdersOnBoard++; 
 	// 	fprintf(stderr, "submitOrder: this is the numOrdersOnBoard %d\n", numOrdersOnBoard);
 	// 	fprintf(stderr, "submitOrder: added the node and so this is my list now\n");
		// print_freelist();
 		cout << "POSTED: cashier " << newOrder->cashierNumber << " sandwich " << newOrder->sandwichNumber << std::endl;
 		int killThread=0;
 		return killThread;
 	}
}

// Cashier Method for adding orders
void cashier(void* arg){ //unsigned int for lock??

	// fprintf(stderr, "Cashier method starting \n");

	CashierInfo* myInfo = ((CashierInfo*) arg);
	//fprintf(stderr, "cashier: cashierNumber %d: enter cashier\n", myInfo->id);

	while(1){
		//fprintf(stderr, "cashier: cashierNumber %d: trying to acquire lock\n", myInfo->id);
		thread_lock(deliLock);
		//fprintf(stderr, "cashier: cashierNumber %d: has lock\n", myInfo->id);

		// unsigned int x = myInfo->id;
		// unsigned int y = checkSandwhich(startBoardList, myInfo->id);
		// fprintf(stderr, "%u ", x);
		// fprintf(stderr, "\n");
		// fprintf(stderr, "%u ", y);
		// fprintf(stderr, "\n");
		// fprintf(stderr, "%u ", numOrdersOnBoard);
		// fprintf(stderr, "\n");



		while(numOrdersOnBoard == maxOrdersOnBoard || checkSandwhich(startBoardList, myInfo->id)){
			//unsigned int x = myInfo->id
			//fprintf(stderr, "cashier: cashierNumber %d: while:enter while loop to wait\n", myInfo->id);

			thread_wait(deliLock, myInfo->id);
		}
		// fprintf(stderr, "cashier: out of wait while loop \n");

		// fprintf(stderr, "Cashier Adding Order\n");
		//fprintf(stderr, "cashier: cashierNumber %d: woken up, left wait + calling submitOrder\n", myInfo->id);
		int killThread = submitOrder(myInfo);
		if(killThread == 1){
			//fprintf(stderr, "cashier: cashierNumber %d: killing this thread, signalling maker CV + unlocking\n", myInfo->id );
			// fprintf(stderr, "max orders on board is %d\n", maxOrdersOnBoard);
			turnCashierToFalse(myInfo->id);
			//update largePoss number (since num chashiers aka live threads is diff)
			if(numCashiers<maxOrdersOnBoard){
				//fprintf(stderr, "cashier: if: if: enter if statement. numCashiers (%d) is < maxOrdersOnBoard (%d) \n", numCashiers, maxOrdersOnBoard);
	 			maxOrdersOnBoard = numCashiers;
	 			//fprintf(stderr, "======Signalling Maker \n");
	 			thread_signal(deliLock, makerCV);
	 		}else{
	 			//fprintf(stderr, "cashier: if: else: signalling other cashiers. numCashiersis %d and maxOrdersOnBoard is %d\n", numCashiers, maxOrdersOnBoard);
	 			for(unsigned int i=0;i<initialNumCashiers;i++){
	 				if(arr[i]==true){
	 					//fprintf(stderr, "im signalling cashier %u\n", i);
	 					thread_signal(deliLock, i);
	 				}
	 				
	 			}
	 		}


			 //signal maker so it can pick another cashier’s sandwhich
			//cout << "Cashier thread exiting\n";
			//fprintf(stderr, "cashier: cashierNumber %d: unlocking\n", myInfo->id);
			thread_unlock(deliLock);
			return;

		}
		//fprintf(stderr, "cashier: cashierNumber %d: signalling makerCV2 + unlocking\n", myInfo->id );

		//fprintf(stderr, "======Signalling Maker \n");
		thread_signal(deliLock, makerCV);
		
		thread_unlock(deliLock);
	}
}


// Maker Method for making orders
void maker(void* arg) {
	// fprintf(stderr, "maker: enter maker\n");
	// fprintf(stderr, "Beginning of maker method \n");
	char **argv = (char **)(arg);
	unsigned int counter = 0;
	for (int i = 2; i < numCashiers + 2; i++){
		//fprintf(stderr, "%s \n",((char*) arg)+i );
	//	FILE* specificCashierFile_ptr = fopen(((char*) arg)+i, "r");											// File reader corresponding to cashier
		FILE* specificCashierFile_ptr = fopen(argv[i], "r");
		unsigned int cashierID = counter;
		counter=counter+1;

		// Stores file reader and cashier number into a struct to pass as one argument
		CashierInfo* specificCashierInfo_ptr = ((struct CashierInfo*)(malloc(sizeof(struct CashierInfo))));
		specificCashierInfo_ptr->file_ptr = specificCashierFile_ptr;
		specificCashierInfo_ptr->id = cashierID;

		//fprintf(stderr, "Calling thread_create for single cashier \n");

		// Creates cashier thread
		if (thread_create((thread_startfunc_t) cashier, ((void*) specificCashierInfo_ptr))){
			cout << "thread_create failed\n";
			exit(1);
		}
		turnCashierToTrue(cashierID);
	}

	while(1){
		//fprintf(stderr, "maker: enter while loop\n");
		thread_lock(deliLock);
    	//while the board isn't full or the number of orders 
    	//isn't at it's max then we wait for the board to be full 
   		//we give up the lock and wait for the makerCV to change
    	//so that we are woken up! 
    	while(numOrdersOnBoard!=maxOrdersOnBoard){
    		//fprintf(stderr, "maker: start wait\n");
    		 //fprintf(stderr, "maker: while: numOrdersOnBoard is %d and maxOrdersOnBoard is %d\n", numOrdersOnBoard, maxOrdersOnBoard);
        	thread_wait(deliLock,makerCV);
    	}
    	///fprintf(stderr, "maker: out of wait\n");
    	//fprintf(stderr, "Maker Removing order\n");
    	//we fall out of the while loop with the lock. so we 
    	//have the lock and now we are going to take off a sandwhich and make it 
    	if(numCashiers==0){
    		//fprintf(stderr, "maker: numCashiers is 0 so gonna kill the maker %d\n",numCashiers );
    		free(startBoardList);
    		free(endBoardList);
    		thread_unlock(deliLock);
    		return;
    		//TO DO: KILL IT ALL, END OF THE PROGRAM
    	}else{
    		//fprintf(stderr, "maker: numCashiers is not 0 so calling take_off %d\n",numCashiers );
			take_off();
    	}

    	//job is done, unlock the thread
    	//fprintf(stderr, "maker: unlocking thread\n");
    	thread_unlock(deliLock);

		}

}

int main(int argc, char* argv[]){
	//******BRIAN TODO: ~~~~~INITILIZE HERE AND CREATE THREADS~~~~
			//*********
				///*********

	// Initializes global variables
	//fprintf(stderr, "main \n");
	//fprintf(stderr, "Initializing Global Variables \n");
	maxOrdersOnBoard = atoi(argv[1]);
	numOrdersOnBoard = 0;
	numCashiers = argc - 2;
	prevMadeSandwich = -1;
	deliLock = 0;
	makerCV = (unsigned int)argc;					// Number we know is greater than number of cashiers
	arr = (bool*) malloc(numCashiers * sizeof(bool));
	initialNumCashiers = numCashiers;
	// fprintf(stderr, "Initializing Boarld List \n");

	// Initializes first and last nodes of the board linked list
	startBoardList = ((struct BoardListNode*)(malloc(sizeof(struct BoardListNode))));
	startBoardList->cashierNumber = -1;
	startBoardList->sandwichNumber = -1;
	endBoardList = ((struct BoardListNode*)(malloc(sizeof(struct BoardListNode))));
	startBoardList->next = endBoardList;
	endBoardList->cashierNumber = 10001;
	endBoardList->sandwichNumber = 1001;
	endBoardList->next = NULL;



	// fprintf(stderr, "Calling thread_libinit with maker method \n");

	// Initiates maker thread with no argument
	if (thread_libinit((thread_startfunc_t) maker, ((void*) argv))){
		cout << "thread_libinit failed \n";
		exit(1);
	}
	//fprintf(stderr, "last main \n");

}
