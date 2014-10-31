#ifndef __OPL_RAMS_H__
#define __OPL_RAMS_H__

#include <std_global.h>
#include <eventbase.h>

#if defined(__cplusplus)
extern "C" {
#endif

#define RAMS_RES_SUCCESS     EVT_RES_SUCCESS
#define RAMS_RES_FAILURE     EVT_RES_FAILURE
#define RAMS_RES_WOULDBLOCK  EVT_RES_WOULDBLOCK

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
typedef int32_t (*DVMThreadFunc)(int argc, char* argv[]);

/**
 * Startup and initialize network.
 * @return RAMS_RES_SUCCESS means success, otherwise failure.
 */
int32_t rams_startupNetwork();

/**
 * connect to rams server, setup TCP connection.
 * @param address, server ip address.
 * @param port, the service port on server side.
 * @instance, saved current connection instance.
 * @return RAMS_RES_SUCCESS means success, otherwise failure.
 */
int32_t rams_connectServer(int32_t address, uint16_t port, int32_t *instance);

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
int32_t rams_recvData(int32_t instance, uint8_t* buf, int32_t bufSize);

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
int32_t rams_sendData(int32_t instance, uint8_t* buf, int32_t bufSize);

/**
 * close the connection of rams server.
 * @param instance, instance of the connection.
 */
int32_t rams_closeConnection(int32_t instance);


/**
 * shutdown and finalize network.
 * @return RAMS_RES_SUCCESS means success, otherwise failure.
 */
int32_t rams_shutdownNetwork();


/**
 * create dalvik VM thread to handle event. After this API is called, the new 
 * thread will start automatically without extra work.
 * @ramsThreadFunc, the new thread process funtion. refer to above definition of
                    RAMSThreadFunc.
 * @argc, the arguments number which passes to new thread.
 * @argv, the arguments list which passes to new thread.
 * @return, the identifier of this created thread.
 */
int32_t rams_createVMThread(DVMThreadFunc pDvmThreadProc, int argc, void* argv[]);

bool_t rams_isNetworkActive(void);

#ifdef __cplusplus
}
#endif

#endif //__OPL_RAMS_H__
