//
//	synch.c
//
//	Routines for synchronization
//
//

#include "dlxos.h"
#include "process.h"
#include "synch.h"

static Sem sems[MAX_SEMS]; 	  //All semaphores in the system
static Lock locks[MAX_LOCKS]; //All locks in the system
static Cond conds[MAX_CONDS]; //All conditionals in the system


extern struct PCB *currentPCB; 
//----------------------------------------------------------------------
//	SynchModuleInit
//
//	Initializes the synchronization primitives: the semaphores
//----------------------------------------------------------------------
void 
SynchModuleInit()
{
    int i;
    dbprintf ('p', "Entering SynchModuleInit\n");
    for(i=0; i<MAX_SEMS; i++)
    {
        sems[i].inuse = 0;
    }
    for(i=0; i<MAX_LOCKS; i++)
    {
        locks[i].inuse = 0;
        locks[i].pid = 0;
    }
    for(i=0; i<MAX_CONDS; i++)
    {
        conds[i].inuse = 0;
    }
    dbprintf ('p', "Leaving SynchModuleInit\n");
}
//---------------------------------------------------------------------
//
//	SemInit
//
//	Initialize a semaphore to a particular value.  This just means
//	initting the process queue and setting the counter.
//
//----------------------------------------------------------------------
void
SemInit (Sem *sem, int count)
{
    QueueInit (&sem->waiting);
    sem->count = count;
}

//----------------------------------------------------------------------
// 	SemCreate
//
//	Grabs a Semaphore, initializes it and returns a handle to this
//	semaphore. All subsequent accesses to this semaphore should be made
//	through this handle
//----------------------------------------------------------------------
sem_t 
SemCreate(int count)
{
    sem_t sem;
    uint32 intrval;

    // grabbing a semaphore should be an atomic operation
    intrval = DisableIntrs();
    for(sem=0; sem<MAX_SEMS; sem++)
    {
      if(sems[sem].inuse==0)
      {
        sems[sem].inuse = 1;
	    break;
      }
    }
    RestoreIntrs(intrval);
    
    if(sem==MAX_SEMS)
      return INVALID_SEM;

    SemInit(&sems[sem], count);

    return sem;
}


//----------------------------------------------------------------------
//
//	SemWait
//
//	Wait on a semaphore.  As described in Section 6.4 of _OSC_,
//      OR section 2.3 of Modern Operating Systems,
//	we decrement the counter and suspend the process if the
//	semaphore's value is less than 0.  To ensure atomicity,
//	interrupts are disabled for the entire operation.  Note that,
//	if the process is put to sleep, interrupts will be OFF when
//	it returns from sleep.  Thus, we enable interrupts at the end of
//	the routine.
//
//----------------------------------------------------------------------
void
SemWait (Sem *sem)
{
    Link	*l;
    int		intrval;

    intrval = DisableIntrs ();
    dbprintf ('I', "Old interrupt value was 0x%x.\n", intrval);
    dbprintf ('s', "Proc 0x%x waiting on sem 0x%x, count=%d.\n", currentPCB,
	      sem, sem->count);
    sem->count -= 1;
    if (sem->count < 0) {
	l = QueueAllocLink ();
	QueueLinkInit (l, (void *)currentPCB);
	dbprintf ('s', "Suspending current proc (0x%x).\n", currentPCB);
	QueueInsertLast (&sem->waiting, l);
	ProcessSleep ();
    }
    RestoreIntrs (intrval);
}
int SemHandleWait(sem_t sem)
{
  if(sem>=0&&sem<MAX_SEMS)
  {
    if(sems[sem].inuse)
    {
      SemWait(&sems[sem]);
      return 0;
    }
    return 1;
  }
  else
  {
    return 1;
  }
}

//----------------------------------------------------------------------
//
//	SemSignal
//
//	Signal on a semaphore.  Again, details are in Section 6.4 of
//	_OSC_ OR section 2.3 of Modern Operating Systems.
//
//----------------------------------------------------------------------
void
SemSignal (Sem *sem)
{
    Link	*l;
    int		intrs;

    intrs = DisableIntrs ();
    dbprintf ('s', "Signalling on sem 0x%x, count=%d.\n", sem, sem->count);
    sem->count += 1;
    if (sem->count <= 0) {
	l = QueueFirst (&sem->waiting);
	QueueRemove (l);
	dbprintf ('s', "Waking up PCB 0x%x.\n", l->object);
	ProcessWakeup ((PCB *)(l->object));
	QueueFreeLink (l);
    }
    RestoreIntrs (intrs);
}

int SemHandleSignal(sem_t sem)
{
  if(sem>=0&&sem<MAX_SEMS)
  {
    if(sems[sem].inuse)
    {
      SemSignal(&sems[sem]);
      return 0;
    }
    return 1;
  }
  else
  {
    return 1;
  }
}


//-----------------------------------------------------------------------
//	LockCreate
//
//	LockCreate grabs a lock from the systeme-wide pool of locks and 
//	initializes it.
//	It also sets the inuse flag of the lock to indicate that the lock is
//	being used by a process. It returns a unique id for the lock. All the
//	references to the lock should be made through the returned id. The
//	process of grabbing the lock should be atomic.
//
//	If a new lock cannot be created, your implementation should return
//	INVALID_LOCK (see synch.h).
//----------------------------------------------------------------------
//Code mostly similar to SemCreate minus SemInit, so used same structure
lock_t LockCreate()
{
    lock_t lock;
    uint32 intrval;

    intrval = DisableIntrs();
    for(lock=0; lock<MAX_SEMS; lock++)
    {
      if(locks[lock].inuse==0)
      {
        locks[lock].inuse = 1;
	    break;
      }
    }
    RestoreIntrs(intrval);
    
    if(lock==MAX_LOCKS)
      return INVALID_LOCK;

    return lock;
}

//---------------------------------------------------------------------------
//	LockHandleAcquire
//
//	This routine acquires a lock given its handle. The handle must be a 
//	valid handle for this routine to succeed. In that case this routine 
//	returns a zero. Otherwise the routine returns a 1.
//
//	Your implementation should be such that if a process that already owns
//	the lock calls LockHandleAcquire for that lock, it should not block.
//---------------------------------------------------------------------------
int
LockHandleAcquire(lock_t lock)
{
    if((lock<0) || (lock>=MAX_LOCKS))
        return 1;
    
    if (!locks[lock].inuse)
        return 1;
    
    return 0;
}

//---------------------------------------------------------------------------
//	LockHandleRelease
//
//	This procedure releases the unique lock described by the handle. It
//	first checks whether the lock is a valid lock. If not, it returns 1.
//	If the lock is a valid lock, it should check whether the calling
//	process actually holds the lock. If not it returns 1. Otherwise it
//	releases the lock, and returns 0.
//---------------------------------------------------------------------------
int
LockHandleRelease(lock_t lock)
{
    if((lock<0) || (lock>=MAX_LOCKS))
        return 1;

    if(locks[lock].inuse)
    {
        locks[lock].inuse = 0;
        return 0;
    } else {
        return 1;
    }
}
//--------------------------------------------------------------------------
//	CondCreate
//
//	This function grabs a condition variable from the system-wide pool of
//	condition variables and associates the specified lock with
//	it. It should also initialize all the fields that need to initialized.
//	The lock being associated should be a valid lock, which means that
//	it should have been obtained via previous call to LockCreate(). 
//	
//	If for some reason a condition variable cannot be created (no more
//	condition variables left, or the specified lock is not a valid lock),
//	this function should return INVALID_COND (see synch.h). Otherwise it
//	should return handle of the condition variable.
//--------------------------------------------------------------------------
//This function also similar to SemCreate.  Structure remains mostly the same
cond_t
CondCreate(lock_t lock)
{
    cond_t cond;
    uint32 intrval;

    // grabbing a semaphore should be an atomic operation
    intrval = DisableIntrs();
    for(cond=0; cond<MAX_CONDS; cond++)
    {
      if(conds[cond].inuse==0)
      {
        conds[cond].inuse = 1;
	    break;
      }
    }
    RestoreIntrs(intrval);
    
    if(cond==MAX_CONDS)
      return INVALID_COND;


    return cond;
}

//---------------------------------------------------------------------------
//	CondHandleWait
//
//	This function makes the calling process block on the condition variable
//	till either ConditionHandleSignal or ConditionHandleBroadcast is
//	received. The process calling CondHandleWait must have acquired the
//	lock associated with the condition variable (the lock that was passed
//	to CondCreate. This implies the lock handle needs to be stored
//	somewhere. hint! hint!) for this function to
//	succeed. If the calling process has not acquired the lock, it does not
//	block on the condition variable, but a value of 1 is returned
//	indicating that the call was not successful. Return value of 0 implies
//	that the call was successful.
//
//	This function should be written in such a way that the calling process
//	should release the lock associated with this condition variable before
//	going to sleep, so that the process that intends to signal this
//	process could acquire the lock for that purpose. After waking up, the
//	blocked process should acquire (i.e. wait on) the lock associated with
//	the condition variable. In other words, this process does not
//	"actually" wake up until the process calling CondHandleSignal or
//	CondHandleBroadcast releases the lock explicitly.
//---------------------------------------------------------------------------
int
CondHandleWait(cond_t cond)
{
    if ((cond < 0) || (cond >= MAX_CONDS))
    {    
        return 1;
    } else {
        if (conds[cond].inuse)
        {    
            return 0;
        }
        return 1;
    }
}
//---------------------------------------------------------------------------
//	CondHandleSignal
//
//	This call wakes up exactly one process waiting on the condition
//	variable, if at least one is waiting. If there are no processes
//	waiting on the condition variable, it does nothing. In either case,
//	the calling process must have acquired the lock associated with
//	condition variable for this call to succeed, in which case it returns
//	0. If the calling process does not own the lock, it returns 1,
//	indicating that the call was not successful. This function should be
//	written in such a way that the calling process should retain the
//	acquired lock even after the call completion (in other words, it
//	should not release the lock it has already acquired before the call).
//
//	Note that the process woken up by this call tries to acquire the lock
//	associated with the condition variable as soon as it wakes up. Thus,
//	for such a process to run, the process invoking CondHandleSignal
//	must explicitly release the lock after the call is complete.
//---------------------------------------------------------------------------
int
CondHandleSignal(cond_t cond)
{
    if ((cond < 0) || (cond >= MAX_CONDS))
    {    
        return 1;
    } else {
        if (conds[cond].inuse)
        {    
            return 0;
        }
        return 1;
    }
}
//---------------------------------------------------------------------------
//	CondHandleBroadcast
//
//	This function is very similar to CondHandleSignal. But instead of
//	waking only one process, it wakes up all the processes waiting on the
//	condition variable. For this call to succeed, the calling process must
//	have acquired the lock associated with the condition variable. This
//	function should be written in such a way that the calling process
//	should retain the lock even after call completion.
//
//	Note that the process woken up by this call tries to acquire the lock
//	associated with the condition variable as soon as it wakes up. Thus,
//	for such a process to run, the process invoking CondHandleBroadcast
//	must explicitly release the lock after the call completion.
//---------------------------------------------------------------------------
int
CondHandleBroadcast(cond_t cond)
{
    if ((cond < 0) || (cond >= MAX_CONDS))
    {    
        return 1;
    } else {
        if (conds[cond].inuse)
        {    
            return 0;
        }
        return 1;
    }
}
