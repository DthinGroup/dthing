
#ifndef __OPL_ES_H__
#define __OPL_ES_H__

#include <std_global.h>

#ifdef __cplusplus
extern "C" {
#endif

/*********************************
 * Mutext definetions.
 **********************************/
typedef struct ES_Mutex_s ES_Mutex;

ES_Mutex* mutex_init(void);

void mutex_lock(ES_Mutex* mutex);

void mutex_unlock(ES_Mutex* mutex);

void mutex_destory(ES_Mutex* mutex);



/**********************************
 * Semaphore definitions.
 **********************************/
/**
 * Semaphore functions return result;
 */
/* means semaphore_ functions call succeeded */
#define SEMAPHORE_SUCCESS (0)
/* means semaphore_ functions call failure */
#define SEMAPHORE_FAILURE (-1)
/* means semaphore_waitOrTimeout returns with timeout */
#define SEMAPHORE_TIMEOUT (-2)

/**
 * parameters for semaphore_waitOrTimeout.
 */
#define SEMAPHORE_TIMEOUT_NOWAIT   (0)
#define SEMAPHORE_TIMEOUT_INFINITE (-1)

typedef struct ES_Semaphore_s  ES_Semaphore;

ES_Semaphore* semaphore_create(int32_t initialCount);

int32_t semaphore_waitOrTimeout(ES_Semaphore* sema, int32_t timeout);

int32_t semaphore_signal(ES_Semaphore* sema);

int32_t semaphore_destroy(ES_Semaphore* sema);

uint32_t EOS_getTimeBase();

#ifdef __cplusplus
}
#endif

#endif //__OPL_ES_H__
