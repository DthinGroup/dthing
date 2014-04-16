#include <std_global.h>
#include <opl_rams.h>
#ifdef WIN32
#include <windows.h>
#elif defined(ARCH_ARM_SPD)
//TODO:
#endif

#ifdef WIN32
static bool_t  wsaStarted = FALSE;
static int32_t lockObjInst;
#define SET_NON_BLOCKING(sock) \
    do { \
        unsigned long val = 1; \
        ioctlsocket(sock, FIONBIO, &val); \
    } while(0)
#endif

/* see opl_rams.h */
int32_t getRemoteServerInstance(uint32_t address, uint16_t port)
{
#ifdef WIN32
    int32_t fd;
    WSADATA wsaData;
    struct sockaddr_in sa;
    if (!wsaStarted && (WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR))
    {
        DVMTraceErr("WSAStartup failure, error code(%d)\n", WSAGetLastError());
        return -1;
    }
    wsaStarted = TRUE;
    fd = (int32_t)socket(PF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_SOCKET)
    {
        DVMTraceErr("getRemoteServerInstance: create socket failure\n");
        return -1;
    }

    lockObjInst = createLockObject();
    SET_NON_BLOCKING(fd);

    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    *((uint32_t *)&sa.sin_addr) = htonl(address);

    while (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) == SOCKET_ERROR)
    {
        int32_t errCode =  WSAGetLastError();
        if (errCode == WSAEWOULDBLOCK)
        {
            //non-blocking mode, wait 1000ms, then try again.
            waitObjectSignalOrTimeout(lockObjInst, 1000);
            continue;
        }
        else if (errCode == WSAEISCONN)
        {
            //already connected.
            break;
        }

        /* connected failure */
        DVMTraceErr("getRemoteServerInstance: socket connect error(%d)\n", WSAGetLastError());
        destroyLockObject(lockObjInst);
        closesocket(fd);
        return -1;
    }
    return fd;

#elif defined(ARCH_ARM_SPD)
    //TODO:
#endif
    return 0;
}

/* see opl_rams.h */
int32_t sendData(int32_t instance, uint8_t* buf, int32_t bufSize)
{
    int32_t ret;
#ifdef WIN32
    ret = send(instance, buf, bufSize, 0);
    while (ret == SOCKET_ERROR)
    {
        int32_t errCode =  WSAGetLastError();
        if (errCode == WSAEWOULDBLOCK)
        {
            //non-blocking mode, wait 1000ms, then try again.
            waitObjectSignalOrTimeout(lockObjInst, 1000);
            continue;
        }
        DVMTraceErr("getRemoteServerInstance: send error(%d)\n", errCode);
        break;
    }
#elif defined(ARCH_ARM_SPD)
    //TODO:
#endif

    return ret;
}

/* see opl_rams.h */
int32_t recvDataWithTimeout(int32_t instance, uint8_t* buf, int32_t bufSize, int32_t timeout)
{
    bool_t loop = TRUE;
    int32_t ret;
    do 
    {
#ifdef WIN32
    ret = recv(instance, buf, bufSize, 0);

    if (ret == SOCKET_ERROR && loop && timeout > 0)
    {
        int32_t errCode =  WSAGetLastError();
        DVMTraceErr("getRemoteServerInstance: recv error(%d)\n", errCode);
        if (errCode == WSAEWOULDBLOCK)
        {
            waitObjectSignalOrTimeout(lockObjInst, timeout);
            loop = !loop;
        }
    }
    else
    {
        break;
    }

#elif defined(ARCH_ARM_SPD)
    //TODO:
#endif
    } while(TRUE);

    return ret;
}

/* see opl_rams.h */
int32_t recvData(int32_t instance, uint8_t* buf, int32_t bufSize)
{
    return recvDataWithTimeout(instance, buf, bufSize, 0);
}

/* see opl_rams.h */
int32_t destroyRemoteServerInstance(int32_t instance)
{
#ifdef WIN32
    destroyLockObject(lockObjInst);
    closesocket(instance);
    WSACleanup();
    wsaStarted = FALSE;
#elif defined(ARCH_ARM_SPD)
#endif
    return 0;
}



#ifdef WIN32
static DWORD WINAPI ramsClientThreadFunc(PVOID pvParam)
{
    RAMSThreadFunc ramsProc = (RAMSThreadFunc)pvParam;
    ramsProc(0, NULL);
    return 0;
}
#elif defined(ARCH_ARM_SPD)
//TODO:
#endif

/* see opl_rams.h */
int32_t ramsCreateClientThread(RAMSThreadFunc pRamsThreadProc, int argc, void* argv[])
{
    UNUSED(argc);
    UNUSED(argv);
#ifdef WIN32
    {
        HANDLE   handle;

/** Ignored parameters. TBD
        uint32_t params[3] = {
            (uint32_t)pRamsThreadProc,
            argc, //argc is >= 0
            (uint32_t)argv
        };
*/
        handle = CreateThread(NULL, 0, ramsClientThreadFunc, (LPVOID)pRamsThreadProc, 0, 0);
        if (handle > 0)
            return (int32_t)handle;
    }
#elif defined(ARCH_ARM_SPD)
    //TODO:
#endif
    return 0;
}


static DWORD WINAPI VMThreadFunc(PVOID pvParam)
{
    uint32_t* params = (uint32_t*)pvParam;
    RAMSThreadFunc vmProc = (RAMSThreadFunc)params[0];
    int32_t argc = (int32_t)params[1];
    void** argv = (void**)params[2];

    vmProc(argc, argv);
    return 0;
}

int32_t ramsCreateVMThread(DVMThreadFunc pDvmThreadProc, int argc, void* argv[])
{
    HANDLE handle;
    static uint32_t params[3] = {0x0,};//static variable is better, any side effect?
    params[0] = (uint32_t)pDvmThreadProc;
    params[1] = argc;
    params[2] = (uint32_t)argv;

    handle = CreateThread(NULL, 0, VMThreadFunc, (LPVOID)params, 0, 0);
    if (handle > 0)
        return (int32_t)handle;
    return -1;
}

/* see opl_rams.h */
int32_t createLockObject()
{
#ifdef WIN32
    HANDLE loInst = CreateSemaphore(NULL, 0, 0xFF, NULL);
    if (loInst == NULL)
    {
        DVMTraceErr("createLockObject, failure!\n");
        return 0;
    }
    return (int32_t)loInst;
#elif defined(ARCH_ARM_SPD)
    //TODO:
#endif
}
/* see opl_rams.h */
bool_t  destroyLockObject(int32_t instance)
{
#ifdef WIN32
    if (CloseHandle((HANDLE)instance))
    {
        return TRUE;
    }
    return FALSE;
#elif defined(ARCH_ARM_SPD)
    //TODO:
#endif
}

/* see opl_rams.h */
bool_t waitObjectSignalOrTimeout(int32_t instance, int32_t timeout)
{
#ifdef WIN32
    int32_t tVal, res;
    if (timeout > 0)
    {
        tVal = timeout;
    }
    else if (timeout == LOCKOBJECT_TIMEOUT_NOWAIT)
    {
        tVal = 0;
    }
    else if (timeout == LOCKOBJECT_TIMEOUT_INFINITE)
    {
        tVal = INFINITE;
    }
    else
    {
        DVMTraceErr("waitObjectSignalOrTimeout, unknown timeout value(%d)\n", timeout);
        return FALSE;
    }
    res = WaitForSingleObject((HANDLE)instance, tVal);
    
    if (res == WAIT_FAILED)
    {
        DVMTraceErr("waitObjectSignalOrTimeout failure (%d)\n", GetLastError());
        return FALSE;
    }
    return TRUE;

#elif defined(ARCH_ARM_SPD)
    //TODO:
#endif
}
/* see opl_rams.h */
bool_t releaseLockObject(int32_t instance)
{
#ifdef WIN32
    if (ReleaseSemaphore((HANDLE)instance, 1, NULL))
    {
        return TRUE;
    }
    return FALSE;
#elif defined(ARCH_ARM_SPD)
    //TODO:
#endif
}
