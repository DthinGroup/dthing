#ifndef __ASYNCIO_H__
#define __ASYNCIO_H__

#include "dthread.h"

typedef struct ASYNC_Notifier_s  ASYNC_Notifier;

struct ASYNC_Notifier_s
{
    Thread * ownthread;
    int  asynIoState;
    int  timeout;
    vbool notified;
};

/*corrspond to state in AsyncIO.java*/
#define ASYNC_IO_IDLE     (0)
#define ASYNC_IO_BUSY     (1)
#define ASYNC_IO_ASLEEP   (2)
#define ASYNC_IO_FIRST    (3)
#define ASYNC_IO_DONE     (4)
#define ASYNC_IO_TIMEOUT  (5)

#define ASYNC_Signalled(thd)     (thd->asynNotifier->notified == TRUE)
#define ASYNC_AllowTimeOut(thd)  (thd->asynNotifier->timeout > 0)
#define ASYNC_SetAIOState(thd,state) do{thd->asynNotifier->asynIoState = state;}while(0)


#include <vm_common.h>
#include <AsyncIO.h>

/*just be called in java thread!!!*/
void AsyncIO_callAgainWhenSignalled(void);

/*just be called in java thread!!!.
 *if mS =0,same as Async_callAgainWhenSignalled
 */
void AsyncIO_callAgainWhenSignalledOrTimeOut(int mS);

/*just be called in java thread!!!.*/
ASYNC_Notifier * Async_getCurNotifier(void);

/*just be called in java thread!!!.*/
vbool AsyncIO_firstCall(void);

/*Just be called in other thread but java thread*/
void AsyncIO_notify(ASYNC_Notifier * notifier);

#endif