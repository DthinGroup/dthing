
#ifndef __DTHREAD_H__
#define __DTHREAD_H__

#include "vm_common.h"
#include "interpState.h"
//#include "sync.h"

/* reserve this many bytes for handling StackOverflowError */
#define STACK_OVERFLOW_RESERVE  768

#define kMinStackSize       (512 + STACK_OVERFLOW_RESERVE)
#define kDefaultStackSize   (16*1024)   /* four 4K pages */
#define kMaxStackSize       (32*1024 + STACK_OVERFLOW_RESERVE)



#define READY_GUARD_ID      (1)
#define OTHER_GUARD_ID      (2)


/*thread state*/
typedef enum{
    THREAD_INITED =0,     //just init or born
    THREAD_READY,       //ready state,can to run
    THREAD_ACTIVE,      //just running at now,only one thread gets the state at one moment
    THREAD_TIME_SUSPENDED,   //be suspened
    THREAD_TRYGET_MONITOR_SUSPENDED,   /*try to get monitor_enter*/
	THREAD_WAIT_MONITOR_SUSPENDED,		/*in wait state,obi.wait()*/
	THREAD_TIMEWAIT_MONITOR_SUSPENDED,  /*in timed wait state,obj.wait(10)*/
	THREAD_MONITOR_SUSPENDED,
	THREAD_ASYNCIO_SUSPENDED,	/*suspended by async io call*/
    THREAD_DEAD,        //be dead,to delete the thread
}THREAD_STATE_E;

struct dthread;
    
typedef void (*CallFunc)(struct dthread *) ;

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

typedef struct interpSchdSave
{	
	u2 inst;
	u2 vsrc1;
	u2 vsrc2;
	u2 vdst;
	u4 ref;

	Method * methodToCall;
	vbool methodCallRange;
    vbool jumboFormat;
}InterpSchdSave;

typedef struct dthread Thread;

/*Round to run*/
struct dthread
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

	u1 beBroken;	//be breaked by schduler this thread ? FALSE only for ghost thread,else TRUE
    
    /* start (high addr) of interp stack (subtract size to get malloc addr) */
    u1*         interpStackStart;    
    /* current limit of stack; flexes for StackOverflowError */
    const u1*   interpStackEnd;  
    /* interpreter stack size; our stacks are fixed-length */
    u4          interpStackSize;
    vbool       stackOverflowed;

	/* the java/lang/Thread that we are associated with */
	Object*     threadObj;

	/*new  add by nix*/
	Method * entryMthd;  // method of entry , "main" or "run" 	
    
    u4 waitTimeSlice;      /* just 'useful' in READY state,to record wait-to-run time *
                            * in each READY thread,scheduler get one of READY threads holding *
                            * max waitTimeSlice to run */
    
    u8 sleepTime;           //if state is SUSPENDED and sleepTime > 0,when sleepTime <=0 ,reACTIVE it
    
	u8 creatTime;           //thread create time

	CallFunc	cb;

	InterpSchdSave itpSchdSave;
	vbool bInterpFirst;		//first call into interpret?

	struct ASYNC_Notifier_s * asynNotifier;

	/* pointer to the monitor lock we're currently waiting on */
    /* guarded by waitMutex */
    /* TODO: consider changing this to Object* for better JDWP interaction */
    struct Monitor_s*    waitMonitor;

    /* links to the next thread in the wait set this thread is part of */
    struct dthread*     waitNext;

    struct dthread * next ;
    struct dthread * pre  ;
};





/*
 *global vars
 */
extern Thread *ghostThread;
extern Thread *currentThread;
extern Thread *readyThreadListHead;
extern Thread *otherThreadListHead;


/*
 *public functions declare
 */
Thread * dthread_alloc(int stackSize);
void     dthread_create(const Method * mth,Object* obj);
void     dthread_delete(Thread * thread);
void     dthread_term(void);
void     dthread_start(Thread * thread);
void     dthread_stop(Thread * thread);
void     dthread_suspend(Thread * thread,THREAD_STATE_E  newState);
void     dthread_resume(Thread * thread);
Thread * dthread_currentThread(void);

void     dthread_create_ghost(void);
void     dthread_fill_ghost(const Method * mth,Object* obj);
void     dthread_delete_ghost(void);
#endif
