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

ucontext_t* firstContext;
ThreadBlock* runningBlock = NULL;
unsigned int numberOfThreads =0;
static list<ThreadBlock*> readyWaitingQueue = list<ThreadBlock*>();
static list<ThreadBlock*> deleteThreadQueue = list<ThreadBlock*>();
int functionIn = 0;
bool isDisabled = false;
//readyWaitingQueue. not ->
// ThreadBlock* deleteThreadNode = NULL;
//add lock
//swapcontext reutn -1 reutn -1
//insert can just be [currLock] == null if that lock doesnt exist

void printQueue(map<unsigned int, list<ThreadBlock*> > queue){
    fprintf(stderr, "Printing queue!!!!!!! \n");
        map<unsigned int, list<ThreadBlock*> >::iterator iter1;
        list<ThreadBlock*>::iterator iter2;
        for(iter1 = queue.begin(); iter1 != queue.end(); ++iter1){
            list<ThreadBlock*> lister = iter1->second;
            fprintf(stderr, "key: %u\n",iter1->first);
            for (iter2 = lister.begin(); iter2 != lister.end(); ++iter2){
                fprintf(stderr, "value: %u\n", (*iter2)->id);

            }
        }


}

void printReadyQ(){
        fprintf(stderr, "Printing Ready List!!!!!!! \n");
        list<ThreadBlock*>::iterator iter;
        for (iter = readyWaitingQueue.begin(); iter != readyWaitingQueue.end(); ++iter){
            fprintf(stderr, "%u\n", (*iter)->id);
        }
}

void enableInterrupts(){
    if(isDisabled){
        isDisabled=false;
        interrupt_enable();
    }
}

void disableInterrupts(){
    if(!isDisabled){
        isDisabled=true;
        interrupt_disable();
    }
}
void checkReadyWaitingQueue(){
    //fprintf(stderr, "check ready start \n");
    //interrupt_disable();
        if (readyWaitingQueue.empty()){
                // exitSuccessfully();
                swapcontext(runningBlock->TCB_ptr, firstContext);
        }
}



int popAndSwap(){
       // fprintf(stderr, "enter pop+swap\n");
        ThreadBlock* threadToDelete = runningBlock;
        checkReadyWaitingQueue();
        runningBlock = readyWaitingQueue.front();
        readyWaitingQueue.pop_front();

        //printReadyQ();

        //interrupt_enable();
        if (swapcontext(threadToDelete->TCB_ptr, runningBlock->TCB_ptr) == -1){
            return(-1);
        }
        return(0);
}


list<ThreadBlock*> deleteList(list<ThreadBlock*> deletingList){
   // interrupt_disable();
    while(! deletingList.empty()){
        //fprintf(stderr, "function calling deleteList %d\n", functionIn );
        // fprintf(stderr, "in delete list\n");
        // fprintf(stderr, "delete list size before delete%lu\n", deletingList.size());
        ThreadBlock* deletingBlock = deletingList.front();
        //fprintf(stderr, "deleting block id%u\n", deletingBlock->id);
        deletingList.pop_front();
        if (deletingList == deleteThreadQueue){
            numberOfThreads--;
        }

        delete(deletingBlock);
    }
    return deletingList;
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
   // fprintf(stderr, "exit success start \n");
    deleteThreadQueue = deleteList(deleteThreadQueue);

    while(!allLockQueues.empty()){
        list<ThreadBlock*> deleteLockList = allLockQueues.begin()->second;
        deleteLockList = deleteList(deleteLockList);
        // delete(deleteLockList);
        allLockQueues.erase(allLockQueues.begin()->first);
    }


    while(!allCondQueues.empty()){
        list<ThreadBlock*> deleteCondList = allCondQueues.begin()->second;
        deleteCondList = deleteList(deleteCondList);
        // delete(deleteCondList);
        allCondQueues.erase(allCondQueues.begin()->first);
    }

    enableInterrupts();
    cout << "Thread library exiting.\n";
    exit(0);
}


void trampoline(thread_startfunc_t func, void* arg){
        //fprintf(stderr, "trampoline start \n");
        // enable interrups

        // if user function messes up return to library (try, catch)
        enableInterrupts();
        func(arg);
        disableInterrupts();
        // Could there be a context switch that would throw this off
        // Going to need to make this part to work without interruptions
        // push(runningNode, deleteThreadQueue);
        deleteThreadQueue = deleteList(deleteThreadQueue);

        //interrupt_disable(); // i dont think this is right but u have to interrupt when you swap context 
        // fprintf(stderr, "threadblock info adding to deleteThreadQueue (aka runningBlock):\n");
        // fprintf(stderr, "%u\n", runningBlock->id);
        // fprintf(stderr, "%p\n", runningBlock->TCB_ptr);
        deleteThreadQueue.push_back(runningBlock);
        ThreadBlock* threadToDelete = runningBlock;
        checkReadyWaitingQueue();
        runningBlock = readyWaitingQueue.front();
        readyWaitingQueue.pop_front();

        //interrupt_enable();
        swapcontext(threadToDelete->TCB_ptr, runningBlock->TCB_ptr);

        //interrupt_disable();

        //interrupt_enable(); //still not sure about this becuase you swap context and now they should be enabled.. but they might now be 

}

ThreadBlock* startThread(thread_startfunc_t func, void* arg){
    try{
        //fprintf(stderr, "threadstart \n");
        numberOfThreads++;
        //interrupt_disable();
        ThreadBlock* newThreadBlock = new ThreadBlock;
        newThreadBlock->TCB_ptr = new ucontext_t;
        newThreadBlock->id = numberOfThreads;
        getcontext(newThreadBlock->TCB_ptr);
        char *stack=new char[STACK_SIZE];
        newThreadBlock->stack_ptr = stack;
        newThreadBlock->TCB_ptr->uc_stack.ss_sp = stack;
        newThreadBlock->TCB_ptr->uc_stack.ss_size = STACK_SIZE;
        newThreadBlock->TCB_ptr->uc_stack.ss_flags = 0;
        newThreadBlock->TCB_ptr->uc_link = NULL;

        makecontext(newThreadBlock->TCB_ptr, (void (*)()) trampoline, 2, func, (arg));    // Question: Do I need the pointer to a pointer here?
        return newThreadBlock;
    }
    catch(bad_alloc&){
        enableInterrupts();
        exit(-1);
    }
}

int thread_libinit(thread_startfunc_t func, void *arg){
    try{
        disableInterrupts();
       // fprintf(stderr, "libinit start \n");
            if (haveRunLibinit == true){
                    enableInterrupts();
                    return -1;
            }
            //fprintf(stderr, "in lib init \n");

            firstContext = new ucontext_t;
            getcontext(firstContext);
            numberOfThreads = 1;
            //interrupt_disable(); // idk if they should be here or if they should be in startThread;
           // interrupt_enable();
            ThreadBlock* newThreadBlock = startThread(func, arg);

            //printReadyQ();

            //interrupt_disable();
            //add thread to ready queue 

            runningBlock = newThreadBlock;
            haveRunLibinit = true;

            // Think this is what the TA said to do
            if(swapcontext(firstContext, runningBlock->TCB_ptr)==-1){
                enableInterrupts();
                return -1;
            }
            exitSuccessfully();

           // fprintf(stderr, "SHOULD NEVER GET HERE \n");
            enableInterrupts();
    }catch(bad_alloc&){
        enableInterrupts();
        return -1;
    }
    return -1;
}       


int thread_create(thread_startfunc_t func, void *arg){
    disableInterrupts();
        //fprintf(stderr, "in thread create \n");
         if (haveRunLibinit == false){
                enableInterrupts();
                return -1;
        }

   // interrupt_enable();
    ThreadBlock* newThreadBlock = startThread(func, arg);
    //interrupt_disable();

    //add thread to ready queue 
    //checkReadyWaitingQueue();
    readyWaitingQueue.push_back(newThreadBlock);

    ///printReadyQ();

    // while(! readyWaitingQueue.empty()){
    //     //fprintf(stderr, "function calling deleteList %d\n", functionIn );
    //     fprintf(stderr, "in delete list\n");
    //     fprintf(stderr, "delete list size before delete%lu\n", deletingList.size());
    //     ThreadBlock* deletingBlock = deletingList.front();
    //     fprintf(stderr, "deleting block id%u\n", deletingBlock->id);
    //     deletingList.pop_front();
    //     if (deletingList == deleteThreadQueue){
    //         numberOfThreads--;
    //     }

    //     delete(deletingBlock);
    // }
    // push(newNode, readyWaitingQueue);
    enableInterrupts();
    //fprintf(stderr, "at the end \n");
    return 0;
}



int thread_yield(void){
    disableInterrupts();
    //fprintf(stderr, "yield start \n");
        if (haveRunLibinit == false){
            enableInterrupts();
                return -1;
        }
        //1. put current TCB on back of ready list
        //push that node to end of readylist queue
        readyWaitingQueue.push_back(runningBlock);

        //2. ***START OF SWITCH***
        //if there are threads are ready
        if(popAndSwap()==-1){
            enableInterrupts();
            return -1;
        }
        //**END OF SWITCH***
        enableInterrupts();
        // fprintf(stderr, "its at bottom of yield\n");
        return 0;
}

int thread_wait(unsigned int currLock, unsigned int currCond){
    try{
        disableInterrupts();
       // fprintf(stderr, "wait start \n");
         //   fprintf(stderr, "lock: %u\n", currLock);
        //fprintf(stderr, "cond: %u\n", currCond);
        if (haveRunLibinit == false){
            enableInterrupts();
            return -1;
        }

        unsigned int ID = runningBlock->id; //get the current thread's ID
        
        ////***UNLOCKING******
        ///BEFORE: error checking
        //error: a thread trying to unlock a lock that has never been locked before (never used)
        if(lockTable.find(currLock)==lockTable.end()){
            enableInterrupts();
            return -1;
        }
        //error: check if it has the lock. if it doesn't then its an error   
        Lock* checkLock = lockTable[currLock]; //get the lock's info from the table
          //error: a thread trying to unlock a lock that is already unlocked
        if(checkLock->inUse ==false){
            enableInterrupts();
            return -1;
        }
        //error: a thread trying to unlock lock that it doesnt have  (aka a lock that another thread is locked to)
        if(!(checkLock->threadID == runningBlock->id)){
            enableInterrupts();
            return -1;
        }
    
        //1->release the lock
        checkLock->inUse =false; //is this how i unlock it?

        //2. get the node that is waiting on this lock q and move to ready queue
        if(!(allLockQueues.find(currLock)  == allLockQueues.end() )){ //if the lock queue for this lock exists in map
            if(!(allLockQueues[currLock].empty())){ //if the lock queue isnt null aka there's still guys on here
                //move thread from this lock waiting queue to ready queue
                //printQueue(allLockQueues);
                ThreadBlock* nextThreadBlock = allLockQueues[currLock].front(); //get next node from lock queue
                allLockQueues[currLock].pop_front();
                
                readyWaitingQueue.push_back(nextThreadBlock); //put this next node onto ready waiting list
               // printReadyQ();
            }
        }
        
        //*****MAIN: PUSH NODE ONTO CV QUEUE
        //ii. check if this queue has been created
        if(allCondQueues.find(currCond) == allCondQueues.end()){
            allCondQueues[currCond] = list<ThreadBlock*>(); //iii. doesn't exist
        }
        allCondQueues[currCond].push_back(runningBlock); //push thread(runningNode) onto the conditino q for currCond
        //printQueue(allCondQueues);
        if(popAndSwap()==-1){
            enableInterrupts();
            return -1;
        }

        ///**GETTTING THE LOCK***//
        if(lockTable.find(currLock) == lockTable.end() ) {
            //lock is not in lock table so create it as lock* struct and add it to the lockTable
            Lock* addLock = new Lock(false, 0);
            lockTable[currLock]= addLock; //will return null
        }

        //BEFORE: Get lock info from table
        checkLock = lockTable[currLock]; //get the lock's info from the table
        

         //BEFFORE: ERROR CHECK
        //a. check if thread already has this lock
        if((checkLock->inUse) && checkLock->threadID == runningBlock->id){
            enableInterrupts();
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
           // printQueue(allLockQueues);

            if(popAndSwap()==-1){
                enableInterrupts();
                return -1;
            }

        }

        enableInterrupts();
    }catch(bad_alloc&){
        enableInterrupts();
        return -1;
    }
    return 0;

}

int thread_lock(unsigned int currLock){
    try{
        disableInterrupts();
       // fprintf(stderr, "lock start \n");
          //  fprintf(stderr, "lock: %u\n", currLock);
       
    	//map<unsigned int, list<Lock*>*>* lockTable; //unsigned int is name of lock
        if (haveRunLibinit == false){
            enableInterrupts();
            return -1;
        }
        //disable interrupts

        //BEFORE: CHECK IF LOCK IS IN LOCK TABLE ALREADY
        //add this lock to lock table if not already a part
        if(lockTable.find(currLock) == lockTable.end() ) {
            //printf("hey, I'm here\n");
            //lock is not in lock table so create it as lock* struct and add it to the lockTable
            Lock* addLock = new Lock(false, 0);
            lockTable[currLock]= addLock; //will return null
            //printf("hey, I'm here 4\n");
        }
        //fprintf(stderr, "further inlock\n");
        //BEFORE: Get lock info from table
        Lock* checkLock = lockTable[currLock]; //get the lock's info from the table
        

         //BEFFORE: ERROR CHECK
        //a. check if thread already has this lock
        if((checkLock->inUse) && checkLock->threadID == runningBlock->id){
            enableInterrupts();
            return -1;
        }


        //check if lock is taken aka busy
        if(!(checkLock->inUse)){ // lock is free
            //fprintf(stderr, "lock free. in if statement\n");
            checkLock->inUse = true;
            checkLock->threadID = runningBlock->id;
            lockTable[currLock] = checkLock; //copy lock back into table
            //printQueue(allLockQueues);
        }
        else{ //lock is taken so put thread on lock queue
           // fprintf(stderr, "lock taken. in else statement\n");
            
            //1. check if this queue has been created
            if (allLockQueues.find(currLock) == allLockQueues.end() ) {    
                //2. doesn't exist so make this TCB the start of the queue list (no need to push)
               allLockQueues[currLock] = list<ThreadBlock*>();
            }
            //add thread(runningNode) to lock queue for this lock(currLock)
            allLockQueues[currLock].push_back(runningBlock);

            //printQueue(allLockQueues);

            if(popAndSwap()==-1){
                enableInterrupts();
                return -1;
            }

        }
        enableInterrupts();
    }catch(bad_alloc&){
        enableInterrupts();
        return -1;
    }
    return 0;
    //enable interrupts
}

int thread_unlock(unsigned int currLock){
    disableInterrupts();
   // fprintf(stderr, "unlock start \n");
        //fprintf(stderr, "lock: %u\n", currLock);

    if (haveRunLibinit == false){
        enableInterrupts();
        return -1;
    }
    //disable interrupts
    //Error
    //error: a thread trying to unlock a lock that has never been locked before (never used)
    if(lockTable.find(currLock)==lockTable.end()){
        enableInterrupts();
        return -1;
    }

    //get lock info from table + get lock queue
    Lock* checkLock = lockTable[currLock]; //get the lock's info from the table

    //error: a thread trying to unlock a lock that is already unlocked
    if(checkLock->inUse ==false){
        enableInterrupts();
        return -1;
    }
    //error: a thread trying to unlock lock that it doesnt have  (aka a lock that another thread is locked to)
    if(!(checkLock->threadID == runningBlock->id)){
        enableInterrupts();
        return -1;
    }
    
    //free lock
    checkLock->inUse=false;

    //get the node that is waiting on this lock and move to ready queue
    if(!(allLockQueues.find(currLock)  == allLockQueues.end() )){ //if the lock queue for this lock exists in map
        if(!(allLockQueues[currLock].empty())){ //if the lock queue isnt null aka there's still guys on here
            //fprintf(stderr, "unlock: popping off lock queue + adding to ready\n");
            //move thread from front of this lock waiting queue to end of ready queue
            ThreadBlock* nextThreadBlock = allLockQueues[currLock].front(); //get next node from front of lock queue
            allLockQueues[currLock].pop_front(); //remove it from lock q
            //printQueue(allLockQueues);

            readyWaitingQueue.push_back(nextThreadBlock);//put this next node onto ready waiting list
            //printReadyQ();

            // //change lock to busy
            // checkLock->inUse=true;
            // checkLock->threadID=nextThreadBlock->id;
        } 
    }

    lockTable[currLock] = checkLock; //copy lock back into lock table with new values
    //enable interrupts
    enableInterrupts();
    return 0;
}

int thread_signal(unsigned int currLock, unsigned int currCond){
    disableInterrupts();
    // fprintf(stderr, "signal start \n");
    // fprintf(stderr, "lock: %u\n", currLock);
    // fprintf(stderr, "cond: %u\n", currCond);
    if (haveRunLibinit == false){
        enableInterrupts();
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
         
                //***move head of contexts from waiting on cond to waiting on lock
                //fprintf(stderr, "signal: moving to ready queue\n");
                //get node from front of this condition queue
                ThreadBlock* moveThreadBlock = allCondQueues[currCond].front(); //get it
                allCondQueues[currCond].pop_front(); //remove it
               //printQueue(allCondQueues);
                //put node on end of lock queue corresponding to lock
                readyWaitingQueue.push_back(moveThreadBlock);
                //printReadyQ();
            
        }
    }
    enableInterrupts();
    return 0;

}

int thread_broadcast(unsigned int currLock, unsigned int currCond){
    disableInterrupts();
    // fprintf(stderr, "broadcast start \n");
    //     fprintf(stderr, "lock: %u\n", currLock);
    // fprintf(stderr, "cond: %u\n", currCond);
    if (haveRunLibinit == false){
        enableInterrupts();
        return -1;
    }
    //move all contexts from waiting on cond to waiting on lock
    if(!(allCondQueues.find(currCond) == allCondQueues.end())){ //that condition queue exists
       // if(!(allLockQueues.find(currLock) == allLockQueues.end())){ //lock queue exists
            while(!(allCondQueues[currCond].empty())){ //exists and its not null (aka there actually IS a node waiting on this cond)
                //***move head of contexts from waiting on cond to waiting on lock
                //fprintf(stderr, "broadcast: moving to ready queue\n");
                //get node from front of this condition queue
                ThreadBlock* moveThreadBlock = allCondQueues[currCond].front(); //get it
                allCondQueues[currCond].pop_front(); //remove it

                //put node on end of lock queue corresponding to lock
                readyWaitingQueue.push_back(moveThreadBlock);
            }
       // }
    }
    enableInterrupts();
    return 0;   
}


