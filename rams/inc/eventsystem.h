
#ifndef __EVENT_SYSTEM_H__
#define __EVENT_SYSTEM_H__

#include <std_global.h>
#include <eventbase.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * There are 5 pre-defined event queue:
 *     1. Low priority event queue
 *     2. Middle priority event queue
 *     3. High priority event queue
 *     4. Critical priority event queue
 *     5. System priority event queue
 * if add or reduce event queue, this number needs to be changed according
 * to new event queue numbers.
 */
#define MAX_EVTQ_NUM  EVT_PRIORITY_MAX

/* Put different priority events into differnt Queue for better
 * management. Especially, we can use timing slice to schedule
 * different priority event to emulat multi-tasks in a single
 * thread. It can better handle aysnc IO operations. 
 */
/* low priority event queue maxium size */
#define LPEQ_SIZE   (512)
/* middle(normal) priority event queue maxium size */
#define MPEQ_SIZE   (128)
/* high priority event queue maxium size */
#define HPEQ_SIZE   (128)
/* SYSEQ and IDLEEQ are only used by interal */
#define SYSEQ_SIZE  (64)
#define CRITEQ_SIZE (64)

#define EQ_MAX_SIZE \
    (SYSEQ_SIZE + CRITEQ_SIZE + HPEQ_SIZE + MPEQ_SIZE + LPEQ_SIZE)


/* time slice for different eq, unit: ms */
#define HPEQ_TIME_SLICE (200)
#define MPEQ_TIME_SLICE (100)
#define LPEQ_TIME_SLICE (50)

/* Maxium scheduling time slice, unit: ms */
#define MAX_TIME_SLICE (0xFFFFFFFF)

/**
 * Event Queue data structure:
 *     Basically, we have 5 different pre-defined priority event queues. All of 
 *     them are stored in a successive storage region for better save space.
 *     All events are gotten automatically in current task, that means, the event queue
 *     is transparent to other tasks. Eevent system task will loop IO interfaces to get new
 *     events and put them into the queue. In other timing slice, the event will be executed
 *     in current task.
 *             +------------------------------+
 * eq[0]       | head | tail | timing | event | //System priority event queue
 *             +------------------------------+
 * eq[1]       | head | tail | timing | event | //High priority event queue
 *             +------------------------------+
 * eq[2]       | head | tail | timing | event | //Middle priority event queue
 *             +------------------------------+
 * eq[3]       | head | tail | timing | event | //Low priority event queue
 *             +------------------------------+
 * eq[4]       | head | tail | timing | event | //Idle priority event queue
 *             +------------------------------+
 * eq[0].event | evt1 | evt2 | ....(LPEQ_SIZE)
 *             +------------------------------+
 * eq[1].event | evt1 | evt2 | ....(MPEQ_SIZE)
 *             +------------------------------+
 * eq[2].event | evt1 | evt2 | ....(HPEQ_SIZE)           
 *             +------------------------------+
 * eq[3].event | evt1 | evt2 | ....(CRITEQ_SIZE)
 *             +------------------------------+
 * eq[4].event | evt1 | evt2 | ....(SYSEQ_SIZE)
 *             +------------------------------+
 */
typedef struct EventQueue_t EventQueue;

struct EventQueue_t
{
    int16_t  head;
    int16_t  tail;
    int32_t  qsize;
    /**
     * for LP,MP,HP events, it means the time taking.
     * for CRIT events, it means the next trigger point.
     * NOTE:
     *     uint32_t means only support around 49 days.
     *     consider to use 64bit integer?
     */
    uint32_t timing;
    Event*   evt;
};

/**
 * Initialize event system.
 * Create event queue and event structure as description
 * in <eventsystem.h>.
 * @return EVT_SUCCESS means initial success, otherwise failure.
 */
int32_t ES_initial();

/**
 * Finalize event system.
 * @return EVT_SUCCESS means final success, otherwise failure.
 */
int32_t ES_final();

/**
 * Push an event into event queue for scheduling.
 */
int32_t ES_pushEvent(Event* evt);

/**
 * scheduled with specified time slice.
 * @param timeSlice, time slice.
 * @return ms of task can be yeild, or -1 means exit es.
 */
int32_t ES_Schedule(int32_t timeSlice);

/**
 * register event request callback.
 * Event system will request to update new events from requested
 * module if register. However, the required point(timing) is decided 
 * by event system itself. Generally, when all event queues are
 * empty or give a request event mannually.
 * @param moduleName, the name of current module for tracking.
 * @param ecb, the callback of modules which event system can retrieve new
 *      events from.
 * @return, return assigned module id if success, otherwise return EVT_RES_FAIL.
 */
int32_t ES_registerModule(uint8_t *moduleName, EventCallback ecb);

/**
 * deregister event request callback.
 * @ecb, the callback of module which will be deregistered.
 * @return return EVT_RES_SUCCESS if success, otherwise return EVT_RES_FAIL.
 */
int32_t ES_deregisterModule(int32_t moduleId);


#ifdef __cplusplus
}
#endif

#endif//__EVENT_SYSTEM_H__
