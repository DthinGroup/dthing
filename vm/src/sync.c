#include "vm_common.h"
#include "sync.h"
#include "dthread.h"
#include "Object.h"
#include "schd.h"



Monitor * gMonitorList = NULL;



void Sync_init(void)
{
    gMonitorList = NULL;
}

void Sync_term(void)
{
    Sync_dvmFreeMonitorList();
    gMonitorList = NULL;
}

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
    MonitorWaitSet*     tWaitSet = NULL;
    MonitorWaitSet*     nWaitSet = NULL;

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
    DVM_MEMSET(tWaitSet,0,sizeof(MonitorWaitSet));
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
    MonitorWaitSet*     tWaitSet = NULL;
    MonitorWaitSet*     nWaitSet = NULL;
    MonitorWaitSet*     gaurd    = NULL;

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
    MonitorWaitSet*     tWaitSet = NULL;
    MonitorWaitSet*     nWaitSet = NULL;

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

MonitorWaitSet * Sync_findMonitorWaitSet(Monitor * mon,Thread * thd)
{
    MonitorWaitSet * mWaitSet = NULL;

    DVM_ASSERT(mon != NULL);
    DVM_ASSERT(thd != NULL);

    mWaitSet = mon->mWaitSet;
    while(mWaitSet != NULL)
    {
        DVM_ASSERT(mWaitSet->waitSet != NULL);
        if(mWaitSet->waitSet == thd )
        {
            return mWaitSet;
        }
        mWaitSet = mWaitSet->next;
    }

    return NULL;
}

MonitorWaitSet * Sync_findNextWakeInWaitSet(Monitor * mon,Thread * notthis,WAKE_COND_E cond)
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
                if(mWaitSet->waitSet->threadState == THREAD_TRYGET_MONITOR_SUSPENDED)
                {
                    return mWaitSet;
                }
            }
            else if(cond == COND_WAIT_MONITOR)
            {
                if( mWaitSet->waitSet->threadState == THREAD_WAIT_MONITOR_SUSPENDED ||
                    mWaitSet->waitSet->threadState == THREAD_TIMEWAIT_MONITOR_SUSPENDED )
                {
                    return mWaitSet;
                }
            }
            else
            {
                return mWaitSet;
            }
        }
        mWaitSet = mWaitSet->next;
    }

    return NULL;
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
        self->waitMonitor = NULL;
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
        self->waitMonitor = mon;
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
    MonitorWaitSet * wakeSet = NULL;
    Monitor * mon = NULL;

    assert(self != NULL);
    assert(self->threadState == THREAD_ACTIVE);
    assert(obj != NULL);

    mon = LW_MONITOR(obj->lock);
    if(mon == NULL || mon->owner != self)
    {
        DVM_ASSERT(0);
        dvmThrowIllegalMonitorStateException("object not locked by thread before wait()");
        return false;
    }

    DVM_ASSERT(mon->lockCount > 0);
    mon->lockCount --;

    if(mon->lockCount ==0)
    {
        /*give up the monitor*/
        mon->lockCount = 0;
        mon->owner = NULL;

        /*find next wait thread,ready to active it*/
        wakeSet = Sync_findNextWakeInWaitSet(mon,self,COND_WAIT_TRYGET);

        if(wakeSet != NULL)
        {
            /*The thread get the monitor directly!!!*/
            mon->lockCount = wakeSet->lockCount;
            mon->owner = wakeSet->waitSet;
            wakeSet->waitSet->waitMonitor = NULL;

            /*RESUME it*/
            Schd_ResumeToReady(wakeSet->waitSet);

            /*remove the wait thread from waitSet*/
            Sync_removeWaitSet(mon,wakeSet->waitSet);
        }
    }
    else
    {
        /*nothing to do,still hold the monitor*/
    }

    return true;
}


/*
 * Object.wait().  Also called for class init.
 */
void Sync_dvmObjectWait(Thread* self, Object *obj, s8 msec, s4 nsec,vbool interruptShouldThrow)
{
    Monitor* mon = NULL;
    vbool timed;
    MonitorWaitSet * wakeSet = NULL;


    DVM_ASSERT(self != NULL);
    DVM_ASSERT(obj  != NULL);

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
        dthread_suspend(self,THREAD_TIMEWAIT_MONITOR_SUSPENDED);
    }
    else
    {
        dthread_suspend(self,THREAD_WAIT_MONITOR_SUSPENDED);
    }
    self->waitMonitor = mon;
    /*find next wait thread,ready to active it*/
    wakeSet = Sync_findNextWakeInWaitSet(mon,self,COND_WAIT_TRYGET);

    if(wakeSet != NULL)
    {
        /*The thread get the monitor directly!!!*/
        mon->lockCount = wakeSet->lockCount;
        mon->owner = wakeSet->waitSet;
        wakeSet->waitSet->waitMonitor = NULL;

        /*RESUME it*/
        Schd_ResumeToReady(wakeSet->waitSet);

        /*remove the wait thread from waitSet*/
        Sync_removeWaitSet(mon,wakeSet->waitSet);
    }

    /*launch schdule*/
    SET_SCHEDULE();

#if 0

    waitMonitor(self, mon, msec, nsec, interruptShouldThrow);
#endif
}


/*
 * Object.notify().
 */
void Sync_dvmObjectNotify(Thread* self, Object *obj)
{
    MonitorWaitSet * wakeSet = NULL;
    Monitor * mon = NULL;

    DVM_ASSERT(self != NULL);
    DVM_ASSERT(obj  != NULL);

    mon = LW_MONITOR(obj->lock);

    if(mon == NULL || mon->owner != self)
    {
        dvmThrowIllegalMonitorStateException("object not locked by thread before wait()");
        return;
    }

    /*find next wait or time-wait thread,ready to active it*/
    wakeSet = Sync_findNextWakeInWaitSet(mon,self,COND_WAIT_MONITOR);

    if(wakeSet != NULL)
    {
        /*Transfer state to tryget,*/
        Schd_ChangeThreadState(wakeSet->waitSet,THREAD_TRYGET_MONITOR_SUSPENDED);
        wakeSet->waitSet->waitMonitor = self;
    }

    /*not give up control of monitor now! give up till MONITOR_EXIT*/
}

/*
 * Object.notifyAll().
 */
void Sync_dvmObjectNotifyAll(Thread* self, Object *obj)
{
    MonitorWaitSet * wakeSet = NULL;
    Monitor * mon = NULL;

    DVM_ASSERT(self != NULL);
    DVM_ASSERT(obj  != NULL);

    mon = LW_MONITOR(obj->lock);

    if(mon == NULL || mon->owner != self)
    {
        dvmThrowIllegalMonitorStateException("object not locked by thread before wait()");
        return;
    }

    while(true)
    {
        /*find next wait or time-wait thread,ready to active it*/
        wakeSet = Sync_findNextWakeInWaitSet(mon,self,COND_WAIT_MONITOR);

        if(wakeSet != NULL)
        {
            /*Transfer state to tryget,*/
            Schd_ChangeThreadState(wakeSet->waitSet,THREAD_TRYGET_MONITOR_SUSPENDED);
            wakeSet->waitSet->waitMonitor = self;
        }
        else
            break;
    }

    /*not give up control of monitor now! give up till MONITOR_EXIT*/
}

/*just check thread state is THREAD_TRYGET_MONITOR_SUSPENDED*/
//bResumeNow == TRUE maybe not safe,please notice!
void Sync_bindTryLockToMonitor(Thread * thd,vbool bResumeNow)
{
    Monitor * mon = NULL;
    MonitorWaitSet * mWaitset = NULL;
    DVM_ASSERT(thd != NULL);
    DVM_ASSERT(thd->threadState == THREAD_TRYGET_MONITOR_SUSPENDED);

    mon = thd->waitMonitor;
    DVM_ASSERT(mon != NULL);
    if(mon->owner != NULL)
    {
        return;
    }
    else
    {
        mWaitset = Sync_findMonitorWaitSet(mon,thd);
        DVM_ASSERT(mWaitset != NULL);
        /*The thread get the monitor directly!!!*/
        mon->lockCount = mWaitset->lockCount;
        mon->owner = thd;
        thd->waitMonitor = NULL;

        /*RESUME it*/
        if(bResumeNow)  //this is not safe!!!
        {
            Schd_ResumeToReady(thd);
        }
        else
        {
            thd->threadState = THREAD_READY;    //
        }

        /*remove the wait thread from waitSet*/
        Sync_removeWaitSet(mon,thd);
    }
}