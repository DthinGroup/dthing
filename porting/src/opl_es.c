
#include <vm_common.h>
#include <vmTime.h>
#include <opl_es.h>
#ifdef ARCH_X86
#include <windows.h>
#elif defined(ARCH_ARM_SPD)
#include <os_api.h>
#endif

struct ES_Mutex_s
{
#ifdef ARCH_X86
    CRITICAL_SECTION mutex;
#elif defined (ARCH_ARM_SPD)
	SCI_MUTEX_PTR mutex;
#endif
};

/* refer to eventcritical.h */
ES_Mutex* mutex_init(void)
{	
    ES_Mutex *mutex = CRTL_malloc(sizeof(ES_Mutex));
    if (mutex != NULL)
    {
#ifdef ARCH_X86
		InitializeCriticalSection(&(mutex->mutex));
#elif defined (ARCH_ARM_SPD)
		static int order = 1;
		char name[20] = {0};
		sprintf(name,"dvm_mutex_%d",order++);
		mutex->mutex = SCI_CreateMutex(name,SCI_INHERIT);
		SCI_TRACE_LOW("create mutex %s : 0x%x",name,mutex->mutex);
#endif
        
    }
    return mutex;
}

/* refer to eventcritical.h */
void mutex_lock(ES_Mutex* mutex)
{    
#ifdef ARCH_X86
	EnterCriticalSection(&(mutex->mutex));
#elif defined (ARCH_ARM_SPD)
	SCI_GetMutex(mutex->mutex,SCI_INVALID_BLOCK_ID != SCI_IdentifyThread() ? SCI_WAIT_FOREVER : 0);
#endif
}

/* refer to eventcritical.h */
void mutex_unlock(ES_Mutex* mutex)
{
#ifdef ARCH_X86
	LeaveCriticalSection(&(mutex->mutex));
#elif defined (ARCH_ARM_SPD)
	SCI_PutMutex(mutex->mutex);
#endif    
}

/* refer to eventcritical.h */
void mutex_destory(ES_Mutex* mutex)
{
	if(mutex ==NULL)
		return;
#ifdef ARCH_X86
	DeleteCriticalSection(&(mutex->mutex));    
#elif defined (ARCH_ARM_SPD)
	if(mutex->mutex !=NULL)
		SCI_DeleteMutex(mutex->mutex);
#endif      
	CRTL_free(mutex);
}


#define MAX_SEMAPHORE_COUNT  INT32_MAX


struct ES_Semaphore_s
{
#ifdef ARCH_X86
	HANDLE handle;
#elif defined (ARCH_ARM_SPD)
	SCI_SEMAPHORE_PTR handle;
#endif    
};

/* refer to eventcritical.h */
ES_Semaphore* semaphore_create(int32_t initialCount)
{
    ES_Semaphore *sema = CRTL_malloc(sizeof(ES_Semaphore));
    if (sema != NULL)
    {
#ifdef ARCH_X86
		sema->handle = CreateSemaphore(NULL, initialCount, MAX_SEMAPHORE_COUNT, NULL);
        if (sema->handle == NULL)
        {
            CRTL_free(sema);
            return NULL;
        }
#elif defined (ARCH_ARM_SPD)
		static int order = 1;
		char name[20] = {0};
		sprintf(name,"dvm_mutex_%d",order++);
		sema->handle = SCI_CreateSemaphore(name,initialCount);
		SCI_TRACE_LOW("create semaphore %s : 0x%x",name,sema->handle);
		if(sema->handle ==NULL){
			CRTL_free(sema);
			return NULL;
		}
#endif        
    }

    return sema;
}

/* refer to eventcritical.h */
int32_t semaphore_waitOrTimeout(ES_Semaphore* sema, int32_t timeout)
{
#ifdef ARCH_X86
	DWORD t, res;
#elif defined (ARCH_ARM_SPD)
	int32_t t, res;
#endif  
    
	if(sema ==NULL || sema->handle){
		return SEMAPHORE_FAILURE;
	}

    if (timeout > 0) 
    {
        t = timeout;
    } 
    else if (timeout == SEMAPHORE_TIMEOUT_NOWAIT)
    {        
#ifdef ARCH_X86
		t = 0;
#elif defined (ARCH_ARM_SPD)
		t = SCI_NO_WAIT;
#endif   
    }
    else if (timeout == SEMAPHORE_TIMEOUT_INFINITE)
    {
        
#ifdef ARCH_X86
		t = INFINITE;
#elif defined (ARCH_ARM_SPD)
		t = SCI_WAIT_FOREVER;
#endif 
    }
    else
    {
        return SEMAPHORE_FAILURE;
    }

#ifdef ARCH_X86
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
#elif defined (ARCH_ARM_SPD)
	res = SCI_GetSemaphore(sema->handle,t);
	if(res ==SCI_SUCCESS){
		return SEMAPHORE_SUCCESS;
	}else{
		return SEMAPHORE_FAILURE;
	}
#endif   
    
}

/* refer to eventcritical.h */
int32_t semaphore_signal(ES_Semaphore* sema)
{
	if(sema ==NULL || sema->handle){
		return SEMAPHORE_FAILURE;
	}

#ifdef ARCH_X86
	if (ReleaseSemaphore(sema->handle, 1, NULL))
    {
        return SEMAPHORE_SUCCESS;
    } 
    return SEMAPHORE_FAILURE;
#elif defined (ARCH_ARM_SPD)
	if(SCI_PutSemaphore(sema->handle)!=SCI_SUCCESS){
		return SEMAPHORE_FAILURE;
	}
	return SEMAPHORE_SUCCESS;
#endif   
}

/* refer to eventcritical.h */
int32_t semaphore_destroy(ES_Semaphore* sema)
{
    int32_t res = SEMAPHORE_FAILURE;

#ifdef ARCH_X86
	if (CloseHandle(sema->handle))
    {
        res = SEMAPHORE_SUCCESS;
    }
#elif defined (ARCH_ARM_SPD)
	if(SCI_DeleteSemaphore(sema->handle) ==SCI_SUCCESS){
		res = SEMAPHORE_SUCCESS;
	}
#endif   
    
    CRTL_free(sema);
    
    return res;
}

uint32_t EOS_getTimeBase()
{
    return vmtime_getTickCount();
}
