package  java.net;

import java.io.IOException;
//import os.thread.*;


public class NetNativeBridge
{
	private NetNativeBridge()
	{}
	
	//check network wether inited ro not
	private native static boolean IsNetworkInited();
	
	//use to activate network(pdp)
	public native static int startUpNetwork(byte[] test);
		
	/*jni native*/
	private native static int socket0(boolean streaming);
	
	private native static int connect0(int sock,int ip,int port,int timeOuts);
	
	private native static int recv0(int sock,byte[] arr,int offset,int count);
	
	private native static int recvfrom0(int sock,byte[] buff,int offset,int count,int flags,int ip,int port,int connectted);
	
	private native static int send0(int sock,byte[] bytes,int offset,int count);
	
	private native static int sendto0(int sock,byte[] buff,int offset,int count,int flag,int ip,int port);
	
	//return socket handle if sucess
	//streaming  true:tcp false:udp
	public static int socket(boolean streaming) throws SocketException
	{
		int res = -1;
		byte[] arr = {1,2,3,4,5};
		NetNativeBridge.log("create socket,stream:" + (streaming ? "tcp":"udp"));
		if(!IsNetworkInited())
		{
			while (AsyncIO.loop()) 
			{
				res = startUpNetwork(arr);
			}
			
			NetNativeBridge.log("startUpNetwork :" + res);	
			if(res !=NetConstants.NET_OP_SUCCESS)
				throw new SocketException("in socket cons,start up network fail!");
		}
		
		int sock = socket0(streaming);
		if(sock == NetConstants.NET_OP_ERROR)
		{
			throw new SocketException("create socket fail!");
		}
		
		NetNativeBridge.log("create sock handle:" + sock);
		return sock;
	}
	
	public static void closeSocket(int sock_handle)
	{
		
	}
	
	public static int available(int sock_handle)
	{
		return 0;
	}
	
	public static void bind(int sock_handle,InetAddress address, int port)
	{
		int ip = address.getAddressToInt();
		int family = address.getFamily();
		
		//not need to support!!
	}
	
	public static void connect(int sock_handle, InetAddress address, int port, int timeoutMs) throws IOException
	{
		int ip = address.getAddressToInt();
		int family = address.getFamily();
		
		int ret =0;
		while (AsyncIO.loop()) 
		{
			ret = connect0(sock_handle, ip, port, timeoutMs);
		}
		if(ret != NetConstants.NET_OP_SUCCESS)
		{
			throw new IOException("connect to server:" + ip + ":" + port +" fail!");
		}
		
	}
	
	public static void setSocketOption(int sock_handle, int opt,int val)
	{
		
	}
	
	public static int getSocketOption(int sock_handle, int opt)
	{
		return 0;	
	}
	
	public static int getSocketLocalPort(int sock_handle)
	{
		return 0;
	}
	
	public static int recv(int sock_handle, byte[] bytes, int byteOffset, int byteCount, int flags) throws IOException
	{
		int cnt = 0;
		
		if(byteOffset + byteCount > bytes.length)
			throw new ArrayIndexOutOfBoundsException("array index out of bound!");
		
		while(AsyncIO.loop())
		{
			cnt = recv0(sock_handle,bytes,byteOffset,byteCount);
		}
		return cnt;	
	}
	
	
	public static int recvfrom(int sock_handle, byte[] bytes, int byteOffset, int byteCount, int flags,DatagramPacket pack,int isNativeConnected) throws IOException
	{
		int cnt =0;
		int ip=0;
		int port =0;
		if(isNativeConnected ==0 && pack ==null)
			throw new NullPointerException("datagram packet is null!");
		
		if(pack != null)
		{
			ip = pack.getAddress().getAddressToInt();
			port = pack.getPort();
		}
		
		if(isNativeConnected==0)
		{
			if(ip ==0 || port ==0)
				throw new SocketException("not specefied remote ip/port address");
		}
		
		if(byteOffset + byteCount > bytes.length)
			throw new ArrayIndexOutOfBoundsException("array index out of bound!");
		
		while(AsyncIO.loop())
		{
			cnt = recvfrom0(sock_handle,bytes,byteOffset,byteCount,flags,ip,port,isNativeConnected);
		}
		
		if(cnt <0)
			throw new IOException("recvfrom exception occurs!,return code is "+cnt);
		
		return cnt;
	}
	
	public static int send(int sock_handle, byte[] bytes, int byteOffset, int byteCount, int flags) throws IOException
	{
		int cnt = 0;
		
		if(byteOffset + byteCount > bytes.length)
			throw new ArrayIndexOutOfBoundsException("array index out of bound!");

		while(AsyncIO.loop())
		{
			cnt = send0(sock_handle,bytes,byteOffset,byteCount);
		}
		return cnt;	
	}
	
	
	public static int sendto(int sock_handle, byte[] bytes, int byteOffset, int byteCount, int flags,InetAddress address,int port) throws IOException
	{
		int cnt = 0;
		int ip = 0;
		
		if(address !=null)
		{
			ip = address.getAddressToInt();
		}
		
		if(byteOffset + byteCount > bytes.length)
			throw new ArrayIndexOutOfBoundsException("array index out of bound!");
		
		while(AsyncIO.loop())
		{
			cnt = sendto0(sock_handle,bytes,byteOffset,byteCount,flags,ip,port);
		}
		return cnt;
		
	}
	
	static void log(String log)
	{
		System.out.println("socket bridge:" + log);	
	}
}