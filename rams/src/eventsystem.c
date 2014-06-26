#include <std_global.h>
#include <eventbase.h>
#include <eventsystem.h>
#include <ramsclient.h>
#include <opl_es.h>


#define IS_Q_FULL(evtQ) \
        ((((evtQ)->tail + 1) % (evtQ)->qsize) == ((evtQ)->head))

#define IS_Q_EMPTY(evtQ) \
        ((evtQ)->tail == (evtQ)->head)

#define IS_SYS_EVT(evt) \
        ((evt)->priority == EVT_PRIORITY_SYS || (evt)->priority == EVT_PRIORITY_CRIT)

#define INSIDE_Q(evtQ, idx)\
        (((evtQ)->head <= (evtQ)->tail) ? \
        ((idx >= (evtQ)->head)&&(idx < (evtQ)->tail)) : (!((idx >= (evtQ)->tail)&&(idx < (evtQ)->head))))

#define IS_EVENT_VALID(evt) \


/* global var of event system. */
static void *gES;
static ES_Mutex *esMutex;
static ES_Mutex *modMutex;
static Event *signalledEvent; /* current signalled event */

static int32_t eventSystem_lifecycleProcess(Event *evt, void *userData);
static int32_t eventSystem_mainLoop(Event *evt, void *userData);

/* pre-defined slave modules list */
static ModuleLifeCycle slaveModulesLifeCycle[] =
{
    {EVT_SYS_MODULE_ID, "eventSystem", eventSystem_lifecycleProcess, &slaveModulesLifeCycle[1]},
    {RAMSCLIENT_MODULE_ID, "ramsClient", ramsClient_lifecycleProcess, NULL},
};

static Event internalEvent[] =
{
    {MARK_EVT_ID(EVT_SYS_MODULE_ID, EVT_SYS_INIT), 0, EP_SYS, NULL, 0, eventSystem_mainLoop},
    {MARK_EVT_ID(EVT_SYS_MODULE_ID, EVT_SYS_EXIT), 0, EP_SYS, NULL, 0, eventSystem_mainLoop},
};

/**
 * Event system lifecycle control.
 */ 
static int32_t eventSystem_lifecycleProcess(Event *evt, void *userData)
{
    int32_t evtId = UNMARK_EVT_ID(evt->evtId);
    switch (evtId)
    {
    case EVT_SYS_INIT:
        //nothing to do.
        break;
    
    case EVT_SYS_EXIT:
        //nothing to do.
        break;

    default:
        break;
    }
    return EVT_RES_SUCCESS;
}

static int32_t eventSystem_mainLoop(Event *evt, void *userData)
{
    ModuleLifeCycle *pModlc = slaveModulesLifeCycle;

    mutex_lock(modMutex);
    
    do
    {
        if (userData != NULL)
        {
            pModlc = (ModuleLifeCycle *)userData;
            pModlc->lifecycleProcess(evt, NULL);
            break;
        }

        for (pModlc = slaveModulesLifeCycle; pModlc != NULL; pModlc = pModlc->next)
        {
            pModlc->lifecycleProcess(evt, NULL);
        }
    }
    while(FALSE);
    mutex_unlock(modMutex);

    return EVT_RES_SUCCESS;
}


/**
 * Popup an event from specify event queue.
 * @param eq, the specify event queue.
 * @param idx, the index of event in event queue which will be popup.
 *      if idx < 0 means just popup the first event;
 * @return the available event.
 */
static Event* popupEvent(EventQueue* eq, int32_t idx)
{
    Event* evt = NULL;

    if (idx >= 0)
    {
        if (INSIDE_Q(eq, idx))
        {
            int32_t i;
            int32_t tail;
            
            tail = (eq->head > eq->tail) ? (eq->tail + eq->qsize) : eq->tail;            
            /* copy event into unused one. */
            evt = &eq->evt[(eq->head - 1 + eq->qsize)%eq->qsize];
            CRTL_memcpy(evt, &eq->evt[idx], sizeof(Event));

            for (i = idx; i < (tail - 1); i++)
            {
                //compact event queue.
                CRTL_memcpy(&eq->evt[i%eq->qsize], &eq->evt[(i+1)%eq->qsize], sizeof(Event));
            }
            eq->tail = (int16_t)((eq->tail - 1 + eq->qsize) % eq->qsize);
        }
    }
    else
    {
        evt = &eq->evt[eq->head];
        eq->head = (int16_t)((eq->head + 1)%eq->qsize);
    }
    return evt;
}

/**
 * assign new time slice for each queue.
 */
static int32_t redistributeTimeSlice()
{
    EventQueue* evtq;
    evtq = (EventQueue*)gES;

    evtq[EP_HIGH].timing = HPEQ_TIME_SLICE;
    evtq[EP_MID].timing  = MPEQ_TIME_SLICE;
    evtq[EP_LOW].timing  = LPEQ_TIME_SLICE;

    return EVT_RES_SUCCESS;
}

/**
 * Get next available event.
 */
static int32_t getNextEvent(Event *evtOut)
{
    int32_t  idx;
    EventQueue* evtq;
    bool_t   critQEmpty = TRUE;
    bool_t   userQempty = TRUE;
    uint32_t critQGap = 0xFFFFFFFF;
    int32_t  res = EVT_RES_QUEUE_EMPTY;

    evtq = (EventQueue*)gES;
    if (evtOut == NULL)
    {
        return EVT_RES_IVALIDE_PARAMS;
    }

    mutex_lock(esMutex);

    if (!IS_Q_EMPTY(&evtq[EP_SYS]))
    {
        Event *tmpEvt;
        tmpEvt = popupEvent(&evtq[EP_SYS], -1);
        CRTL_memcpy(evtOut, tmpEvt, sizeof(Event));
        res = EVT_RES_SUCCESS;
        goto bail;
    }
    
    if (!IS_Q_EMPTY(&evtq[EP_CRIT]))
    {
        uint32_t curPoint;
        int32_t  tail;
        int32_t  ei = EP_CRIT;
        Event *evt = evtq[ei].evt;

        critQEmpty = FALSE;
        tail = (evtq[ei].head > evtq[ei].tail) ? (evtq[ei].tail + evtq[ei].qsize) :  evtq[ei].tail;     

        for (idx = evtq[ei].head; idx < tail; idx++)
        {
            curPoint = EOS_getTimeBase();
            if (evt[idx%evtq[ei].qsize].triggerPoint <= curPoint)
            {
                Event *tmpEvt;
                tmpEvt = popupEvent(&evtq[ei], idx%evtq[ei].qsize);
                CRTL_memcpy(evtOut, tmpEvt, sizeof(Event));
                res = EVT_RES_SUCCESS;
                goto bail;
            }
            else
            {
                uint32_t tmp = evt[idx%evtq[ei].qsize].triggerPoint - curPoint;
                critQGap = (critQGap > tmp ? tmp : critQGap);
            }
        }
    }

    for (idx = EP_HIGH; idx >= EP_LOW; idx--)
    {
        if (!IS_Q_EMPTY(&evtq[idx]))
        {
            if (evtq[idx].timing > 0)
            {
                Event *tmpEvt;
                tmpEvt = popupEvent(&evtq[idx], -1);
                CRTL_memcpy(evtOut, tmpEvt, sizeof(Event));
                res = EVT_RES_SUCCESS;
                goto bail;
            }
            userQempty = FALSE;
        }
    }

    if (!userQempty)
    {
        res = EVT_RES_NO_TIMING;
        goto bail;
    }
    
    if (!critQEmpty)
    {
        res = critQGap;
        goto bail;
    }
bail:

    mutex_unlock(esMutex);
    return res;
}

/**
 * Scheduled the event.
 * @param evt, the event which to be scheduled.
 * @return executed time eslaped while success. Otherwise
 *      return ESResult values.
 */
static int32_t execute(Event* evt)
{
    uint32_t startPoint;
    int16_t  diff;
    EventQueue* evtq;
    evtq = (EventQueue*)gES;

    if (evt == NULL)
    {
        return EVT_RES_IVALIDE_PARAMS;
    }
    
    /* record current signalled event for scheduled unit. */
    signalledEvent = evt;

    /**
     * Some sys events do not need CB. So, treat the return
     * value as success for system event.
     */
    if (evt->evtcb == NULL)
    {
        return EVT_RES_NO_CALLBACK;
    }

    startPoint = EOS_getTimeBase();

    evt->evtcb(evt, evt->userData);

    diff = (uint16_t)(EOS_getTimeBase() - startPoint);
    evtq[evt->priority].timing -= diff;
    
    /* reset signalled event. */
    signalledEvent = NULL;

    return diff;
}

/**
 * Push an event into event queue.
 * @param evt, the event will push into corresponding event queue.
 *       it depends on evt priority.
 * @param newEvt, the reference of event in the queue. if newEvt is 
 *       a NULL pointer, nothing will be output.
 * @return, EVT_RES_SUCCESS if succss. Other wise means
 *       failure. Refer to ESResult definitions to learn about
 *       different return values.
 */
static int32_t pushEvent(Event *evt, Event **newEvt)
{
    int32_t res = EVT_RES_SUCCESS;
    int32_t tail;
    int32_t priority = evt->priority;
    EventQueue *evtq, *eqTmp;

    mutex_lock(esMutex);
    evtq = (EventQueue*)gES;
    eqTmp = &evtq[priority];
    
    if (IS_Q_FULL(eqTmp))
    {
        res = EVT_RES_QUEUE_FULL;
        goto bail;
    }

    /**
     * Copy event data into event queue;
     * NOTE:
     *      userData is just a pointer references, caller should release this 
     *      parameter in the last fsm step. Otherwise, invalid pointer 
     *      may cause un-expect result.
     */
    tail = eqTmp->tail;
    eqTmp->evt[tail].evtId = evt->evtId;
    eqTmp->evt[tail].fsm_state = evt->fsm_state;
    eqTmp->evt[tail].priority = evt->priority;
    eqTmp->evt[tail].userData = evt->userData;
    eqTmp->evt[tail].triggerPoint = evt->triggerPoint;
    eqTmp->evt[tail].evtcb = evt->evtcb;
    if (newEvt != NULL && *newEvt != NULL)
    {
        *newEvt = &(eqTmp->evt[tail]);
    }
    eqTmp->tail = (int16_t)((eqTmp->tail + 1) % eqTmp->qsize);

bail:
    mutex_unlock(esMutex);
    return res;
}


/* refer to eventsystem.h */
int32_t ES_initial()
{
    void*   esBuf = NULL;
    int32_t req_size = (sizeof(EventQueue) * MAX_EVTQ_NUM) +
            (sizeof(Event) * EQ_MAX_SIZE);
    EventQueue* evtq;

    esBuf = CRTL_malloc(req_size);
    if (esBuf == NULL)
    {
        //DVMTraceErr("initialEventSystem - create event queue failure\n");
        return EVT_RES_FAILURE;
    }
    CRTL_memset(esBuf, 0x0, req_size);

    /* initial event queue structure */
    evtq = (EventQueue*)esBuf;
    /* lp eq */
    evtq[EP_LOW].evt = (Event*)(evtq + MAX_EVTQ_NUM);
    evtq[EP_LOW].qsize = LPEQ_SIZE;
    evtq[EP_LOW].timing = LPEQ_TIME_SLICE;
    /* mp eq */
    evtq[EP_MID].evt = (Event*)(evtq[EP_LOW].evt + evtq[EP_LOW].qsize);
    evtq[EP_MID].qsize = MPEQ_SIZE;
    evtq[EP_MID].timing = MPEQ_TIME_SLICE;
    /* hp eq */
    evtq[EP_HIGH].evt = (Event*)(evtq[EP_MID].evt + evtq[EP_MID].qsize);
    evtq[EP_HIGH].qsize = HPEQ_SIZE;
    evtq[EP_HIGH].timing = HPEQ_TIME_SLICE;
    /* critcal eq */
    evtq[EP_CRIT].evt = (Event*)(evtq[EP_HIGH].evt + evtq[EP_HIGH].qsize);
    evtq[EP_CRIT].qsize = CRITEQ_SIZE;
    evtq[EP_CRIT].timing = MAX_TIME_SLICE;
    /* system eq */
    evtq[EP_SYS].evt = (Event*)(evtq[EP_CRIT].evt + evtq[EP_CRIT].qsize);
    evtq[EP_SYS].qsize = SYSEQ_SIZE;
    evtq[EP_SYS].timing = MAX_TIME_SLICE;

    gES = (void*)esBuf;

    if ((esMutex = mutex_init()) == NULL || (modMutex = mutex_init()) == NULL)
    {
        return EVT_RES_FAILURE;
    }

    pushEvent(&internalEvent[0], NULL);

    signalledEvent = NULL;

    return EVT_RES_SUCCESS;
}

/* refer to eventsystem.h */
int32_t ES_final()
{
    CRTL_freeif(gES);
    mutex_destory(esMutex);
    esMutex = NULL;
    mutex_destory(modMutex);
    modMutex = NULL;
    signalledEvent = NULL;
    return EVT_RES_SUCCESS;
}

/* refer to eventsystem.h */
int32_t ES_pushEvent(Event* evt)
{
    if (evt == NULL)
    {
        return EVT_RES_FAILURE;
    }

    return pushEvent(evt, NULL);
}
/* refer to eventsystem.h */
int32_t ES_Schedule(int32_t timeSlice)
{
    int32_t timeRest = timeSlice;
    int32_t getRes;
    int32_t exeRes;
    Event   evt;
    bool_t  roundDone = FALSE;

    do
    {
        getRes = getNextEvent(&evt);
        switch (getRes)
        {
        case EVT_RES_SUCCESS:
            exeRes = execute(&evt);
            if (exeRes > 0)
            {
                timeRest -= exeRes;
                timeRest = timeRest > 0 ? timeRest : 0;
            }
            break;

        case EVT_RES_NO_TIMING:
            redistributeTimeSlice();
            break;

        case EVT_RES_QUEUE_EMPTY:
            roundDone = TRUE;
            timeRest = 0x7FFFFFFF;
            break;
        
        default:
            if (getRes > 0)
            {
                timeRest = getRes;
                roundDone = TRUE;
            }
            break;
        }

        if (roundDone)
        {
            //start next scheduling.
            break;
        }
    }
    while(timeRest > 0);

    return timeRest;
}

/* refer to eventsystem.h */
int32_t ES_registerModule(uint8_t *moduleName, EventCallback ecb)
{
    ModuleLifeCycle *pModLifeCycle, *pNode;
    int32_t len;
    Event   evt;

    if (moduleName == NULL || ecb == NULL)
    {
        return EVT_RES_IVALIDE_PARAMS;
    }

    pModLifeCycle = CRTL_malloc(sizeof(ModuleLifeCycle));
    if (pModLifeCycle == NULL)
    {
        return EVT_RES_IVALIDE_PARAMS;
    }

    mutex_lock(modMutex);
    for (pNode = slaveModulesLifeCycle; pNode->next != NULL; pNode = pNode->next);
    
    len = (int32_t)CRTL_strlen(moduleName);
    len = (MODULE_NAME_LEN - 1) < len ? (MODULE_NAME_LEN - 1) : len;
    CRTL_memcpy(pModLifeCycle->module_name, moduleName, len);
    pModLifeCycle->module_name[len] = '\0';

    pModLifeCycle->module_id = pNode->module_id + 1;
    pModLifeCycle->lifecycleProcess = ecb;
    pModLifeCycle->next = NULL;
    
    pNode->next = pModLifeCycle;
    
    /* push event to initial module. */
    CRTL_memcpy(&evt, &internalEvent[0], sizeof(Event));
    evt.userData = pModLifeCycle;
    ES_pushEvent(&evt);

    mutex_unlock(modMutex);

    return pModLifeCycle->module_id;
}

/* refer to eventsystem.h */
int32_t ES_deregisterModule(int32_t moduleId)
{
    ModuleLifeCycle *pNode, *pPrev;
    int32_t slaveModulesNum = sizeof(slaveModulesLifeCycle)/sizeof(slaveModulesLifeCycle[0]);

    if (moduleId <= slaveModulesLifeCycle[slaveModulesNum-1].module_id)
    {
        //only can deregister dynamically modules.
        return EVT_RES_FAILURE;
    }

    mutex_lock(modMutex);
    for (pPrev = slaveModulesLifeCycle, pNode = pPrev->next; pNode != NULL; pPrev = pNode, pNode = pNode->next)
    {
        if (pNode->module_id == moduleId)
        {
            pPrev->next = pNode->next;
            CRTL_freeif(pNode);
        }
    }
    mutex_unlock(modMutex);

    return EVT_RES_SUCCESS;
}



/* refer to eventbase.h */
Event* ES_getCurrentSignalledEvent()
{
    return signalledEvent;
}

/* refer to eventbase.h */
int32_t ES_firstScheduled()
{
    Event *evt = ES_getCurrentSignalledEvent();

    if (evt == NULL || ((evt->fsm_state)&ES_SECOND_CALL_MARK) != 0)
    {
        return FALSE;
    }

    return TRUE;
}

/* refer to eventbase.h */
Event* ES_scheduleAgainAsFirstTimeWithTimeout(int32_t timeout)
{
    Event *evt = ES_getCurrentSignalledEvent();
    Event *tmpEvt = NULL;

    if ((timeout) > 0)
    {
        evt->triggerPoint = EOS_getTimeBase() + timeout;
        evt->priority = EP_CRIT;
    }

    if (evt->fsm_state >= (int16_t)ES_SECOND_CALL_MARK)
    {
        //unmark second schedule
        evt->fsm_state = FSM_UNMARK(evt->fsm_state);
    }

    if (pushEvent(evt, &tmpEvt) == EVT_RES_SUCCESS)
    {
        return tmpEvt;
    }

    return NULL;
}

/* refer to eventbase.h */
Event* ES_scheduleAgainWithTimeout(int32_t timeout)
{
    Event *evt = ES_getCurrentSignalledEvent();
    Event *tmpEvt = NULL;

    if ((timeout) > 0)
    {
        evt->triggerPoint = EOS_getTimeBase() + timeout;
        evt->priority = EP_CRIT;
    }

    if (evt->fsm_state < (int16_t)ES_SECOND_CALL_MARK)
    {
        //add second schedule mark
        evt->fsm_state = (evt->fsm_state | (int16_t)ES_SECOND_CALL_MARK);
    }

    if (pushEvent(evt, &tmpEvt) == EVT_RES_SUCCESS)
    {
        return tmpEvt;
    }

    return NULL;
}

/* refer to eventbase.h */
int32_t ES_notify(Event *evt)
{
    if (evt == NULL)
    {
        return EVT_RES_IVALIDE_PARAMS;
    }
    evt->triggerPoint = EOS_getTimeBase();
    return EVT_RES_SUCCESS;
}

/* refer to eventbase.h */
int32_t ES_notifyAll()
{
    //TODO:notify all pending events.
    //un-supported yet!
    return FALSE;
}

