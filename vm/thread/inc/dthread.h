
#ifndef __DTHREAD_H__
#define __DTHREAD_H__

#include "vm_common.h"
#include "interpState.h"

/* reserve this many bytes for handling StackOverflowError */
#define STACK_OVERFLOW_RESERVE  768

#define kMinStackSize       (512 + STACK_OVERFLOW_RESERVE)
#define kDefaultStackSize   (16*1024)   /* four 4K pages */
#define kMaxStackSize       (256*1024 + STACK_OVERFLOW_RESERVE)



#define READY_GUARD_ID      (1)
#define OTHER_GUARD_ID      (2)


/*thread state*/
typedef enum{
    THREAD_INITED =0,     //just init or born
    THREAD_READY,       //ready state,can to run
    THREAD_ACTIVE,      //just running at now,only one thread gets the state at one moment
    THREAD_TIME_SUSPENDED,   //be suspened
    THREAD_MONITOR_SUSPENDED,
    THREAD_DEAD,        //be dead,to delete the thread
}THREAD_STATE_E;
    
typedef (*CallFunc)(int) ;

/*
 * Interpreter control struction.  Packed into a long long to enable
 * atomic updates.
 */
typedef union  {
    volatile s8   all;
    struct {
        u2   subMode;
        u1    breakFlags;
        s1     unused;   /* for future expansion */
#ifndef DVM_NO_ASM_INTERP
        void* curHandlerTable;
#else
        void* unused;
#endif
    } ctl;
}InterpBreak;

/*Round to run*/
typedef struct dthread
{
	 /*
     * Interpreter state which must be preserved across nested
     * interpreter invocations (via JNI callbacks).  Must be the first
     * element in Thread.
     */
    InterpSaveState interpSave;

    u4 threadId;

	 /*
     * interpBreak contains info about the interpreter mode, as well as
     * a count of the number of times the thread has been suspended.  When
     * the count drops to zero, the thread resumes.
     */
    InterpBreak interpBreak;

	u1*         cardTable;

	/* current exception, or NULL if nothing pending */
    Object*     exception;

	vbool debugIsMethodEntry;
    
    THREAD_STATE_E threadState;
    
    u1 threadPrio;  //not support for now!
    
    /* start (high addr) of interp stack (subtract size to get malloc addr) */
    u1*         interpStackStart;    
    /* current limit of stack; flexes for StackOverflowError */
    const u1*   interpStackEnd;  
    /* interpreter stack size; our stacks are fixed-length */
    u4          interpStackSize;
    vbool       stackOverflowed;

	/* the java/lang/Thread that we are associated with */
	Object*     threadObj;

    
    u4 waitTimeSlice;      /* just 'useful' in READY state,to record wait-to-run time *
                            * in each READY thread,scheduler get one of READY threads holding *
                            * max waitTimeSlice to run */
    
    u4 sleepTime;           //if state is SUSPENDED and sleepTime > 0,when sleepTime <=0 ,reACTIVE it
    
	CallFunc	cb;

    struct dthread * next ;
    struct dthread * pre  ;
}Thread,*ThreadP;


/*
 *global vars
 */
extern Thread *currentThread;
extern Thread *readyThreadListHead;
extern Thread *otherThreadListHead;


/*
 *public functions declare
 */
Thread * allocThread(int stackSize);
void dthread_init(void);
void dthread_term(void);
void dthread_start(Thread * thread);
void dthread_stop(Thread * thread);
void dthread_suspend(Thread * thread);
void dthread_resume(Thread * thread);
void dthread_delete(Thread * thread);

#endif