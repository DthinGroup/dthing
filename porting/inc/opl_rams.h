
#ifndef __OPL_RAMS_H__
#define __OPL_RAMS_H__

#include <std_global.h>

#if defined(__cplusplus)
extern "C" {
#endif

/**
 * Thread process function defintion.
 * @argc arguments count.
 * @argv arguments content.
 * @return value to indicate the process function execution result.
 * NOTE:
 *      argc and argv are not mandatory to use. This delaration adds
 *      these 2 parameters is for funtion extension in future. But
 *      we can ignore them at 1st phaze
 */
typedef int32_t (*RAMSThreadFunc)(int argc, void* argv[]);

/**
 * See above.
 */
typedef int32_t (*DVMThreadFunc)(int argc, void* argv[]);

/**
 * create Remote AMS thread to handle event. After this API is called, the new 
 * thread will start automatically without extra work.
 * @ramsThreadFunc, the new thread process funtion. refer to above definition of
                    RAMSThreadFunc.
 * @argc, the arguments number which passes to new thread.
 * @argv, the arguments list which passes to new thread.
 * @return, the identifier of this created thread.
 */
int32_t ramsCreateClientThread(RAMSThreadFunc ramsThreadFunc, int argc, void* argv[]);

/**
 * See above.
 */
int32_t ramsCreateVMThread(DVMThreadFunc pDvmThreadProc, int argc, void* argv[]);

/**
 * Abastract remote server APIs for easily communication.
 * This API returns an instance of network with specified ip 
 * address and port number. Actually, server only support socket connection
 * at present, so the instance returns from this API is an instance of
 * socket. Anyway, it can be expanded for future if needs.
 * @addres, ip address of server.
 * @port,   port number of server listened.
 * @return  return the instance of this server for next operations.
 */
int32_t getRemoteServerInstance(uint32_t address, uint16_t port);


/**
 * Send packaged data from client to remote server. This API is working
 * on block mode.
 * @instance, the instance of server for communication. returns
 *            from API getRemoteServerInstance().
 * @buf, the packaged data to be sent.
 * @bufSize, the data size in bytes.
 * @return, the real data bytes that has been sent on success, or
 *          a negative number means send failure.
 */
int32_t sendData(int32_t instance, uint8_t* buf, int32_t bufSize);

/**
 * Recive remote AMS server data or message. The data is packaged
 * with predefined structures. See the Remote CSC documents.
 * This API is working on block mode. 
 * @instance, the instance of server for communication. returns
 *            from API getRemoteServerInstance().
 * @buf, the packaged data to be received.
 * @bufSize, maximum number of receiving data in bytes.
 * @return, number of bytes recived on success, or 0 means EOF has
 *          been reached, or negative number means error.
 */
int32_t recvData(int32_t instance, uint8_t* buf, int32_t bufSize);

/**
 * The same as recvData(). Only difference is this API is working on blocking mode with
 * give timeout value.
 * @params, see recvData() description.
 */
int32_t recvDataWithTimeout(int32_t instance, uint8_t* buf, int32_t bufSize, int32_t timeout);

/**
 * destroy the instance.
 * @instance, the instance of server for communication. returns
 *            from API getRemoteServerInstance().
 * @return, 0 means success, or a negative value for failure.
 */ 
int32_t destroyRemoteServerInstance(int32_t instance);


/**
 * LockObject defintions.
 */
#define LOCKOBJECT_TIMEOUT_NOWAIT (0)
#define LOCKOBJECT_TIMEOUT_INFINITE (-1)

/**
 * Create LockObject for rams.
 * The lock object depends on platform supportable, it can be semaphore,
 * mutex or event.
 * @return the instance of lock object.
 */
int32_t createLockObject();

/**
 * Destroy or end a lock object instance.
 * @instance, the lock object isntance which is returned by createLockObject
 * @return, TRUE means success, otherwise failure.
 */
bool_t  destroyLockObject(int32_t instance);


/**
 * Wait on a Lock object.
 * This API waits until the lock object condition is trigger, or the timeout 
 * value is reached.
 * @instance, the lock object isntance which is returned by createLockObject
 * @timeout, the time-out interval. 
 *           If the timeout value is greater than 0, then the function waits
 *           until the lockobject unblocked or timeout elapses. If the timeout
 *           value is LOCKOBJECT_TIMEOUT_NOWAIT, then the function returns
 *           immediately even if the semaphore value is zero. If the timeout value
 *           is LOCKOBJECT_TIMEOUT_INFINITE, the function waits forever until 
 *           the semaphore is unblocked by releaseLockObject().
 * @return, TRUE means success, otherwise failure.
 */
bool_t waitObjectSignalOrTimeout(int32_t instance, int32_t timeout);

/**
 * release the lockobject.
 * for different type lockojbect, it can have different implementations
 * 1) for semaphore, it can add a sema to ensure blocked tasks can get one lockobject.
 * 2) for mutext, it can release the mutex for lock object.
 *
 * @instance, the lock object isntance which is returned by createLockObject.
 * @return, TRUE means success, otherwise failure.
 */
bool_t releaseLockObject(int32_t instance);


#ifdef __cplusplus
}
#endif

#endif //__OPL_RAMS_H__
