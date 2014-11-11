
#ifndef __EVENT_BASE_H__
#define __EVENT_BASE_H__

#include <std_global.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * EVT priority. SYS and CRIT(critical) are only used by 
 * internal. Event priority is same as eventqueue index. 
 * Each event can use priority value to add into the 
 * corresponding event queue.
 * SYS: system events, only used by eventsystem itself for 
 *      some highest priority events.
 * CRIT: critical events, it's same as system event which
 *      only used for internal. But it may not be triggered
 *      immediately for some specical situation.
 * HIGH, MID, LOW: different priority events. 
 */
enum EventPriority_e
{
    EVT_PRIORITY_LOW = 0,
    EVT_PRIORITY_MID,
    EVT_PRIORITY_HIGH,
    EVT_PRIORITY_CRIT,
    EVT_PRIORITY_SYS, 
    EVT_PRIORITY_MAX
};

/* rename for coding usage */
#define EP_SYS  EVT_PRIORITY_SYS
#define EP_CRIT EVT_PRIORITY_CRIT
#define EP_HIGH EVT_PRIORITY_HIGH
#define EP_MID  EVT_PRIORITY_MID
#define EP_LOW  EVT_PRIORITY_LOW

/* event structure definitions */
typedef struct Event_t Event;
typedef int32_t (*EventCallback)(Event* evt, void* userData);

struct Event_t
{
    int32_t  evtId;
    int16_t  fsm_state;
    int16_t  priority;
    void*    userData;
    uint32_t triggerPoint;
    EventCallback evtcb;
};

#define newNormalEvent(eid, fsmState, uData, cb, evt) \
    do \
    { \
        (evt)->evtId = MARK_EVT_ID(RAMSCLIENT_MODULE_ID, (eid)); \
        (evt)->fsm_state = fsmState; \
        (evt)->priority = EP_LOW; \
        (evt)->userData = uData; \
        (evt)->triggerPoint = 0; \
        (evt)->evtcb = cb; \
    } \
    while(FALSE)

#define MODULE_ID_SHIFT (24)
#define MODULE_ID_SHIFT_MASK (~((1<<MODULE_ID_SHIFT)|(-(1<<MODULE_ID_SHIFT))))

#define MARK_EVT_ID(module_id, internal_evt_id) \
    (((module_id)<<MODULE_ID_SHIFT) | (internal_evt_id))

#define UNMARK_EVT_ID(es_id) ((es_id)&MODULE_ID_SHIFT_MASK)


#define ES_SECOND_CALL_MARK (0x1<<14)
#define ES_SECOND_CALL_MASK (~((1<<14)|(-(1<<14))))
#define FSM_UNMARK(fsmState) (fsmState&ES_SECOND_CALL_MASK)

/* unset state, it means to use module's recording state.*/
#define FSM_STATE_UNSET (-100)

/**
 * Returned current signalled event. The event must be being scheduled.
 * This API only can be called in event system task(platform side),
 * it's unknown if it's called in other tasks. (non-task dependence)
 * @return signalled event. Or NULL if no event is signalled.
 */
Event* ES_getCurrentSignalledEvent();

/**
 * Check current scheduled event whether is first-time to schedule.
 * @return TRUE means first-time, FALSE means not only 1 time to schedule.
 */
int32_t ES_firstScheduled();

/**
 * Some async IO operation needs next step trigger with timeout
 * value. Once timeout, the event will be scheduled again.
 * if timeout <= 0, the event will triggered by event queue's
 * scheduling.
 * @param timeout, the timeout value.
 * @return, the instance of next scheduling event. it will be used
 *      for ES_notify to wakeup next-time scheduling.
 */
Event* ES_scheduleAgainWithTimeout(int32_t timeout);

/**
 * Almost the same as ES_scheduleAgainWithTimeout. But this API will
 * make the scheduled event as first time to schedule. Means 
 * ES_firstScheduled() will return TRUE if this API is called.
 * @param timeout, the timeout value.
 * @return, the instance of next scheduling event.
 */
Event* ES_scheduleAgainAsFirstTimeWithTimeout(int32_t timeout);

/**
 * Wakeup the blocked event specified by instance.
 * @param evt, the event needs to be scheduled;
 * @return EVT_RES_SUCCESS means success, otherwise failure.
 */ 
int32_t ES_notify(Event *evt);

/**
 * Wakeup all blocked events.
 * @return EVT_RES_SUCCESS means success, otherwise failure.
 */
int32_t ES_notifyAll();


/* event system result */
typedef enum EvtSysResult_e ESResult;

enum EvtSysResult_e
{
    EVT_RES_SUCCESS        = 0,
    EVT_RES_FAILURE        = -1, //normal failure.
    EVT_RES_QUEUE_FULL     = -2,
    EVT_RES_QUEUE_EMPTY    = -3,
    EVT_RES_IVALIDE_PARAMS = -4,
    EVT_RES_NO_TIMING      = -5,
    EVT_RES_NO_CALLBACK    = -6,
    EVT_RES_WOULDBLOCK     = -7
};

/* predefined slave modules ID */
#define EVT_SYS_MODULE_ID    (1) /* self ID */
#define RAMSCLIENT_MODULE_ID (2)
#define AMS_MODULE_ID (3)

/* predefined system events */
#define EVT_SYS_INIT      (0xFF01)
#define EVT_SYS_REQUEST   (0xFF02)
#define EVT_SYS_EXIT      (0xFF03)


/**
 * All operations are considered as asynchronized. Before event system
 * launched, only has event system(itself) own. Different modules can 
 * through register mechanism to get a callback, event system will use
 * this callback to get new events from this module.
 * Indeed, some modules don't have execution task. We just call them
 * slave modules. These slave modules will be initialized by event system
 * task to drive.
 */
#define MODULE_NAME_LEN  (32)
typedef struct ModuleLifeCycle_s
{
    int32_t  module_id;
    uint8_t  module_name[MODULE_NAME_LEN];
    EventCallback lifecycleProcess;
    struct ModuleLifeCycle_s *next;
} ModuleLifeCycle;


#ifdef __cplusplus
}
#endif

#endif
