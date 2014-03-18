#include<nativeNet.h>
#include<opl_net.h>
#include "vm_common.h"



/*
 * Class:     java_net_NetNativeBridge
 * Method:    IsNetworkInited
 * Signature: ()Z
 */
void Java_java_net_NetNativeBridge_IsNetworkInited(const u4* args, JValue* pResult)
{
	int ret = OPL_NET_SUCCESS;
	printf("call IsNetworkInited!\n");
	
	RETURN_BOOLEAN(ret);
}

/*
 * Class:     java_net_NetNativeBridge
 * Method:    startUpNetwork
 * Signature: ()I
 */
//to activate pdp!
void Java_java_net_NetNativeBridge_startUpNetwork(const u4* args, JValue* pResult)
{
	int opVal =0;
	
	//ArrayObject * arr = (ArrayObject *)args[1];
	char * arrbuf = KNI_GET_ARRAY_BUF(args[1]);
	int    arrlen = KNI_GET_ARRAY_LEN(args[1]);

	memset(arrbuf,5,arrlen);

	printf("call startUpNetwork!\n");
	opVal = Opl_net_startup();
	
	

over:
	RETURN_INT(opVal);
}

/*
 * Class:     java_net_NetNativeBridge
 * Method:    socket0
 * Signature: (Z)I
 */
void Java_java_net_NetNativeBridge_socket0(const u4* args, JValue* pResult)
{
	int handle =0;
	

	int stream = (int) args[1];

	printf("call socket init!\n");

	if(stream)	//tcp
	{
		handle = Opl_net_streamSocket();
	}
	else		//udp
	{
		handle = Opl_net_datagramSocket();
	}
	
	RETURN_INT(handle);
}


void Java_java_net_NetNativeBridge_connect0(const u4* args, JValue* pResult)
{
	int ret =0;
	int sock = (int)args[1];
	int ip   = (int)args[2];
	int port = (int)args[3];
	int timeout  = (int)args[4];

	ret = Opl_net_connect(sock,ip,port,timeout);
	RETURN_INT(ret);
}

void Java_java_net_NetNativeBridge_recv0(const u4* args, JValue* pResult)
//(int sock,byte[] arr,int offset,int count)
{
	int sock = (int)args[1];
	char * arrbuf = KNI_GET_ARRAY_BUF(args[2]);
	int    arrlen = KNI_GET_ARRAY_LEN(args[2]);
	int offset = (int)args[3];
	int count  = (int)args[4];
	int ret =0;

	if(offset + count > arrlen)
	{
		//throw outofindex exception
		DVM_ASSERT(0);
	}
	ret = Opl_net_recv(sock,&arrbuf[offset],count);

	RETURN_INT(ret);
}	
	
void Java_java_net_NetNativeBridge_send0(const u4* args, JValue* pResult)
//(int sock,byte[] bytes,int offset,int count,int family,int ip,int port)
{
	int sock = (int)args[1];
	char * arrbuf = KNI_GET_ARRAY_BUF(args[2]);
	int    arrlen = KNI_GET_ARRAY_LEN(args[2]);
	int offset = (int)args[3];
	int count  = (int)args[4];
	int ret =0;
	
	if(offset + count > arrlen)
	{
		//throw outofindex exception
		DVM_ASSERT(0);
	}
	ret = Opl_net_send(sock,&arrbuf[offset],count,ip,port);

	RETURN_INT(ret);
	
}

/*
 * Class:     java_net_NetNativeBridge
 * Method:    recvfrom0
 * Signature: (Z)I
 */
void Java_java_net_NetNativeBridge_recvfrom0(const u4* args, JValue* pResult)
{
	int sock = (int)args[1];
	char * arrbuf = KNI_GET_ARRAY_BUF(args[2]);
	int    arrlen = KNI_GET_ARRAY_LEN(args[2]);
	int offset = (int)args[3];
	int count  = (int)args[4];
	int flags  = (int)args[5];
	int ip     = (int)args[6];
	int port   = (int)args[7];
	int conn   = (int)args[8];

	int ret = Opl_net_recvfrom(sock,&arrbuf[offset],count,ip,port);
}

/*
 * Class:     java_net_NetNativeBridge
 * Method:    sendto0
 * Signature: (Z)I
 */
//int sock,byte[] buff,int offset,int count,int flag,int ip,int port
void Java_java_net_NetNativeBridge_sendto0(const u4* args, JValue* pResult)
{
	int sock = (int)args[1];
	char * arrbuf = KNI_GET_ARRAY_BUF(args[2]);
	int    arrlen = KNI_GET_ARRAY_LEN(args[2]);
	int offset = (int)args[3];
	int count  = (int)args[4];
	int flags  = (int)args[5];
	int ip     = (int)args[6];
	int port   = (int)args[7];
	

}