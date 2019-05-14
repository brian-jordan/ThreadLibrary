#include <stdlib.h>
#include <iostream>
#include "thread.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <ucontext.h> //need this for later when using u context structs 
#include "interrupt.h"
#include <map>
#include <list>

using namespace std;
bool haveRunLibinit = false;

class ThreadBlock {
public:
        // Public Variables that can be referenced
        ucontext_t* TCB_ptr;
        char* stack_ptr;
        unsigned int id;
        // Constructor
        ThreadBlock(){};
        // Destructor
        ~ThreadBlock() {
                // Deletes stack ptr of ucontext
                delete[](stack_ptr);
        }

};

class Lock {
public:
        // Public Variables that can be referenced
        bool inUse;
        unsigned int threadID;
        // Constructor
        Lock(bool used, unsigned int thread) : inUse(used), threadID(thread) {};
};

static map<unsigned int, Lock*> lockTable = map<unsigned int, Lock*>(); //unsigned int is name of lock
static map<unsigned int, list<ThreadBlock*> > allLockQueues = map<unsigned int, list<ThreadBlock*> >(); //map where key is lock val and the value is the queue 
static map<unsigned int, list<ThreadBlock*> > allCondQueues = map<unsigned int, list<ThreadBlock*> >(); //map where key is cond val and the value is the queue 


ThreadBlock* runningBlock = NULL;
static list<ThreadBlock*> readyWaitingQueue = list<ThreadBlock*>();
static list<ThreadBlock*> deleteThreadQueue = list<ThreadBlock*>();

//readyWaitingQueue. not ->
// ThreadBlock* deleteThreadNode = NULL;
//add lock
//swapcontext reutn -1 reutn -1
//insert can just be [currLock] == null if that lock doesnt exist



void deleteList(list<ThreadBlock*> deletingList){
   // interrupt_disable();
    while(! deletingList.empty()){
        fprintf(stderr, "in delete list\n");
        ThreadBlock* deletingBlock = deletingList.front();

        deletingList.pop_front();
        delete(deletingBlock);
    }
        // if (! deletingList.empty()){
        //         list<ThreadBlock*>::iterator iter;
        //         for (iter = deletingList.begin(); iter != deletingList.end(); ++iter){
        //                 delete(iter);
        //         }
        //         deletingList.clear();
        // }

   // interrupt_enable();
}

void exitSuccessfully(){
   // interrupt_disable();
    deleteList(deleteThreadQueue);

    // delete(deleteThreadQueue);
    // while(!lockTable.empty()){
    //     Lock* deleteLock = lockTable.begin()->second;
    //     delete(deleteLock);
    //     lockTable.erase(lockTable.begin()->first);
    // }
    if (! allLockQueues.empty()){
        map<unsigned int, list<ThreadBlock*> >::iterator iter;
        for(iter = allLockQueues.begin(); iter != allLockQueues.end(); ++iter){
                deleteList(iter->second);
        }
        allLockQueues.clear();
    }

    // while(!allLockQueues.empty()){
    //     list<ThreadBlock*> deleteLockList = allLockQueues.begin()->second;
    //     deleteList(deleteLockList);
    //     // delete(deleteLockList);
    //     allLockQueues.erase(allLockQueues.begin()->first);
    // }
    if (! allCondQueues.empty()){
        map<unsigned int, list<ThreadBlock*> >::iterator iter;
        for(iter = allCondQueues.begin(); iter != allCondQueues.end(); ++iter){
                deleteList(iter->second);
        }
        allCondQueues.clear();
    }
    //delete(allLockQueues);
    // while(!allCondQueues.empty()){
    //     list<ThreadBlock*> deleteCondList = allCondQueues.begin()->second;
    //     deleteList(deleteCondList);
    //     // delete(deleteCondList);
    //     allCondQueues.erase(allCondQueues.begin()->first);
    // }
  //  interrupt_enable();
    //delete(allCondQueues);
    interrupt_enable();
    cout << "Thread library exiting.\n";
    exit(0);
}

void checkReadyWaitingQueue(){
    //interrupt_disable();
        if (readyWaitingQueue.empty()){
                exitSuccessfully();
        }
}



void trampoline(thread_startfunc_t func, void* arg){
    
        // enable interrups

        // if user function messes up return to library (try, catch)
       // interrupt_enable();
        func(arg);
        interrupt_disable();
        // Could there be a context switch that would throw this off
        // Going to need to make this part to work without interruptions
        // push(runningNode, deleteThreadQueue);
        deleteList(deleteThreadQueue);

        //interrupt_disable(); // i dont think this is right but u have to interrupt when you swap context 

        ThreadBlock* threadToDelete = runningBlock;
        deleteThreadQueue.push_back(threadToDelete);
        checkReadyWaitingQueue();
        runningBlock = readyWaitingQueue.front();
        readyWaitingQueue.pop_front();
        interrupt_enable();
        swapcontext(threadToDelete->TCB_ptr, runningBlock->TCB_ptr);
        interrupt_disable();

        //interrupt_enable(); //still not sure about this becuase you swap context and now they should be enabled.. but they might now be 

}

ThreadBlock* startThread(thread_startfunc_t func, void* arg){
        fprintf(stderr, "threadstart \n");
        //interrupt_disable();
        ThreadBlock* newThreadBlock = new ThreadBlock;
        newThreadBlock->TCB_ptr = new ucontext_t;
        getcontext(newThreadBlock->TCB_ptr);
        char *stack=new char[STACK_SIZE];
        newThreadBlock->stack_ptr = stack;
        newThreadBlock->TCB_ptr->uc_stack.ss_sp = stack;
        newThreadBlock->TCB_ptr->uc_stack.ss_size = STACK_SIZE;
        newThreadBlock->TCB_ptr->uc_stack.ss_flags = 0;
        newThreadBlock->TCB_ptr->uc_link = NULL;

        makecontext(newThreadBlock->TCB_ptr, (void (*)()) trampoline, 2, func, (arg));    // Question: Do I need the pointer to a pointer here?
        //interrupt_enable();
        fprintf(stderr, "ending thread start \n");
        return newThreadBlock;

}

int thread_libinit(thread_startfunc_t func, void *arg){
    interrupt_disable();
        if (haveRunLibinit == true){
                return -1;
        }
        fprintf(stderr, "in lib init \n");

        ucontext_t* firstContext = new ucontext_t;
        getcontext(firstContext);

        //interrupt_disable(); // idk if they should be here or if they should be in startThread;
       // interrupt_enable();
        ThreadBlock* newThreadBlock = startThread(func, arg);
        //interrupt_disable();
        //add thread to ready queue 

        runningBlock = newThreadBlock;
        haveRunLibinit = true;

        // Think this is what the TA said to do
        interrupt_enable();
        if(swapcontext(firstContext, runningBlock->TCB_ptr)==-1){
            return -1;
        }
        interrupt_disable();

        fprintf(stderr, "SHOULD NEVER GET HERE \n");
        interrupt_enable();
        return -1;
}       


int thread_create(thread_startfunc_t func, void *arg){
    interrupt_disable();
        fprintf(stderr, "in thread create \n");
         if (haveRunLibinit == false){
                return -1;
        }

   // interrupt_enable();
    ThreadBlock* newThreadBlock = startThread(func, arg);
    //interrupt_disable();

    //add thread to ready queue 
    //checkReadyWaitingQueue();
    readyWaitingQueue.push_back(newThreadBlock);
    // push(newNode, readyWaitingQueue);
    interrupt_enable();
    fprintf(stderr, "at the end \n");
    return 0;
}



int thread_yield(void){
    interrupt_disable();
        if (haveRunLibinit == false){
                return -1;
        }
        //1. put current TCB on back of ready list
        //push that node to end of readylist queue
        ThreadBlock* currentRunning=runningBlock;
        readyWaitingQueue.push_back(runningBlock);

        //2. ***START OF SWITCH***
        //if there are threads are ready
        checkReadyWaitingQueue();
        ThreadBlock* nextThreadBlock = readyWaitingQueue.front(); //get next ready node (could just be you if you are the only thread left)
        readyWaitingQueue.pop_front(); //remove it
        runningBlock=nextThreadBlock;
        interrupt_enable();
        if(swapcontext(currentRunning->TCB_ptr, runningBlock->TCB_ptr)==-1){
            return -1;
        }
        interrupt_disable();
        //**END OF SWITCH***
      interrupt_enable();
        // fprintf(stderr, "its at bottom of yield\n");
        return 0;
}

int thread_wait(unsigned int currLock, unsigned int currCond){
   interrupt_disable();
    if (haveRunLibinit == true){
        return -1;
    }

    unsigned int ID = runningBlock->id; //get the current thread's ID
    
    ////***UNLOCKING******
    ///BEFORE: error checking
    //error: a thread trying to unlock a lock that has never been locked before (never used)
    if(lockTable.find(currLock)==lockTable.end()){
        return -1;
    }
    //error: check if it has the lock. if it doesn't then its an error   
    Lock* checkLock = lockTable[currLock]; //get the lock's info from the table
    if(!((checkLock->threadID == ID) && (checkLock->inUse))){ //check that the locks threadID (aka thread who is using it) is this thread + that the lock is marked as inUse
        //NOT GOOD!!! ERRRORRRR!!!!
        return -1;
    }

    //1->release the lock
    checkLock->inUse =false; //is this how i unlock it?

    //2. get the node that is waiting on this lock q and move to ready queue
    if(!(allLockQueues.find(currLock)  == allLockQueues.end() )){ //if the lock queue for this lock exists in map
        if(!(allLockQueues[currLock].empty())){ //if the lock queue isnt null aka there's still guys on here
            //move thread from this lock waiting queue to ready queue
            ThreadBlock* nextThreadBlock = allLockQueues[currLock].front(); //get next node from lock queue
            allLockQueues[currLock].pop_front();
            
            readyWaitingQueue.push_back(nextThreadBlock); //put this next node onto ready waiting list
        }
    }
    
    //*****MAIN: PUSH NODE ONTO CV QUEUE
    //ii. check if this queue has been created
    if(allCondQueues.find(currCond) == allCondQueues.end()){
        allCondQueues[currCond] = list<ThreadBlock*>(); //iii. doesn't exist
    }
        allCondQueues[currCond].push_back(runningBlock); //push thread(runningNode) onto the conditino q for currCond

    //3-> **START OF SWITCH
    checkReadyWaitingQueue(); //if there are threads are ready
    ThreadBlock* nextThreadBlock = readyWaitingQueue.front();; //get next ready node (could just be you if you are the only thread left
    readyWaitingQueue.pop_front(); //remove it
    interrupt_enable();
    if(swapcontext(runningBlock->TCB_ptr, nextThreadBlock->TCB_ptr)==-1){
        return -1;
    }
    interrupt_disable();



    ///**GETTTING THE LOCK***//
    if(lockTable.find(currLock) == lockTable.end() ) {
        //lock is not in lock table so create it as lock* struct and add it to the lockTable
        Lock* addLock;
        addLock->inUse=false;
        addLock->threadID = 0;
        lockTable[currLock]= addLock; //will return null
    }

    //BEFORE: Get lock info from table
    checkLock = lockTable[currLock]; //get the lock's info from the table
    

     //BEFFORE: ERROR CHECK
    //a. check if thread already has this lock
    if((checkLock->inUse) && checkLock->threadID == runningBlock->id){
        return -1;
    }


    //check if lock is taken aka busy
    if(!(checkLock->inUse)){ // lock is free
        checkLock->inUse = true;
        checkLock->threadID = runningBlock->id;
        lockTable[currLock] = checkLock; //copy lock back into table
    }
    else{ //lock is taken so put thread on lock queue
        
        //1. check if this queue has been created
        if (allLockQueues.find(currLock) == allLockQueues.end() ) {    
            //2. doesn't exist so make this TCB the start of the queue list (no need to push)
           allLockQueues[currLock] =list<ThreadBlock*>();
        }
        //add thread(runningNode) to lock queue for this lock(currLock)
        allLockQueues[currLock].push_back(runningBlock);

        //3. switch to next ready thread
        checkReadyWaitingQueue();
        ThreadBlock* nextThreadBlock =readyWaitingQueue.front(); //get next ready node (could just be you if you are the only thread left
        readyWaitingQueue.pop_front(); //remove it from ready q
        interrupt_enable();
        if(swapcontext(runningBlock->TCB_ptr, nextThreadBlock->TCB_ptr)){
            return -1;
        }
        interrupt_disable();

    }


   interrupt_enable();
    return 0;
}

int thread_lock(unsigned int currLock){
    interrupt_disable();
	//map<unsigned int, list<Lock*>*>* lockTable; //unsigned int is name of lock
    if (haveRunLibinit == true){
        return -1;
    }
    //disable interrupts

    //BEFORE: CHECK IF LOCK IS IN LOCK TABLE ALREADY
    //add this lock to lock table if not already a part
    if(lockTable.find(currLock) == lockTable.end() ) {
        //lock is not in lock table so create it as lock* struct and add it to the lockTable
        Lock* addLock;
        addLock->inUse=false;
        addLock->threadID = 0;
        lockTable[currLock]= addLock; //will return null
    }

    //BEFORE: Get lock info from table
    Lock* checkLock = lockTable[currLock]; //get the lock's info from the table
    

     //BEFFORE: ERROR CHECK
    //a. check if thread already has this lock
    if((checkLock->inUse) && checkLock->threadID == runningBlock->id){
        return -1;
    }


    //check if lock is taken aka busy
    if(!(checkLock->inUse)){ // lock is free
        checkLock->inUse = true;
        checkLock->threadID = runningBlock->id;
        lockTable[currLock] = checkLock; //copy lock back into table
    }
    else{ //lock is taken so put thread on lock queue
        
        //1. check if this queue has been created
        if (allLockQueues.find(currLock) == allLockQueues.end() ) {    
            //2. doesn't exist so make this TCB the start of the queue list (no need to push)
           allLockQueues[currLock] =list<ThreadBlock*>();
        }
        //add thread(runningNode) to lock queue for this lock(currLock)
        allLockQueues[currLock].push_back(runningBlock);

        //3. switch to next ready thread
        checkReadyWaitingQueue();
        ThreadBlock* nextThreadBlock =readyWaitingQueue.front(); //get next ready node (could just be you if you are the only thread left
        readyWaitingQueue.pop_front(); //remove it from ready q
        interrupt_enable();
        if(swapcontext(runningBlock->TCB_ptr, nextThreadBlock->TCB_ptr)==-1){
            return -1;
        }
        interrupt_disable();

    }
    interrupt_enable();
    return 0;
    //enable interrupts
}

int thread_unlock(unsigned int currLock){
    interrupt_disable();
    if (haveRunLibinit == true){
        return -1;
    }
    //disable interrupts
    //Error
    //error: a thread trying to unlock a lock that has never been locked before (never used)
    if(lockTable.find(currLock)==lockTable.end()){
        return -1;
    }

    //get lock info from table + get lock queue
    Lock* checkLock = lockTable[currLock]; //get the lock's info from the table

    //error: a thread trying to unlock a lock that is already unlocked
    if(checkLock->inUse ==false){
        return -1;
    }
    //error: a thread trying to unlock lock that it doesnt have  (aka a lock that another thread is locked to)
    if(!(checkLock->threadID == runningBlock->id)){
        return -1;
    }
    
    //free lock
    checkLock->inUse=false;

    //get the node that is waiting on this lock and move to ready queue
    if(!(allLockQueues.find(currLock)  == allLockQueues.end() )){ //if the lock queue for this lock exists in map
        if(!(allLockQueues[currLock].empty())){ //if the lock queue isnt null aka there's still guys on here
            //move thread from front of this lock waiting queue to end of ready queue
            ThreadBlock* nextThreadBlock = allLockQueues[currLock].front(); //get next node from front of lock queue
                allLockQueues[currLock].pop_front(); //remove it from lock q

            readyWaitingQueue.push_back(nextThreadBlock);//put this next node onto ready waiting list

            //change lock to busy
            checkLock->inUse=true;
            checkLock->threadID=nextThreadBlock->id;
        } 
    }

    lockTable[currLock] = checkLock; //copy lock back into lock table with new values
    //enable interrupts
  interrupt_enable();
    return 0;
}

int thread_signal(unsigned int currLock, unsigned int currCond){
    interrupt_disable();
    if (haveRunLibinit == true){
        return -1;
    }
    //errors
    // //error: lock doesnt exist?
    // if(lockTable->find(currLock) == lockTable->end()){
    //     return -1;
    // }
    // //error: cond doesn't exist?
    ////error: theres nothing waiting on this condition
    if(!(allCondQueues.find(currCond) == allCondQueues.end())){ //that condition queue exists
        if(!(allCondQueues[currCond].empty())){ //exists and its not null (aka there actually IS a node waiting on this cond)
            if(!(allLockQueues.find(currLock) == allLockQueues.end())){
                //***move head of contexts from waiting on cond to waiting on lock

                //get node from front of this condition queue
                ThreadBlock* moveThreadBlock = allCondQueues[currCond].front(); //get it
                                allCondQueues[currCond].pop_front(); //remove it

                //put node on end of lock queue corresponding to lock
                readyWaitingQueue.push_back(moveThreadBlock);
            }
        }
    }
    interrupt_enable();
    return 0;

}

int thread_broadcast(unsigned int currLock, unsigned int currCond){
    interrupt_disable();
    if (haveRunLibinit == true){
        return -1;
    }
    //move all contexts from waiting on cond to waiting on lock
    if(!(allCondQueues.find(currCond) == allCondQueues.end())){ //that condition queue exists
        if(!(allLockQueues.find(currLock) == allLockQueues.end())){ //lock queue exists
            while(!(allCondQueues[currCond].empty())){ //exists and its not null (aka there actually IS a node waiting on this cond)
                //***move head of contexts from waiting on cond to waiting on lock

                //get node from front of this condition queue
                ThreadBlock* moveThreadBlock = allCondQueues[currCond].front(); //get it
                allCondQueues[currCond].pop_front(); //remove it

                //put node on end of lock queue corresponding to lock
                readyWaitingQueue.push_back(moveThreadBlock);
            }
        }
    }
   interrupt_enable();
    return 0;   
}


