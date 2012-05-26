// synch.cc 
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks 
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
    
    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    } 
    value--; 					// semaphore available, 
						// consume its value
    
    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments 
// Note -- without a correct implementation of Condition::Wait(), 
// the test case in the network assignment won't work!

//T11 Lock constructor implemented by Team 11
Lock::Lock(char* debugName) {
	lockOwnerThread = new Thread("TestingThread");
	lockStatus = 0;
	name = debugName;
	lockWaitQueue = new List();
}

//T11 Lock destructor implemented by Team 11
Lock::~Lock() {
	delete lockWaitQueue;
	delete lockOwnerThread;
}

//T11 Acquire method implemented by Team 11
void Lock::Acquire() {
	#ifdef CHANGED 
		IntStatus oldLevel = interrupt->SetLevel(IntOff);			//Disable interrupts
		if(lockOwnerThread == currentThread){						//If I'm the lock owner, do nothing and return
			printf("\nLock Owner is again trying to acquire the same lock\n");
			(void) interrupt->SetLevel(oldLevel);					//Restore interrupts
			return;
		}
		
		if(lockStatus == 0){										//If lock is FREE, let the thread Acquire it
			lockStatus = 1;
			lockOwnerThread = currentThread;
		}else{														//If lock is BUSY, send thread to wait queue
			lockWaitQueue->Append((void*)currentThread);
			currentThread->Sleep();
		}
		
		(void) interrupt->SetLevel(oldLevel);						//Restore interrupts
	#endif	
}

//T11 Release method implemented by Team 11
void Lock::Release() {
	#ifdef CHANGED
		IntStatus oldLevel = interrupt->SetLevel(IntOff);			//Disable interrupts
		if(lockOwnerThread != currentThread){						//If I'm not the lock owner print and return
			currentThread->Print();
			printf(" is not the owner thread for lock %s\n", getName());
			(void) interrupt->SetLevel(oldLevel);						//Restore interrupts		
			return;	
		}
		
		if(!lockWaitQueue->IsEmpty()){								//If lock waiting queue is not empty, make another thread the lockOwner and put it in
																	//Ready Queue
			Thread* poppedThread = (Thread*)lockWaitQueue->Remove();
			lockOwnerThread = poppedThread;
			scheduler->ReadyToRun(poppedThread);	
		}else{														//If no waiting thread, make lock FREE and lock owner NULL
			lockStatus = 0;
			lockOwnerThread = NULL;	
		}

		(void) interrupt->SetLevel(oldLevel);						//Restore interrupts
	#endif	
}

Condition::Condition(char* debugName) { 
	name = debugName;
	//waitingLock = new Lock("TestWaitLock");
	waitingLock = NULL;
	conditionWaitQueue = new List();
}
Condition::~Condition() { 
	delete conditionWaitQueue;
	delete waitingLock;
}

//T11 Condition-Wait method implemented by Team 11
void Condition::Wait(Lock* conditionLock) { 
	//ASSERT(FALSE);
	#ifdef CHANGED
		IntStatus oldLevel = interrupt->SetLevel(IntOff);			//disable Interrupts to make operation atomic
		
		if(conditionLock==NULL){									//Condition to check if the Lock passed is pointing to null
			printf("There is no lock to wait on");					//Print error message
			(void) interrupt->SetLevel(oldLevel);					//Restore Interrupts to previous state
			return;
		}
		
		if(waitingLock==NULL){										//Condition to check if this is the first lock on which condition-wait was called
			waitingLock=conditionLock;								//Make the condition Lock as the waiting Lock to enable sequencing	
		}
		
		if(conditionLock!=waitingLock){								//Condition to check if the lock on which condition-lock is called is actually a waiting 
																	//lock
			printf("The Lock on which you are calling wait is the same as waiting Lock");//Print error message
			(void) interrupt->SetLevel(oldLevel);					//Restore Interrupts to previous state
			return;	
		}
		
		//If condition lock passes all of the above tests, we can make the thread to wait
		
		conditionLock->Release();									//Release the lock for the current Thread
		conditionWaitQueue->Append((void *)currentThread);			//Insert the current Thread on condition wait Queue
		currentThread->Sleep();										//Change the state of current Thread to sleep
		conditionLock->Acquire();									//On Signal Acquire the lock to enter critical section
		
		(void) interrupt->SetLevel(oldLevel);						//Restore interrupts
	#endif	
}

//T11 Condition-signal method implemented by Team 11
void Condition::Signal(Lock* conditionLock) { 
	#ifdef CHANGED
		IntStatus oldLevel = interrupt->SetLevel(IntOff);			//disable Interrupts to make operation atomic
		
		if(conditionWaitQueue->IsEmpty()){							//If there is no thread waiting on a lock then return
			printf("There is no thread waiting\n");
			(void) interrupt->SetLevel(oldLevel);					//Restore interrupts
			return;
		}
		
		if(waitingLock!=conditionLock){								//If the lock on which signal is called is not the actual waiting lock print error msg.
			printf("The condition Lock on which signal is called is actually not the one waiting \n");//print error mesg
			(void) interrupt->SetLevel(oldLevel);					//Restore interrupts
			return;
		}
		
		//After certain condition checks WakeUp one thread from the Condition Wait Queue and bring it to Ready State
		if(conditionWaitQueue->IsEmpty()){							//Condition to check if conditionwaitqueue is emtpy  
			waitingLock=NULL;										//If so make waitingLock point to nothing
		}
		else{														//If condition wait queue is not empty
			Thread* poppedThread = (Thread*)conditionWaitQueue->Remove();//Wake up one thread from condition wait queue
			scheduler->ReadyToRun(poppedThread);					//and change its state to ready
		}
		
		(void) interrupt->SetLevel(oldLevel);						//Restore interrupts
	#endif
}

//T11 Condition-signal method implemented by Team 11
void Condition::Broadcast(Lock* conditionLock) { 
	#ifdef CHANGED
		//It is used to signal all threads in a condition wait Queue 
		while(!conditionWaitQueue->IsEmpty()){
			Signal(conditionLock);
		}
	#endif
}
