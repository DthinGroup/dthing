package java.net;

public final class NetConstants 
{
    private NetConstants() { }
    
    //defines from android/OsConstants.java
    public static final int AF_UNSPEC = 0;
    public static final int AF_UNIX = 1;
    public static final int AF_INET = 2;
    public static final int AF_INET6 = 10;    
    public static final int ECONNREFUSED = 111;
    
    public static final int AI_PASSIVE = 0x00000001;    /* get address to use bind() */
    public static final int AI_CANONNAME = 0x00000002;      /* fill ai_canonname */
    public static final int AI_NUMERICHOST = 0x00000004;    /* prevent host name resolution */
    public static final int AI_NUMERICSERV = 0x00000008;    /* prevent service name resolution */
    
    
    /*net opreation result as in cpl_net.h of jbed*/
	public static final int NET_OP_SUCCESS		= 0;		/** General success. */
	public static final int NET_OP_ERROR		= -1;		/** General failure. */
	public static final int NET_OP_CONNREFUSED	= -2;		/** Connection refused. */
	public static final int NET_OP_WOULDBLOCK	= -100;		/** Operation will be handled asynchronously */
	
	public static final int MSG_PEEK =2;
}