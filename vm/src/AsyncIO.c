#include <vm_common.h>
#include <AsyncIO.h>

/*just be called in java thread!!!*/
void AsyncIO_callAgainWhenSignalled(void)
{
    Thread * curthd = dthread_currentThread();
    ASYNC_Notifier *notifier;

    DVM_ASSERT(curthd != NULL);
    notifier = curthd->asynNotifier;

    DVM_ASSERT(notifier->ownthread == curthd);
    notifier->asynIoState = ASYNC_IO_ASLEEP;
    notifier->timeout  = 0;
    notifier->notified = FALSE;
}

/*just be called in java thread!!!.
 *if mS =0,same as Async_callAgainWhenSignalled
 */
void AsyncIO_callAgainWhenSignalledOrTimeOut(int mS)
{
    Thread * curthd = dthread_currentThread();
    ASYNC_Notifier *notifier;

    DVM_ASSERT(curthd != NULL);
    notifier = curthd->asynNotifier;

    if(mS <0)
    {
        mS = 0;
    }
    DVM_ASSERT(notifier->ownthread == curthd);
    notifier->asynIoState = ASYNC_IO_ASLEEP;
    notifier->timeout  = mS;
    notifier->notified = FALSE;
}

/*just be called in java thread!!!.*/
ASYNC_Notifier * Async_getCurNotifier(void)
{
    Thread * curthd = dthread_currentThread();

    DVM_ASSERT(curthd != NULL);

    return curthd->asynNotifier;
}

/*just be called in java thread!!!.*/
vbool AsyncIO_firstCall(void)
{
    Thread * curthd = dthread_currentThread();
    ASYNC_Notifier *notifier;

    DVM_ASSERT(curthd != NULL);
    notifier = curthd->asynNotifier;

    return (notifier->asynIoState == ASYNC_IO_FIRST);
}

/*Just be called in other thread but java thread*/
void AsyncIO_notify(ASYNC_Notifier * notifier)
{
    Thread * thread = NULL;

    DVM_ASSERT(notifier != NULL);
    thread = notifier->ownthread;
    DVM_ASSERT(thread != NULL);
    DVM_ASSERT(notifier->asynIoState == ASYNC_IO_ASLEEP);
    notifier->notified = TRUE;  //set the signal
}

