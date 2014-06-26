
#include <opl_es.h>
#ifdef WIN32
#include <windows.h>
#endif

struct ES_Mutex_s
{
    CRITICAL_SECTION cs;
};

/* refer to eventcritical.h */
ES_Mutex* mutex_init(void)
{
    ES_Mutex *mutex = CRTL_malloc(sizeof(ES_Mutex));
    if (mutex != NULL)
    {
        InitializeCriticalSection(&(mutex->cs));
    }
    return mutex;
}

/* refer to eventcritical.h */
void mutex_lock(ES_Mutex* mutex)
{
    EnterCriticalSection(&(mutex->cs));
}

/* refer to eventcritical.h */
void mutex_unlock(ES_Mutex* mutex)
{
    LeaveCriticalSection(&(mutex->cs));
}

/* refer to eventcritical.h */
void mutex_destory(ES_Mutex* mutex)
{
    DeleteCriticalSection(&(mutex->cs));
    CRTL_free(mutex);
}


#define MAX_SEMAPHORE_COUNT  INT32_MAX


struct ES_Semaphore_s
{
    HANDLE handle;
};

/* refer to eventcritical.h */
ES_Semaphore* semaphore_create(int32_t initialCount)
{
    ES_Semaphore *sema = CRTL_malloc(sizeof(ES_Semaphore));
    if (sema != NULL)
    {
        sema->handle = CreateSemaphore(NULL, initialCount, MAX_SEMAPHORE_COUNT, NULL);
        if (sema->handle == NULL)
        {
            CRTL_free(sema);
            return NULL;
        }
    }

    return sema;
}

/* refer to eventcritical.h */
int32_t semaphore_waitOrTimeout(ES_Semaphore* sema, int32_t timeout)
{
    DWORD t, res;

    if (timeout > 0) 
    {
        t = timeout;
    } 
    else if (timeout == SEMAPHORE_TIMEOUT_NOWAIT)
    {
        t = 0;
    }
    else if (timeout == SEMAPHORE_TIMEOUT_INFINITE)
    {
        t = INFINITE;
    }
    else
    {
        return SEMAPHORE_FAILURE;
    }

    res = WaitForSingleObject(sema->handle, t);

    if (res == WAIT_OBJECT_0)
    {
        return SEMAPHORE_SUCCESS;
    }
    else if (res == WAIT_TIMEOUT)
    {
        return SEMAPHORE_TIMEOUT;
    }
    else
    {
        return SEMAPHORE_FAILURE;
    }
}

/* refer to eventcritical.h */
int32_t semaphore_signal(ES_Semaphore* sema)
{
    if (ReleaseSemaphore(sema->handle, 1, NULL))
    {
        return SEMAPHORE_SUCCESS;
    } 
    return SEMAPHORE_FAILURE;
}

/* refer to eventcritical.h */
int32_t semaphore_destroy(ES_Semaphore* sema)
{
    int32_t res = SEMAPHORE_FAILURE;

    if (CloseHandle(sema->handle))
    {
        res = SEMAPHORE_SUCCESS;
    }

    CRTL_free(sema);
    
    return res;
}

uint32_t EOS_getTimeBase()
{
    return GetTickCount();
}
