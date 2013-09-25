#include "vm_common.h"
#include "sync.h"
#include "dthread.h"
#include "Object.h"



Monitor * gMonitorList = NULL;

/*
 * Create and initialize a monitor.
 */
Monitor* Sync_dvmCreateMonitor(Object* obj)
{
    Monitor* mon = NULL;

    mon = (Monitor*) calloc(1, sizeof(Monitor));
    DVM_ASSERT(mon != NULL) ;

	DVM_MEMSET(mon,0,sizeof(Monitor));

    if (((u4)mon & 7) != 0) 
	{        
        DVM_ASSERT(0);
    }
    mon->obj = obj;

    /* replace the head of the list with the new monitor */
    do 
	{
        mon->next = gMonitorList;
		gMonitorList = mon;
    } while (0);

    return mon;
}

/*
 * Free the monitor list.  Only used when shutting the VM down.
 */
void Sync_dvmFreeMonitorList(void)
{
	Monitor* mon     = NULL;
    Monitor* nextMon = NULL;

    mon = gMonitorList;
    while (mon != NULL) 
	{
        nextMon = mon->next;
		Sync_removeAllWaitSet(mon);
        DVM_FREE(mon);
        mon = nextMon;
    }
}

/*
 * Get the object that a monitor is part of.
 */
Object* Sync_dvmGetMonitorObject(Monitor* mon)
{
    if (mon == NULL)
        return NULL;
    else
        return mon->obj;
}

/*if lockCount== -1;not change the original lockCount*/
void Sync_appendWaitSet(Monitor * mon,Thread * thd,int lockCount)
{
	MonitorWaitSet*		tWaitSet = NULL;
	MonitorWaitSet*		nWaitSet = NULL;

	DVM_ASSERT(mon != NULL);
	DVM_ASSERT(thd != NULL);

	tWaitSet = mon->mWaitSet;
	while(tWaitSet != NULL)
	{
		if(tWaitSet->waitSet == thd)
		{
			if(lockCount != SYNC_KEEP_LOCK_COUNT)
				tWaitSet->lockCount = lockCount;
			return;
		}
		tWaitSet = tWaitSet->next;
	}

	tWaitSet = (MonitorWaitSet*) DVM_MALLOC(sizeof(MonitorWaitSet));
	DVM_ASSERT(tWaitSet != NULL);
	tWaitSet->waitSet   = thd;
	tWaitSet->lockCount = (lockCount == SYNC_KEEP_LOCK_COUNT) ? 0 : lockCount;
	/*append to tail of list*/
	if(mon->mWaitSet == NULL)
	{
		mon->mWaitSet = tWaitSet;
	}
	else
	{
		nWaitSet = mon->mWaitSet;
		while(nWaitSet->next != NULL)
			nWaitSet = nWaitSet->next;
		nWaitSet->next = tWaitSet;
	}
	return;
}

void Sync_removeWaitSet(Monitor * mon,Thread * thd)
{
	MonitorWaitSet*		tWaitSet = NULL;
	MonitorWaitSet*		nWaitSet = NULL;
	MonitorWaitSet*		gaurd    = NULL;

	DVM_ASSERT(mon != NULL);
	DVM_ASSERT(thd != NULL);
	tWaitSet = mon->mWaitSet;
	gaurd    = mon->mWaitSet;
	while(tWaitSet != NULL)
	{		
		if(tWaitSet->waitSet == thd)
		{
			if(tWaitSet == mon->mWaitSet)
			{
				mon->mWaitSet = tWaitSet->next;
			}
			else
			{
				gaurd->next   = tWaitSet->next;
			}
			DVM_FREE(tWaitSet);			
			return;
		}
		gaurd = tWaitSet;		
		tWaitSet = tWaitSet->next;
	}
	return;
}

void Sync_removeAllWaitSet(Monitor * mon)
{
	MonitorWaitSet*		tWaitSet = NULL;
	MonitorWaitSet*		nWaitSet = NULL;

	DVM_ASSERT(mon != NULL);
	tWaitSet = mon->mWaitSet;

	while(tWaitSet != NULL)
	{
		nWaitSet = tWaitSet->next;
		DVM_FREE(tWaitSet);
		tWaitSet = nWaitSet;
	}
	return;
}

vbool Sync_dvmLockObject(Thread* self, Object *obj)
{
	Monitor * mon = NULL;
	vbool lockRet = 0;

	DVM_ASSERT(self != NULL);
    DVM_ASSERT(obj != NULL);

	if(LW_MONITOR(obj->lock) == NULL)
	{
		mon = Sync_dvmCreateMonitor(obj);
		obj->lock = (u4) mon;
	}
	else
	{
		mon = LW_MONITOR(obj->lock);
	}

	//考虑是否执行一次 Sync_removeWaitSet ?要保证lockCount的正确性

	if(mon->owner == NULL)  /*first time to lock monitor*/
	{
		mon->owner = self;
		mon->lockCount++;
		lockRet = 1;
	}
	else if(mon->owner == self)  /*self,recursively*/
	{
		mon->lockCount++;
		lockRet = 1;
	}
	else /*owned by other thread*/
	{
		/*fail ! to wait*/
		dthread_suspend(self,THREAD_TRYGET_MONITOR_SUSPENDED);
		Sync_appendWaitSet(mon,self,SYNC_KEEP_LOCK_COUNT);
		lockRet = 0;
	}
	return lockRet;
}

/*
 * Implements monitorexit for "synchronized" stuff.
 *
 * On failure, throws an exception and returns "false".
 */
vbool Sync_dvmUnlockObject(Thread* self, Object *obj)
{

    assert(self != NULL);
    assert(self->threadState == THREAD_ACTIVE);
    assert(obj != NULL);


}

typedef enum
{
    COND_WAIT_TRYGET,   //in try get state  cor to THREAD_TRYGET_MONITOR_SUSPENDED
    COND_WAIT_MONITOR,  //in wait monitor state
    COND_WAIT_ANY,      //in any state
}WAKE_COND_E;

Thread * Sync_findNextWakeInWaitSet(Monitor * mon,Thread * notthis,WAKE_COND_E cond)
{
	MonitorWaitSet * mWaitSet = NULL;

	DVM_ASSERT(mon != NULL);

	mWaitSet = mon->mWaitSet;
	while(mWaitSet != NULL)
	{
		DVM_ASSERT(mWaitSet->waitSet != NULL);
		if(mWaitSet->waitSet != notthis )
		{
			if(cond == COND_WAIT_TRYGET)
			{
				if(mWaitSet->waitSet ->threadState == THREAD_TRYGET_MONITOR_SUSPENDED)
				{
					return mWaitSet->waitSet;
				}
			}
			else if(cond == COND_WAIT_MONITOR)
			{
				if(mWaitSet->waitSet ->threadState == THREAD_WAIT_MONITOR_SUSPENDED)
				{
					return mWaitSet->waitSet;
				}
			}
			else
			{
				return mWaitSet->waitSet;
			}
		}
		mWaitSet = mWaitSet->next;
	}

	return NULL;
}

/*
 * Object.wait().  Also called for class init.
 */
void Sync_dvmObjectWait(Thread* self, Object *obj, s8 msec, s4 nsec,vbool interruptShouldThrow)
{
    Monitor* mon = NULL;
	vbool timed;
	Thread * wakeThd = NULL;

    
	DVM_ASSERT(self != NULL);
    DVM_ASSERT(mon  != NULL);

    mon = LW_MONITOR(obj->lock);
    
	if(mon == NULL || mon->owner != self)
    {
		dvmThrowIllegalMonitorStateException("object not locked by thread before wait()");
		return;
	}

	/*
     * Enforce the timeout range.
     */
    if (msec < 0 || nsec < 0 || nsec > 999999) 
	{
        dvmThrowIllegalArgumentException("timeout arguments out of range");
        return;
    }

	if (msec == 0 && nsec == 0) 
	{
        timed = false;
    } 
	else 
	{
        //absoluteTime(msec, nsec, &ts);
		//only usemsec,ignore nsec
        timed = true;
    }

	Sync_appendWaitSet(mon,self,mon->lockCount);
	mon->lockCount = 0;
	mon->owner = NULL;

	if(timed)
	{
	    self->sleepTime = msec;         //only usemsec,ignore nsec
		dthread_suspend(self,THREAD_WAIT_MONITOR_SUSPENDED);
	}
	else
	{
		dthread_suspend(self,THREAD_TIMEWAIT_MONITOR_SUSPENDED);
    }
    
    /*find next wait thread,ready to active it*/
	wakeThd = Sync_findNextWakeInWaitSet(mon,self,COND_WAIT_TRYGET);

	if(wakeThd != NULL)
	{
		
	}
    
    

#if 0
    
    waitMonitor(self, mon, msec, nsec, interruptShouldThrow);
#endif
}
