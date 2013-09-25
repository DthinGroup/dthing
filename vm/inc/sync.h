#ifndef __SYNC_H__
#define __SYNC_H__

#include <dvmdex.h>
#include <dthread.h>

/*
 * Hash state field.  Used to signify that an object has had its
 * identity hash code exposed or relocated.
 */
#define LW_HASH_STATE_UNHASHED 0
#define LW_HASH_STATE_HASHED 1
#define LW_HASH_STATE_HASHED_AND_MOVED 3
#define LW_HASH_STATE_MASK 0x3
#define LW_HASH_STATE_SHIFT 0
#define LW_HASH_STATE(x) (((x) >> LW_HASH_STATE_SHIFT) & LW_HASH_STATE_MASK)

/*
 * Monitor accessor.  Extracts a monitor structure pointer from a fat
 * lock.  Performs no error checking.
 */
#define LW_MONITOR(x)  (Monitor*)((x) & ~(LW_HASH_STATE_MASK << LW_HASH_STATE_SHIFT))


#define SYNC_KEEP_LOCK_COUNT	(-1)

typedef struct MonitorWaitSet_s MonitorWaitSet;
typedef struct Monitor_s Monitor;

struct MonitorWaitSet_s 
{
	Thread * waitSet;
	int      lockCount;
	struct MonitorWaitSet_s * next;
};


typedef struct Monitor_s 
{
	int					lockCount;      /* owner's recursive lock depth */
    Thread*				owner;          /* which thread currently owns the lock? */    
    Object*				obj;            /* what object are we part of [debug only] */
    struct MonitorWaitSet_s*		mWaitSet;	    /* threads currently waiting on this monitor */
    struct Monitor_s*		next;

};

Monitor* Sync_dvmCreateMonitor(Object* obj);

/*
 * Free the monitor list.  Only used when shutting the VM down.
 */
void Sync_dvmFreeMonitorList(void);

/*
 * Get the object that a monitor is part of.
 */
Object* Sync_dvmGetMonitorObject(Monitor* mon);

/*if lockCount== -1;not change the original lockCount*/
void Sync_appendWaitSet(Monitor * mon,Thread * thd,int lockCount);

void Sync_removeWaitSet(Monitor * mon,Thread * thd);

void Sync_removeAllWaitSet(Monitor * mon);

vbool Sync_dvmLockObject(Thread* self, Object *obj);
/*
 * Implements monitorexit for "synchronized" stuff.
 *
 * On failure, throws an exception and returns "false".
 */
vbool Sync_dvmUnlockObject(Thread* self, Object *obj);

/*
 * Object.wait().  Also called for class init.
 */
void Sync_dvmObjectWait(Thread* self, Object *obj, s8 msec, s4 nsec,vbool interruptShouldThrow);

#endif /*__SYNC_H__*/