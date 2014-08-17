#include <nativeNet.h>
#include <opl_net.h>
#include "vm_common.h"

/**
 * Class:     java_net_NetNativeBridge
 * Method:    isNetworkInited
 * Signature: ()Z
 */
void Java_java_net_NetNativeBridge_isNetworkInited(const u4* args, JValue* pResult) {
    jboolean ret = false;
    printf("call isNetworkInited!\n");

    RETURN_BOOLEAN(ret);
}

/**
 * Class:     java_net_NetNativeBridge
 * Method:    startUpNetwork
 * Signature: ([B)I
 *
 * To activate PDP!
 */
void Java_java_net_NetNativeBridge_startUpNetwork(const u4* args, JValue* pResult) {
    int ret = OPL_NET_SUCCESS;
    //ArrayObject * arr = (ArrayObject *)args[1];
    char* arrbuf = KNI_GET_ARRAY_BUF(args[1]);
    int arrlen = KNI_GET_ARRAY_LEN(args[1]);

    memset(arrbuf, 5, arrlen);

    printf("call startUpNetwork!\n");
    ret = Opl_net_startup();

    RETURN_INT(ret);
}

/**
 * Class:     java_net_NetNativeBridge
 * Method:    socket0
 * Signature: (Z)I
 */
void Java_java_net_NetNativeBridge_socket0(const u4* args, JValue* pResult) {
    int handle = 0;
    int stream = (int) args[1];

    printf("call socket init!\n");

    if (stream) { // TCP
        handle = Opl_net_streamSocket();
    } else { // UDP
        handle = Opl_net_datagramSocket();
    }

    RETURN_INT(handle);
}

/**
 * Class:     java_net_NetNativeBridge
 * Method:    connect0
 * Signature: (IIII)I
 */
void Java_java_net_NetNativeBridge_connect0(const u4* args, JValue* pResult) {
    int sock = (int) args[1];
    int ip = (int) args[2];
    int port = (int) args[3];
    int timeout = (int) args[4];

    int ret = Opl_net_connect(sock, ip, port, timeout);
    RETURN_INT(ret);
}

/**
 * Class:     java_net_NetNativeBridge
 * Method:    recv0
 * Signature: (I[BII)I
 */
void Java_java_net_NetNativeBridge_recv0(const u4* args, JValue* pResult) {
    int sock = (int) args[1];
    char* arrbuf = KNI_GET_ARRAY_BUF(args[2]);
    int arrlen = KNI_GET_ARRAY_LEN(args[2]);
    int offset = (int) args[3];
    int count = (int) args[4];

    // Java layer has verified offset, count and arrlen
    int ret = Opl_net_recv(sock, &arrbuf[offset], count);

    RETURN_INT(ret);
}

/**
 * Class:     java_net_NetNativeBridge
 * Method:    send0
 * Signature: (I[BII)I
 */
void Java_java_net_NetNativeBridge_send0(const u4* args, JValue* pResult) {
    int sock = (int) args[1];
    char* arrbuf = KNI_GET_ARRAY_BUF(args[2]);
    int arrlen = KNI_GET_ARRAY_LEN(args[2]);
    int offset = (int) args[3];
    int count = (int) args[4];

    // Java layer has verified offset, count and arrlen
    int ret = Opl_net_send(sock, &arrbuf[offset], count);

    RETURN_INT(ret);

}

/**
 * Class:     java_net_NetNativeBridge
 * Method:    recvfrom0
 * Signature: (I[BIIIIII)I
 */
void Java_java_net_NetNativeBridge_recvfrom0(const u4* args, JValue* pResult) {
    int sock = (int) args[1];
    char* arrbuf = KNI_GET_ARRAY_BUF(args[2]);
    int arrlen = KNI_GET_ARRAY_LEN(args[2]);
    int offset = (int) args[3];
    int count = (int) args[4];
    int flags = (int) args[5];
    int ip = (int) args[6];
    int port = (int) args[7];
    int conn = (int) args[8];

    int ret = Opl_net_recvfrom(sock, &arrbuf[offset], count, ip, port);

    RETURN_INT(ret);
}

/**
 * Class:     java_net_NetNativeBridge
 * Method:    sendto0
 * Signature: (I[BIIIII)I
 */
void Java_java_net_NetNativeBridge_sendto0(const u4* args, JValue* pResult) {
    int sock = (int) args[1];
    char* arrbuf = KNI_GET_ARRAY_BUF(args[2]);
    int arrlen = KNI_GET_ARRAY_LEN(args[2]);
    int offset = (int) args[3];
    int count = (int) args[4];
    int flags = (int) args[5];
    int ip = (int) args[6];
    int port = (int) args[7];

    int ret = Opl_net_sendto(sock, &arrbuf[offset], count, ip, port);

    RETURN_INT(ret);
}

/**
 * Class:     java_net_NetNativeBridge
 * Method:    closeSocket0
 * Signature: (I)I
 */
void Java_java_net_NetNativeBridge_closeSocket0(const u4* args, JValue* pResult) {
    int sock = (int) args[1];
    int ret = Opl_net_closeSocket(sock);

    RETURN_INT(ret);
}
