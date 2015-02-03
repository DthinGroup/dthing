package jp.co.cmcc.wmmp;

import java.io.*;
import java.net.*;

import com.yarlungsoft.util.CRC16;
import com.yarlungsoft.util.DigitalEncrypt;
import com.yarlungsoft.util.StringOps;

//import com.sun.cldc.io.ConnectionBaseInterface;
import java.util.*;

/* for J2SE
import java.net.*;   //J2se
import java.io.*;
import java.util.*;
import java.lang.*;
import javax.microedition.io.*;
import com.yarlung.util.StringOps;
import java.util.Date;
import java.security.*;
import java.util.zip.*;  //for CRC32
*/

public class wmmp_protocol implements WMMPConnection {
    private static final boolean DEBUG = true;

    private static final int TCP_READ_EACH  = 1024;
    private static final int UDP_MAX_LEN = 1500;

    private static int link_flow_id =0;

    private boolean support_sum = false;   //ÊÇ·ñÖ§³ÖÕªÒªÌå
    //use tcp as link protocol in default
    private String link_protocol;
    private String link_ip;
    private int    link_port;

    private String wmmp_appip;
    private String wmmp_appport;
    private String wmmp_bearertype;

    //field option value
    /*default:1 detail in wmmp spec*/
    private volatile int fragment_val   = 1;
    /*default:0 detail in wmmp spec*/
    private volatile int keepalive_val  = 0;
    /*default:0 detail in wmmp spec*/
    private volatile int linger_val     = 0;

    private volatile boolean isLongConnect = false;  //ÊÇ·ñ³¤Á¬½Ó

    /*ÅÐ¶Ï³¤Á¬½Ó¼ä¸ô£º¼ÇÂ¼×îºóÓÐÊý¾ÝÊÕ·¢ÐÐÎªµ½1970-1-1 Á÷ÊÅµÄÃëÊý£¬Ö»ÒªÁ´Â·ÓÐÊý¾Ý£¬ÎÞÂÛÊÕ»¹ÊÇ·¢£¬¶¼¸üÐÂ¼ÇÂ¼£¬
     *×¨ÃÅ¿ªÆôÒ»¸öHeartBeatWorker·þÎñÀ´²éÑ¯Õâ¸ö¼ÇÂ¼Ê±¼ä£¬´ïµ½heart_beat_intervalÃëÃ»ÓÐÊ±¼ä¸üÐÂÊ±£¬¾Í·¢ËÍÐÄÌø°ü
     */
    private volatile int lastDataConnectTime = 0; //µ¥Î» ÎªÃë

    /*ÉèÖÃµÄ¼¸¸öÍ¨ÐÅ×´Ì¬*/
    /* state flow:
     *
     *
     */
    private static final int STATE_NOTLINK  = 0;  /*not linked state*/
    private static final int STATE_LINKING  = 1;  /*state in linking*/
    private static final int STATE_SENDING  = 2;  /*state in sending*/
    private static final int STATE_SENDFAIL = 3;  /*state in send fail*/
    private static final int STATE_WAITACK  = 4;  /*state in wait ack*/
    private static final int STATE_FAIL     = 5;  /*state in fail,to break the connect*/

    private volatile int curLinkState ; /*link state*/

    /*ÕâÀï¿ÉÒÔ¿¼ÂÇÊ¹ÓÃVector´æ´¢¼üÖµ¶Ô*/
    //seconds in unit
    private int heart_beat_interval = 5;
    //seconds in unit
    private int heart_beat_timeout  = 15;
    //times in unit
    private int heart_beat_times    = 3;

    private int cur_waitack_timeout ;   /*wait time for ack,unit in second*/
    private int continus_fail_times ;   /*if > heart_beat_times,close connect*/

    private int link_timestamp = 0;     /*time stamp in this link*/

    /*tcp stream vars in J2me Gcf*/
    private Socket  sockConnection;
    /*tcp stream vars in J2se*/

    /*for J2SE
    //private SocketAddress sAddress;
    //private Socket socket ;
*/
    private InputStream       tcpInStream;    //
    private OutputStream      tcpOutStream;

    /*exit flag*/
    private volatile boolean recv_work_exit  = false;
    private volatile boolean hbeat_task_exit = false;

    private volatile boolean recv_work_exited  = false;
    private volatile boolean hbeat_task_exited = false;

    //Check whether login
    private volatile boolean islogin = false;

    /*udp gram vars,for GCF*/
//    private UDPDatagramConnection  asServeConnect ;
//    private UDPDatagramConnection  asClientConnect ;

    private byte[] lastMsgRecord;   /*record last send msg*/

    /** Flag indicating if the connection is open. */
    protected boolean connectionOpen = false;


    /*ÓÃ×÷Õû¸öÐ­ÒéµÄ»¥³â£ºÃ¿´Î·¢ÏûÏ¢µ½Æ½Ì¨µÄÊ±ºò£¬mutex.wait(),Æ½Ì¨»ØÏûÏ¢Ö®ºó£¬ÔÙnotify.
     *ËùÓÐµÄwmmpÐ­Òé½Ó¿Ú¶¼ÊÇÐòÁÐ»¯µÄ£¬²»ÄÜ±»Í¬Ê±µ÷ÓÃ£¬±ØÐëÒ»¸ö½Ó¿ÚÓÐÁË·µ»Ø²ÅÄÜµ÷ÓÃÏÂÒ»¸ö½Ó¿Ú
     */
    private Object mutex = new Object();

    /*ÓÃ×÷Ã¿´ÎµÄ²Ù×÷½á¹û·µ»ØÖµ£¬ËùÓÐ²Ù×÷¹²ÓÃÕâ¸ö±äÁ¿£¬ÀàËÆÓÚLinuxµÄerror±äÁ¿ÓÃ·¨*/
    private int opRetVal = -1;

    private WMMPListener gwmmpListener ;

    private byte[] sendTlvDataStack;
    private DataItemStack recvTlvDataStack;    //data stack,only save tlv data

    //
//    private CRC32 crc = new CRC32();

//    private Hashtable m2mCfgProperty = null;

    private Hashtable wmmpStackTable;

    public wmmp_protocol(String ip,int port)
    {
        log("consc wmmp_protocol");
        //default
        link_protocol = WMMPConnection.WMMP_BEARER_TCP;
        link_ip = ip;
        link_port = port;    	
        tcpOutStream = null;
        tcpInStream  = null;

//        asServeConnect  = null;
//        asClientConnect = null;

        lastMsgRecord   = null;

        wmmpStackTable = loadWmmpStackConfig();

//        crc.reset();

       // m2mCfgProperty = loadM2MCfgProperty();

        //Ä¬ÈÏÖ§³Ö³¤Á¬½Ó
        isLongConnect = true;
        setCurLinkState(STATE_NOTLINK);

        link_timestamp = 0;

        islogin = false;

        WMMPRMS.loadRmsData();
    }
    /*
    static WMMPConnection gWmmpConn = null;
    public static WMMPConnection getInstance()
    {
        if(gWmmpConn == null)
        {
            try {
                gWmmpConn = (WMMPConnection) Connector.open("wmmp://218.206.176.102:8763;bearer_type=tcp");
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }

        return gWmmpConn;
    }*/

    private boolean isSupportedTag(int tag)
    {
        //check simply now!
        if(tag <= 0)
            return false;
        return true;
    }

    private Hashtable loadWmmpStackConfig()
    {
        Hashtable hashCfgTab = new Hashtable();

        /*
        hashProperty.put(new Integer(tag),new WMMPDataItem(xx));
        */
        return hashCfgTab;
    }

    /*¼ÓÔØm2mÖÕ¶Ë¼°Æ½Ì¨Ò»Ð©Ä¬ÈÏÉèÖÃÊôÐÔ*/
    //Key is TAG str(as "TAG_0x0001",x is lower case)
/*
    private Hashtable loadM2MCfgProperty()
    {
        Hashtable hashProperty = new Hashtable();
        Vector    iVec ;//

        hashProperty.put("TAG_0x0001",new String("1111111"));
        hashProperty.put("TAG_0x0002",new String("1111111"));
        hashProperty.put("TAG_0x0003",new String("1111111"));
        hashProperty.put("TAG_0x0004",new String("1111111"));
        hashProperty.put("TAG_0x0005",new String("1111111"));
        hashProperty.put("TAG_0x0006",new String("1111111"));
        hashProperty.put("TAG_0x0007",new String("1111111"));
        hashProperty.put("TAG_0x0008",new String("1111111"));
        hashProperty.put("TAG_0x0009",new Integer(0xffffffff));
        hashProperty.put("TAG_0x000a",new Integer(0xaabbbbaa));
        hashProperty.put("TAG_0x000b",new String("1111111"));
        hashProperty.put("TAG_0x000c",new String("1111111"));
        hashProperty.put("TAG_0x000d",new Integer(0xffffffff));
        hashProperty.put("TAG_0x000e",new Integer(0xaabbbbaa));

        hashProperty.put("TAG_0x0010",new Integer(0xffffffff));
        hashProperty.put("TAG_0x0011",new Integer(heart_beat_interval)); //heart beat interval*
        hashProperty.put("TAG_0x0012",new String("1111111"));
        hashProperty.put("TAG_0x0013",new String("1111111"));
        hashProperty.put("TAG_0x0014",new String("1111111"));

        //0025´æ·Å¶à¸öºËÐÄÅäÖÃTag£¬Ê¹ÓÃvector
        ivec = new Vector();
        ivec.addElement(new Integer(1));
        hashProperty.put("TAG_0x0025",ivec);   //import
        hashProperty.put("TAG_0x0026",new String("1111111"));
        hashProperty.put("TAG_0x0027",new String("1111111"));
        hashProperty.put("TAG_0x0028",new String("1111111"));
        hashProperty.put("TAG_0x0029",new String("1111111"));
        hashProperty.put("TAG_0x002a",new String("1111111"));
        hashProperty.put("TAG_0x002b",new String("1111111"));
        hashProperty.put("TAG_0x002c",new String("1111111"));
        hashProperty.put("TAG_0x002d",new String("1111111"));
        hashProperty.put("TAG_0x002e",new String("1111111"));

        return hashProperty;
    }

    private synchronized boolean setM2MCfgProperty(Object key,Object val)
    {
        if(!m2mCfgProperty.containsKey(key))
        {
            return false;
        }

        m2mCfgProperty.put(key,val);
        return true;
    }

    private synchronized Object getM2MCfgProperty(Object key)
    {
        return m2mCfgProperty.get(key);
    }
*/

    /* format - wmmp://ip:port;bearer_type=tcp
     * eg: wmmp://192.168.1.1:9999;bearer_type=tcp
     */
    private String[] splitWmmpPath(String wmmp_name) throws IOException
    {
        String name = wmmp_name;
        String app_ip    = null;
        String app_port  = null;
        String protocol  = null;

        log("in wmmp_protocol.java,split: " + wmmp_name);
        // Must starts with "//" and not contain path separator
/*for J2SE
        if (!name.startsWith("//"))  //'wmmp' is deleted in upper callback
            throw new IllegalArgumentException(DEBUG ? "URI must start with 'wmmp://' " + name : null);

            name = name.toLowerCase();
            name = name.substring(2);   //'//' is deleted
*/
/*for GCF*/
        if (!name.startsWith("wmmp://"))  /*'wmmp' is deleted in upper callback */
            throw new IllegalArgumentException(DEBUG ? "URI must start with 'wmmp://' " + name : null);
        name = name.toLowerCase();
        name = name.substring(7);   /*'//' is deleted*/

        int idx = name.indexOf(';');  //find bearertype

        if(idx <= 0)
            throw new IllegalArgumentException(DEBUG ? "Contains invalid path" : null);

        app_ip = name.substring(0,name.indexOf(':'));  /*left is: 'ip' */

        app_port = name.substring(name.indexOf(':')+1,name.indexOf(';'));  /*left is: 'port' */

        idx = name.indexOf(';');
        name = name.substring(idx +1);

        if (!name.startsWith(WMMP_BEARER_KEY))
            throw new IllegalArgumentException(DEBUG ? "URI must contains 'bearer_type=' " + name : null);

        idx = name.indexOf('=');
        if(idx <= 0)
            throw new IllegalArgumentException(DEBUG ? "Contains invalid path" : null);

        protocol = name.substring(idx +1);

        log("app_ip - " + app_ip + ",app_port - " + app_port + ",protocol - " + protocol);
        if(app_ip ==null || app_port == null || protocol ==null)
        {
            log("app_ip/port or protocol is null");
            throw new IllegalArgumentException(DEBUG ? "Contains invalid path" : null);
        }

        if(!protocol.equals(WMMP_BEARER_TCP) &&
           !protocol.equals(WMMP_BEARER_UDP) &&
           !protocol.equals(WMMP_BEARER_SMS) )
        {
            throw new IllegalArgumentException(DEBUG ? "Contains invalid bearer type" : null);
        }

        return new String[] {app_ip,app_port,protocol};
    }

    public void open() throws WMMPException
    {
        /*
        log("name - " + name +",mode - " + mode + ",timeout - " + timeouts);
        name = StringOps.decode(name);

        String split[] = {"","",""};

        try
        {
            split = splitWmmpPath(name);
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }

        //??ip ?? ?Э?(TCP/UDP)
        wmmp_appip      = split[0];
        wmmp_appport    = split[1];
        wmmp_bearertype = split[2];
        */

        //link_protocol = wmmp_bearertype;
        //link_ip       = wmmp_appip;
        //link_port     = Integer.parseInt(wmmp_appport);

        log("link protocol: " + link_protocol + ",ip: " + link_ip + ",port: " + link_port);

        //ˇ???·
        try
        {
            doNetConnect();
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }

        startNetService(link_protocol);   //????П?

        cur_waitack_timeout = 0;
        continus_fail_times = 0;
    }
    
    public InputStream getInputStream() throws IOException
    {
    	return tcpInStream;
    }

    public OutputStream getOutputStream() throws IOException
    {
    	return tcpOutStream;
    }

    
    /*??·l?ìTCP??UDP*/
    protected synchronized void doNetConnect() throws IOException
    {
/* J2ME  GCF */
        if(link_protocol.equals(WMMPConnection.WMMP_BEARER_TCP))
        {
            String scheme = "socket://" + link_ip + ":" + link_port;
            log("scheme: " + scheme);
            if(sockConnection ==null)
            {
                //sockConnection = (SocketConnection)javax.microedition.io.Connector.open(scheme);
                log("link_ip="+link_ip+"link_port="+link_port);
            	sockConnection = new Socket(link_ip,link_port);
            }

            try
            {
                tcpInStream = sockConnection.getInputStream();
                log("get tcpInStream - " + tcpInStream);
            }
            catch(Exception e)
            {
                e.printStackTrace();
            }

            try
            {
                tcpOutStream = sockConnection.getOutputStream();
            }
            catch(Exception e)
            {
                e.printStackTrace();
            }

        }
        else
        {
            //udp
            String cScheme = "datagram://" + link_ip + ":" + link_port;
            String sScheme = "datagram://" + ":" + link_port;

            //asServeConnect = (UDPDatagramConnection)Connector.open(sScheme);
            //asClientConnect= (UDPDatagramConnection)Connector.open(cScheme);
        }
/**/

/*J2SE
        if(link_protocol.equals(WMMPConnection.WMMP_BEARER_TCP))
        {
            String scheme = "socket://" + link_ip + ":" + link_port;
            log("scheme: " + scheme);

            socket   = new Socket();
            sAddress = new InetSocketAddress(link_ip,link_port);


            try{
                socket.connect(sAddress);  //timeout 20s
            }
            catch(Exception e)
            {
                log("connect to sever error!");
                e.printStackTrace();
            }

            if(!socket.isConnected())
            {
                log("fail to connect - " + link_ip + ":" + link_port);
            }
            else
            {
                log("success to connect - " + link_ip + ":" + link_port);
            }

            try
            {
                tcpInStream = socket.getInputStream();
                log("get tcpInStream - " + tcpInStream);
            }
            catch(Exception e)
            {
                e.printStackTrace();
            }

            try
            {
                tcpOutStream = socket.getOutputStream();
            }
            catch(Exception e)
            {
                e.printStackTrace();
            }
        }
        else
        {}
*/
        log("success to create net link!");
        connectionOpen = true;
    }

    protected synchronized void doNetDisconnect()
    {
        log("doNetDisconnect()");
        connectionOpen = false;

        if(link_protocol.equals(WMMPConnection.WMMP_BEARER_TCP))
        {

/* for GCF */
            if (sockConnection == null)
            {
                return;
            }

            try
            {
                log("close all net connection ");
                tcpOutStream.close();
                tcpInStream.close();
                sockConnection.close();
            }
            catch(IOException e)
            {
                e.printStackTrace();
            }

            tcpOutStream = null;
            tcpInStream  = null;
            sockConnection = null;
/**/

/*for J2SE

            //close socket
            try
            {
                socket.close();
            }
            catch(Exception e)
            {
                e.printStackTrace();
            }
*/
        }
        else  //UDP
        {
/*
            try
            {
                asServeConnect.close();
            }
            catch(IOException e)
            {
                e.printStackTrace();
            }

            try
            {
                asClientConnect.close();
            }
            catch(IOException e)
            {
                e.printStackTrace();
            }
*/
        }
    }
    /**
     * Check if the connection is open.
     *
     * @exception  IOException  is thrown, if the stream is not open.
     */
    void ensureOpen() throws WMMPException
    {
        if (!connectionOpen)
        {
            throw new WMMPException("Connection closed");
        }
    }
    /**
     * Check if the connection is open.
     *
     * @exception  IOException  is thrown, if the stream is not open.
     */
    void ensureLogin() throws WMMPException
    {
        if (!islogin)
        {
            throw new WMMPException("WMMP not login in");
        }
    }

    /*flow id*/
    private static int getFlowId(boolean add)
    {
        if(add ==true)
        {
            link_flow_id++;
        }

        return link_flow_id;
    }

    private synchronized int getOpResult()
    {
        return opRetVal;
    }

    private synchronized void setOpResult(int val)
    {
        opRetVal = val;
    }

    private void waitResponse()
    {
        synchronized (mutex)
        {
            try
            {
                mutex.wait();
            }
            catch(Exception f)
            {
                log("waitResponse exception");
                f.printStackTrace();
            }
        }
    }

    /**
     * Called from EventConsumer, to ask for a refresh pulse.
     */
    private void wakeupWait()
    {
        log("call wakeupWait()");
        synchronized(mutex)
        {
            mutex.notify();
        }
        log("wakeupWait waked up");
    }

    /*current system time escape from 1970-1-1 00:00:00,unit in second*/
    private synchronized int getCurTimeSec()
    {
        int ret = 0;

        /* J2ME cannot call getTimeInMillis from calendar instance.
         * It's absolutely a fucking design!!!!!!!!!!!!!!!!!!!!!!!!!!
         *
         * Consider to use System.currentTimeMillis()?
         */

/* for JCF */
        ret = (int)(System.currentTimeMillis()/1000);
/**/

/*for J2SE
        Calendar cal = Calendar.getInstance();
        Date curDate = new Date();
        cal.setTime(curDate);
        ret = (int)(cal.getTimeInMillis()/1000);//get(Calendar.SECOND);
*/
        //log("cur time: " + ret);
        return ret;
    }

    /*get key valid date,return seconds from 1970 to the valid date*/
    private synchronized int getKeyValidDateInSec()
    {
        /*for J2SE*
        return new Date(2012,12,31).getSeconds();
        */

        //just fake a value for valid date seconds.

        return (int)(System.currentTimeMillis()/1000 - 0xFFFF);
    }

    private synchronized void updateLastConnectTime()
    {
        lastDataConnectTime = getCurTimeSec();
    }

    private synchronized int getLastConnectTime()
    {
        return lastDataConnectTime;
    }

    private synchronized void setCurLinkState(int newState)
    {
        log("link state,old: " + curLinkState + ",new:" + newState);
        curLinkState = newState;
    }

    private synchronized int getCurLinkState()
    {
        return curLinkState;
    }

    private synchronized void setContinusFailTimes(int newtimes)
    {
        continus_fail_times = newtimes;
        log("continus fail times: " + continus_fail_times);
    }

    private synchronized int getContinusFailTimes()
    {
        return continus_fail_times;
    }

    private synchronized void sendMsg(byte[] buff,int len) // throws Exception
    {
        int i=0;
        setCurLinkState(STATE_SENDING);
        lastMsgRecord = new byte[len];
        for(i=0;i<len;i++)
        {
            lastMsgRecord[i] = buff[i];
        }
        log("len ---:" + buff.length);
        log("---------------------------------------------");
        logByteArr(buff);
        log("---------------------------------------------");
        if(link_protocol.equals(WMMPConnection.WMMP_BEARER_TCP))
        {
            try
            {
                tcpOutStream.write(buff,0,len);
                log("send tcp data ok!");
            }
            catch(Exception e)
            {
                setCurLinkState(STATE_SENDFAIL);
                e.printStackTrace();
            }
        } //udp
        else
        {
/*
            Datagram dgram = asClientConnect.newDatagram(buff,len) ;

            try
            {
                asClientConnect.send(dgram);
            }
            catch(Exception e)
            {
                setCurLinkState(STATE_SENDFAIL);
                log("send datagram fail!");
                e.printStackTrace();
            }
*/
        }
        updateLastConnectTime();
        setCurLinkState(STATE_WAITACK);
    }

    private void sendLastMsgAgain()
    {
        log("send last msg again!");
        if(lastMsgRecord !=null)
        {
            sendMsg(lastMsgRecord,lastMsgRecord.length);
        }
    }

    /*½âÎöÊÕµ½µÄÊý¾Ý*/
    private synchronized void praseRecvData(byte[] rcvdata,int len) throws IOException
    {
        ByteArrayInputStream bArrIs = new ByteArrayInputStream(rcvdata,0,len);
        DataInputStream dataIs      = new DataInputStream(bArrIs);

        int totalLen = 0;
        int cmdId  = 0;
        int sequenceId = 0;
        char protocolVersion = 0;
        byte safeTag = 0;
        byte keepTag = 0;
        byte[] serialNumber = new byte[16];
        int filter = 0xffff;

        updateLastConnectTime();
        log("call praseRecvData!");
        //read the datagram head info
        totalLen = dataIs.readChar(); // read total length of the message
        log("========totalLen1="+totalLen);
        totalLen = totalLen&filter;
        log("========totalLen2="+totalLen);
        cmdId  = (int)dataIs.readChar(); //read current command ID
        sequenceId = dataIs.readInt(); //read current flow ID
        protocolVersion = dataIs.readChar(); //read protocol version
        safeTag = dataIs.readByte();// read safe tag value
        keepTag = dataIs.readByte(); // keep value , no sense here, only to read it out
        dataIs.read(serialNumber,0,16); // read serial Number to buffer


        log("prase gram,cmd id: " + cmdId + "totalLen :" + totalLen);

        switch(cmdId)
        {
            case WMMPConnection.REGISTER_ACK:
            {
                byte regRet = 0;
                byte[] seqBytes = new byte[16];

                regRet = dataIs.readByte();
                dataIs.readFully(seqBytes,0,16);
                log("REGISTER_ACK ret val: " + regRet);
                setOpResult(regRet);
                setCurLinkState(STATE_LINKING);
                setContinusFailTimes(0);
                wakeupWait();
            }
            break;

            case WMMPConnection.LOGIN_ACK:
            {
                byte regRet = 0;
                int  timestamp =0;

                regRet = dataIs.readByte();
                log("LOGIN_ACK ret val: " + regRet);
                timestamp = dataIs.readInt();
                log("LOGIN_ACK timestamp is: " + timestamp);
                if(regRet ==0x00 || regRet ==0x01)
                {
                    link_timestamp = timestamp;
                }
                setOpResult(regRet);
                setCurLinkState(STATE_LINKING);
                setContinusFailTimes(0);
                wakeupWait();
            }
            break;

            case WMMPConnection.LOGOUT_ACK:
            {
                byte regRet = 0;
                log("Recv Logout Ack");
                setOpResult(regRet);
                setCurLinkState(STATE_LINKING);
                setContinusFailTimes(0);
                wakeupWait();
            }
            break;

            case WMMPConnection.CONFIG_TRAP_ACK:
            {
                byte regRet = 0;

                regRet = dataIs.readByte();
                log("CONFIG_TRAP_ACK ret val: " + regRet);
                if(totalLen -29 > 0)
                {
                    byte[] arr = new byte[totalLen -29];
                    dataIs.read(arr);
                    recvTlvDataStack = new DataItemStack(WMMPConnection.CONFIG_TRAP_ACK,arr);
                }
                setOpResult(regRet);
                setCurLinkState(STATE_LINKING);
                setContinusFailTimes(0);
                wakeupWait();
            }
            break;

            case WMMPConnection.CONFIG_REQ_ACK:
            {
                byte regRet = 0;

                regRet = dataIs.readByte();
                log("CONFIG_REQ_ACK ret val: " + regRet);
                if(totalLen -29 > 0)
                {
                    byte[] arr = new byte[totalLen -29];
                    dataIs.read(arr);
                    recvTlvDataStack = new DataItemStack(WMMPConnection.CONFIG_REQ_ACK,arr);
                }
                setOpResult(regRet);
                setCurLinkState(STATE_LINKING);
                setContinusFailTimes(0);
                wakeupWait();
            }
            break;

            case WMMPConnection.SECURITY_CONFIG:
            {
                log("got SECURITY_CONFIG msg!");
            }
            break;

            case WMMPConnection.HEART_BEAT_ACK:
            {
                log("Heart beat ack!");
                setCurLinkState(STATE_LINKING);
                setContinusFailTimes(0);
                //not need to wakeup
            }
            break;

            case WMMPConnection.REMOTE_CTRL:
            {
                log("REMOTE_CTRL cmd recevied!");
                int result = 0;
                int i = 0;
                int tlvlen = totalLen-28;
                WMMPDataItem[] dstack =null;
                
                if(totalLen -28 > 0)
                {
                    byte[] arr = new byte[totalLen -28];
                    dataIs.read(arr);
                    recvTlvDataStack = new DataItemStack(WMMPConnection.REMOTE_CTRL,arr);
                    dstack = recvTlvDataStack.getAllDataItem();
                }
                
                sendRemoteControlACK(result,null,0,false,sequenceId);
                
                setCurLinkState(STATE_LINKING);
                setContinusFailTimes(0);
                if(dstack !=null)
                {
                    handleWMMPListener(WMMPConnection.REMOTE_CTRL,dstack);
                }
            }
            break;

            case WMMPConnection.TRANSPARENT_DATA:
            {
                log("TRANSPARENT_DATA cmd recevied!");
                int result = 0;
                int i = 0;
                int tlvlen = totalLen-28;
                WMMPDataItem[] dstack =null;

                int crc = dataIs.readChar();
                if(totalLen - 30 > 0)
                {
                    byte[] arr = new byte[totalLen - 30];
                    dataIs.read(arr);
                    recvTlvDataStack = new DataItemStack(WMMPConnection.TRANSPARENT_DATA,arr);
                    dstack = recvTlvDataStack.getAllDataItem();
                }
                /*
                byte[] tlvs = new byte[tlvlen];
                for(i=0;i<tlvlen;i++)
                {
                    tlvs[i] = rcvdata[i+28];
                }

                result = handleTransparentData(tlvs,totalLen-28);
                */
                log("REMOTE_CTRL handleTransparentDataCMD result = " + result);
                setCurLinkState(STATE_LINKING);
                setContinusFailTimes(0);
                if(dstack !=null)
                {
                    handleWMMPListener(WMMPConnection.TRANSPARENT_DATA,dstack);
                }

            }
            break;

            case WMMPConnection.TRANSPARENT_DATA_ACK:
            {
                log("TRANSPARENT_DATA_ACK cmd recevied!");
                byte regRet = 0;

                regRet = dataIs.readByte();
                log("TRANSPARENT_DATA_ACK ret val: " + regRet);
                setOpResult(regRet);
                setCurLinkState(STATE_LINKING);
                setContinusFailTimes(0);
                wakeupWait();
            }
            break;

            case WMMPConnection.TRANSPARENT_CMD:
            {
                log("TRANSPARENT_CMD cmd recevied!");
            }
            break;

            case WMMPConnection.TRANSPARENT_CMD_ACK:
            {
                log("TRANSPARENT_CMD_ACK cmd recevied!");
            }
            break;

            case WMMPConnection.URL_INFO:
            {
                log("====DOWNLOAD_INFO cmd recevied!=============");
                if(totalLen - 28 > 0)
                {
                    byte[] arr = new byte[totalLen - 28];
                    dataIs.read(arr);
                    recvTlvDataStack = new DataItemStack(WMMPConnection.URL_INFO,arr);
                    logByteArr(arr);
                    WMMPDataItem ditem = recvTlvDataStack.getDataItem(0x1001);
                    if(ditem ==null)
                    {
                        log("URL:prase fail!");
                        sendDownloadInfoAck(0x04);
                    }
                    else
                    {
                        String url = new String(ditem.getRawData());
                        log("URL:"+url);

                        WMMPFileDownloadTask dlTask;
                        if(WMMPConnection.ARM_VERSION)
                        {
                            //for arm
                            dlTask = new WMMPFileDownloadTask(this,gwmmpListener,url,"Phone/");
                        }
                        else
                        {
                            //for win
                            dlTask = new WMMPFileDownloadTask(this,gwmmpListener,url,"root1/");
                        }
                        //sendDownloadInfoAck(0x01);
                        dlTask.start();
                        log("Started download!");
                    }
                }
                setCurLinkState(STATE_LINKING);
                setContinusFailTimes(0);
            }
            break;

            default:
            {
                wakeupWait();
                log("Cmd :"+ cmdId +" not to prase for now!!");
            }
            break;
        }

    }

    private void ReconnectWhenReadErr()
    {
        if(!recv_work_exit){
            log("Read error,Re-Connect to server");
            try {
                doNetDisconnect();
                Thread.sleep(1000);
                sockConnection = null;
                doNetConnect();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    /*×¨ÓÃÓÚ½ÓÊÕÏûÏ¢/²¢´¦ÀíÊÕµ½ÏûÏ¢µÄÏß³Ì*/
    private class TcpRecvWorker implements Runnable
    {
        private wmmp_protocol parent;

        TcpRecvWorker(wmmp_protocol parent)
        {
            this.parent = parent;
        }

        public void run()
        {
            int nbyte = 0;
            int read  = 0;
            int len   = 0;
            byte[] lenbuff = new byte[4];
            byte[] rcvbuff;
            if(this.parent == null)
            {
                wmmp_protocol.log("error: parent instance is null!");
                return;
            }

            if(this.parent.tcpInStream == null)
            {
                wmmp_protocol.log("error: parent tcpInStream instance is null!");
                return;
            }

            log("get parent tcpInStream - " + parent.tcpInStream);
            while(!parent.recv_work_exit)
            {
                len = 0;
                log("TcpRecvWorker recv_work_exit=" + recv_work_exit);
                try {
                    nbyte = parent.tcpInStream.read(lenbuff,0,4);
                    if(nbyte == -1)
                    {
                        log("TcpRecvWorker nbyte == -1 ");
                        if(!parent.recv_work_exit)
                        {
                            try{
                                Thread.sleep(500);  //500 ms
                            } catch(Exception e) {
                                e.printStackTrace();
                            }
                        }
                        continue;
                    }
                    log("nbyte = " + nbyte);
                } catch(Exception e) {
                    //handle exception here
                    log("tcp read exception 0");
                    e.printStackTrace();
                    if(!parent.recv_work_exit)
                    {
                        log("ReconnectWhenReadErr() ==1");
                        ReconnectWhenReadErr();
                    }
                    continue;
                }

                len = (lenbuff[0] << 8) + lenbuff[1];
                len = len&0xff;
                wmmp_protocol.log("read nbyte: " + nbyte + ",total len:" + len);

                /*workargound*/
                int cmdId = (lenbuff[2] << 8) + lenbuff[3];
                cmdId = cmdId&0xff;
                /*
                if(cmdId == WMMPConnection.LOGOUT_ACK)
                {
                    len =28;
                }
                */
                rcvbuff = new byte[len];

                rcvbuff[0] = lenbuff[0];
                rcvbuff[1] = lenbuff[1];
                rcvbuff[2] = lenbuff[2];
                rcvbuff[3] = lenbuff[3];

                if(len > nbyte)
                {
                    while(len > nbyte && !parent.recv_work_exit)
                    {
                        read = (len - nbyte) > TCP_READ_EACH ? TCP_READ_EACH : (len - nbyte);
                        try {
                            nbyte += parent.tcpInStream.read(rcvbuff,nbyte,read);
                            wmmp_protocol.log("nbyte:" + nbyte);
                        } catch(Exception e) {
                            //handle exception here
                            log("tcp read exception");
                            e.printStackTrace();
                            if(!parent.recv_work_exit)
                            {
                                log("ReconnectWhenReadErr() ==2");
                                ReconnectWhenReadErr();
                            }
                            continue;
                        }
                    }

                    if(parent.recv_work_exit) break;
                }

                /*prase data*/
                try {
                    parent.praseRecvData(rcvbuff,len);                    
                } catch(IOException e) {
                    e.printStackTrace();
                }
                rcvbuff = null;  //notify GC
            }
            parent.recv_work_exited = true;
        }
    }

    private class UdpRecvWorker implements Runnable
    {
        private wmmp_protocol parent;

        UdpRecvWorker(wmmp_protocol parent)
        {
            this.parent = parent;
        }

        public void run()
        {
        /*
            Datagram dgram = asServeConnect.newDatagram(UDP_MAX_LEN);
            byte[] tmpbuff;
            int len = 0;
            int declen =0;
            while(!parent.recv_work_exit)
            {
                try
                {
                    asServeConnect.receive(dgram);
                }
                catch(IOException e)
                {
                    e.printStackTrace();
                }

                if(parent.recv_work_exit) break;

                //check data simplely
                tmpbuff = dgram.getData();
                len = dgram.getLength();
                declen = (tmpbuff[0] << 8) + tmpbuff[1];
                if(len != declen)
                {
                    wmmp_protocol.log("Recv datagram error: length is incorrect!");
                    setOpResult(WMMPConnection.ERR_NET_EXCEPTION);
                    wakeupWait();
                }
                else
                {
                    try
                    {
                        parent.praseRecvData(tmpbuff,len);
                    }
                    catch(IOException e)
                    {
                        e.printStackTrace();
                    }
                }
                dgram.reset();
            }
            parent.recv_work_exited = true;
        */
        }
    }

    /*config trap timer task*/

    private class sendConfigTrapTask extends TimerTask
    {
        public void run()
        {
            byte[] tlvs;
            try{
            ByteArrayOutputStream bArrOs = new ByteArrayOutputStream(5);
            DataOutputStream dataOs = new DataOutputStream(bArrOs);
            log("send sendConfigTrapTask write:");

            dataOs.writeShort(0x300b);            //total length of gram
            dataOs.writeShort(1);
            dataOs.writeByte(0x01);
            /*
            dataOs.writeShort(0x3002);
            dataOs.writeShort(16);
            dataOs.write(WMMPRMS.getRmsVal(PARAM_TERMSEQSTR).getBytes(),0,WMMPRMS.getRmsVal(PARAM_TERMSEQSTR).length());

            dataOs.writeShort(0x3003);
            dataOs.writeShort(16);
            dataOs.write(WMMPRMS.getRmsVal(PARAM_IMSI).getBytes(),0,WMMPRMS.getRmsVal(PARAM_IMSI).length());
            //imsi length is 15, add a byte
            dataOs.writeByte(0);

            dataOs.writeShort(0x3004);
            dataOs.writeShort(8);
            dataOs.writeLong(1);
            */

            dataOs.flush();

            tlvs = bArrOs.toByteArray();
            log("send sendConfigTrapTask tlvs.length=" + tlvs.length);
            sendConfigTrap(tlvs);
            dataOs.close();
            bArrOs.close();
            }catch(Exception e)
            {
                log("sendConfigTrapTask exception" + e);
            }
        }
    }

    /*¿ªÆôÒ»¸öÐÄÌø·þÎñ×¨ÃÅÓÃÓÚ³¤Á¬½Ó¹ÜÀí*/
    private class HeartBeatWorker implements Runnable
    {
        private wmmp_protocol parent;
        private int curState = 0;
        private int cur_time_sec = 0;
        private int continue_fail_times = 0;

        HeartBeatWorker(wmmp_protocol parent)
        {
            this.parent = parent;
        }

        public void run()
        {
            boolean isSleep = false;
            while(!parent.hbeat_task_exit)
            {
                if(isSleep)
                {
                    try{
                        Thread.sleep(500);  //500 ms
                    } catch (Exception e)
                    {
                        e.printStackTrace();
                    }
                }
                if(parent.hbeat_task_exit)
                {
                    break;
                }
                isSleep = true;
                cur_time_sec = parent.getCurTimeSec();
                switch(parent.getCurLinkState())
                {
                    case STATE_LINKING:
                    {
                        if(!islogin)
                            break;
                        if(cur_time_sec >= parent.getLastConnectTime() + parent.getHeartBeatVal(WMMPConnection.HEART_BEAT_INTERVAL))
                        {
                            wmmp_protocol.log("send one heart beat!!");
                            /*send heart beat*/
                            try {
                                parent.sendHeartBeatMsg();
                            } catch(IOException e) {
                                e.printStackTrace();
                            }
                        }
                    }
                    break;

                    case STATE_SENDING:
                    {
                        /*nothing to do!*/
                    }
                    break;

                    case STATE_SENDFAIL:
                    {
                        parent.setContinusFailTimes(parent.getContinusFailTimes() +1);
                        if(parent.getContinusFailTimes() < parent.getHeartBeatVal(WMMPConnection.HEART_BEAT_TIMES))
                        {
                            parent.sendLastMsgAgain();
                        }
                        else
                        {
                            wmmp_protocol.log("fail times reach MAX!! ready to break connection");
                            parent.setCurLinkState(STATE_FAIL);
                        }
                        isSleep = false;
                    }
                    break;

                    case STATE_WAITACK:
                    {
                        if(cur_time_sec >= parent.getLastConnectTime() + parent.getHeartBeatVal(WMMPConnection.HEART_BEAT_TIMEOUT))
                        {
                            wmmp_protocol.log("time out! reSend!");
                            parent.setCurLinkState(STATE_SENDFAIL);
                            isSleep = false;
                        }
                        else
                        {
                            wmmp_protocol.log("is in waiting Ack state!");
                        }
                    }
                    break;

                    case STATE_FAIL:
                    {
                        /*how to fix? break connection or return cmd fail?*/
                        //handle to wake up wait,may be new problem!
                        parent.setOpResult(99);
                        parent.setCurLinkState(STATE_LINKING);
                        parent.setContinusFailTimes(0);
                        parent.wakeupWait();
                    }
                    break;

                    default:break;
                }
            }
            parent.hbeat_task_exited = true;
        }
    }

    private void startNetService(String protocol)
    {
        recv_work_exit  = false;
        hbeat_task_exit = false;
        recv_work_exited  = false;
        hbeat_task_exited = false;

        islogin = false;

        if(protocol.equals(WMMPConnection.WMMP_BEARER_TCP))
        {
            new Thread(new TcpRecvWorker(this)).start();
        }
        else
        {
            new Thread(new UdpRecvWorker(this)).start();
        }

        if(isLongConnect)
        {
            /*start heart beat service if long connection*/
            new Thread(new HeartBeatWorker(this)).start();
        }

        updateLastConnectTime();
        setCurLinkState(STATE_LINKING);
    }


    private String intToByteArrayString(int a)
    {
        byte[] tmp = new byte[4];

        tmp[0] = (byte)((a >>24 ) & 0xff);
        tmp[1] = (byte)((a >>16 ) & 0xff);
        tmp[2] = (byte)((a >> 8 ) & 0xff);
        tmp[3] = (byte)((a >> 0 ) & 0xff);

        return new String(tmp);
    }

    private byte[] intToByteArray(int a)
    {
        byte[] tmp = new byte[4];

        tmp[0] = (byte)((a >>24 ) & 0xff);
        tmp[1] = (byte)((a >>16 ) & 0xff);
        tmp[2] = (byte)((a >> 8 ) & 0xff);
        tmp[3] = (byte)((a >> 0 ) & 0xff);

        return tmp;
    }


    private byte[] getGramHead(int gram_len,int version,int gram_cmd,int flow_id,byte safeId) throws IOException
    {
        byte[] retBA;
        log("getGramHead ");
        ByteArrayOutputStream bArrOs = new ByteArrayOutputStream(28);
        DataOutputStream dataOs = new DataOutputStream(bArrOs);
        log("getGramHead write");
        dataOs.writeShort(gram_len);            //total length of gram
        dataOs.writeShort(gram_cmd);
        dataOs.writeInt(flow_id);
        dataOs.writeShort(version);
        dataOs.writeByte(safeId);
        dataOs.writeByte(0x00);
        dataOs.write(WMMPRMS.getRmsVal(PARAM_TERMSEQSTR).getBytes(),0,WMMPRMS.getRmsVal(PARAM_TERMSEQSTR).length());
        dataOs.flush();
        dataOs.close();

        log("Gramhead length is: " + bArrOs.size());

        retBA = bArrOs.toByteArray();
        bArrOs.close();
        return retBA;
    }

    private int getTlvLength(int tag)
    {
        int len = 0;

        switch(tag)
        {
            case 0:
            break;

            default:
            break;
        }

        return len;
    }

    private byte[] getTlvE021(byte[] gram,int timestamp,byte[] pwd) throws IOException
    {
        byte[] retBA;
        int len = gram.length + 4 + WMMPRMS.getRmsVal(PARAM_IMEI).length() + WMMPRMS.getRmsVal(PARAM_IMSI).length() + pwd.length ;
        ByteArrayOutputStream bArrOs = new ByteArrayOutputStream(len);
        DataOutputStream dataOs = new DataOutputStream(bArrOs);

        dataOs.write(gram,0,gram.length);
        dataOs.writeInt(timestamp);
        log("======WMMPRMS.getRmsVal(PARAM_IMEI)" + WMMPRMS.getRmsVal(PARAM_IMEI));
        log("======WMMPRMS.getRmsVal(PARAM_IMSI)" + WMMPRMS.getRmsVal(PARAM_IMSI));
        dataOs.write(WMMPRMS.getRmsVal(PARAM_IMEI).getBytes(),0,WMMPRMS.getRmsVal(PARAM_IMEI).length());
        dataOs.write(WMMPRMS.getRmsVal(PARAM_IMSI).getBytes(),0,WMMPRMS.getRmsVal(PARAM_IMSI).length());
        dataOs.write(pwd,0,pwd.length);

        dataOs.flush();
        dataOs.close();

        log("TLV E021 length is: " + bArrOs.size());

        retBA = bArrOs.toByteArray();
        bArrOs.close();

        /*get MD5*/
        retBA = DigitalEncrypt.getMD5ByteArray(retBA);

        bArrOs = new ByteArrayOutputStream(20);
        dataOs = new DataOutputStream(bArrOs);

        dataOs.writeByte(0xe0);/*T*/
        dataOs.writeByte(0x21);
        dataOs.writeByte(0x00);/*L*/
        dataOs.writeByte(0x10);
        dataOs.write(retBA,0,16);/*V*/

        dataOs.flush();
        dataOs.close();

        retBA = bArrOs.toByteArray();
        bArrOs.close();
        return retBA;
    }

    private byte[] getTlvE03A() throws IOException
    {
        /*E03A = MD5(E036 + E038)*/
        //now E036 is null,E038 is 0x00000000
        byte[] retBA = {0x00,0x00,0x00,0x00};;
        ByteArrayOutputStream bArrOs = new ByteArrayOutputStream(20);
        DataOutputStream dataOs = new DataOutputStream(bArrOs);

        /*get MD5*/
        retBA = DigitalEncrypt.getMD5ByteArray(retBA);

        dataOs.writeByte(0xe0);/*T*/
        dataOs.writeByte(0x3a);
        dataOs.writeByte(0x00);/*L*/
        dataOs.writeByte(0x10);
        dataOs.write(retBA,0,16);/*V*/

        dataOs.flush();
        dataOs.close();

        retBA = bArrOs.toByteArray();
        bArrOs.close();
        return retBA;
    }




    /*---------------------------------public API of WMMP-----------------------------------*/
/*
    public Connection openPrim(String name, int mode, boolean timeouts) throws IOException
    {
        log("name - " + name +",mode - " + mode + ",timeout - " + timeouts);
        name = StringOps.decode(name);

        String split[] = {"","",""};

        try
        {
            split = splitWmmpPath(name);
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }

        //??ip ?? ?Э?(TCP/UDP)
        wmmp_appip      = split[0];
        wmmp_appport    = split[1];
        wmmp_bearertype = split[2];

        link_protocol = wmmp_bearertype;
        link_ip       = wmmp_appip;
        link_port     = Integer.parseInt(wmmp_appport);

        log("link protocol: " + link_protocol + ",ip: " + link_ip + ",port: " + link_port);

        //ˇ???·
        try
        {
            doNetConnect();
        }
        catch(IOException e)
        {
            e.printStackTrace();
            return null;
        }

        startNetService(link_protocol);   //????П?

        cur_waitack_timeout = 0;
        continus_fail_times = 0;

        return this;
    }
*/
    private synchronized void waitExit()
    {
        if(link_protocol.equals(WMMPConnection.WMMP_BEARER_TCP))
        {
            while(!recv_work_exited)
            {
                log("sleep 1000 to wait recvworker stop");
                try{
                    Thread.sleep(1000);
                } catch(Exception e) {
                    e.printStackTrace();
                }
            }
/**/

/* for J2SE
            while(!recv_work_exited || !socket.isClosed())
            {
                try{
                    Thread.sleep(10);
                }
                catch(Exception e)
                {
                    e.printStackTrace();
                }
            }
*/
        }
        else
        {

        }

        if(isLongConnect)
        {
            while(!hbeat_task_exited)
            {
                log("sleep 1000 to wait heartbeat stop");
                try{
                    Thread.sleep(1000);
                } catch(Exception e) {
                    e.printStackTrace();
                }
            }
        }
        log("all wmmp thread exited!");
    }

    public void close() throws WMMPException
    {
        ensureOpen();
        recv_work_exit  = true;
        hbeat_task_exit = true;
        log("to wait wmmp exit!");
        waitExit();
        log("Wmmp connection closed!");
        doNetDisconnect();
    }

    public int getOption(byte option)
    {
        int ret;
        log("get option is :" + option);

        switch(option)
        {
            case WMMPConnection.FRAGMENT:
            {
                ret = fragment_val;
            }
            break;

            case WMMPConnection.KEEPALIVE:
            {
                ret = keepalive_val;
            }
            break;

            case WMMPConnection.LINGER:
            {
                ret = linger_val;
            }
            break;

            default:
            {
                ret = -1;
            }
            break;
        }

        return ret;
    }

    public void setOption(byte option,int value)
    {
        log("set option is :" + option);

        switch(option)
        {
            case WMMPConnection.FRAGMENT:
            {
                fragment_val = value;
            }
            break;

            case WMMPConnection.KEEPALIVE:
            {
                keepalive_val = value;
            }
            break;

            case WMMPConnection.LINGER:
            {
                linger_val = value;
            }
            break;

            default:
            {
                log("set opt,nothing to do");
            }
            break;
        }
    }



    /*¿¼ÂÇÊ¹ÓÃvectorÈÝÆ÷*/
    public void setHeartBeatVal(String key,int value) throws IOException
    {
        /*ÊÇ·ñ¿¼ÂÇÔÚÄÄÐ©ÇéÐÎÏÂ²ÅÄÜÉèÖÃ?*/

        if(value <0)
        {
            throw new IllegalArgumentException(DEBUG ? "illegal value:" + value : null);
        }

        if(key.equals(WMMPConnection.HEART_BEAT_INTERVAL))
        {
            heart_beat_interval = value;
        }
        else if(key.equals(WMMPConnection.HEART_BEAT_TIMEOUT))
        {
            heart_beat_timeout = value;
        }
        else if(key.equals(WMMPConnection.HEART_BEAT_TIMES))
        {
            heart_beat_times = value;
        }
        else
        {
            throw new IllegalArgumentException(DEBUG ? "illegal key:" + key : null);
        }
    }

    public int getHeartBeatVal(String key)
    {
        if(key.equals(WMMPConnection.HEART_BEAT_INTERVAL))
        {
            return heart_beat_interval;
        }
        else if(key.equals(WMMPConnection.HEART_BEAT_TIMEOUT))
        {
            return heart_beat_timeout;
        }
        else if(key.equals(WMMPConnection.HEART_BEAT_TIMES))
        {
            return heart_beat_times ;
        }
        else
        {
            return -1;
        }
    }

    public int getOpCode()
    {
        return getOpResult();
    }

    //¼ì²éÊÇ·ñ×¢²á
    public boolean checkIfRegister()
    {
        String reg = WMMPRMS.getRmsVal(WMMPConnection.PARAM_REGISTERSTATE);
        if(reg.equals("false"))
        {
            log("Terminal has not registered!");
            return false;
        }
        else
        {
            log("Terminal has registered!");
            return true;
        }
    }

    public int register(byte type) throws IOException
    {
        byte[] gramHead;
        String tmpStr;
        ByteArrayOutputStream bArrOs = null;
        DataOutputStream dataOs = null;
        int valid_key_sec = 0;
        if((type >REG_TYPE_4) || (type < REG_TYPE_0))
        {
            throw new IllegalArgumentException();
        }

        ensureOpen();

        switch(type)
        {
            case WMMPConnection.REG_TYPE_0:
            case WMMPConnection.REG_TYPE_1:  //0/1 difference: seqString
            {
                /*length: head 28 + 1+16+15 = 60*/
                bArrOs = new ByteArrayOutputStream(60);
                dataOs = new DataOutputStream(bArrOs);

                /*write gram head*/
                gramHead = getGramHead(60,WMMPConnection.WMMP_VERSION,WMMPConnection.REGISTER,getFlowId(true),(byte)0x00);
                dataOs.write(gramHead,0,gramHead.length);

                /*op type*/
                dataOs.writeByte(type/*WMMPConnection.REG_TYPE_0*/);

                /*write IMEI*/
                dataOs.write(WMMPRMS.getRmsVal(PARAM_IMEI).getBytes(),0,WMMPRMS.getRmsVal(PARAM_IMEI).length());

                if(WMMPRMS.getRmsVal(PARAM_IMEI).length() == 15)
                {
                    dataOs.writeByte(0x00);
                }

                /*write IMSI*/
                dataOs.write(WMMPRMS.getRmsVal(PARAM_IMSI).getBytes(),0,WMMPRMS.getRmsVal(PARAM_IMSI).length());
            }
            break;

            case WMMPConnection.REG_TYPE_2:
            {}
            break;

            case WMMPConnection.REG_TYPE_3:
            {
                /*0xE026,0xE029,0xE03A size: 1+20+20+20+16+15 =92 + head 28 = 120*/
                bArrOs = new ByteArrayOutputStream(120);
                dataOs = new DataOutputStream(bArrOs);

                /*write gram head*/
                gramHead = getGramHead(120,WMMPConnection.WMMP_VERSION,WMMPConnection.REGISTER,getFlowId(true),(byte)0x00);
                dataOs.write(gramHead,0,gramHead.length);

                /*op type*/
                dataOs.writeByte(WMMPConnection.REG_TYPE_3);

                valid_key_sec = getKeyValidDateInSec();                            /*ÃÜÔ¿ÓÐÐ§ÆÚ£¬Õâ¸öÖµ¿ÉÄÜÓÐÎÊÌâ!!!!!!!!!!*/
                /*write 0xE026*/
                tmpStr = new String(WMMPRMS.getRmsVal(PARAM_UPLINKPWD));
                tmpStr.concat(intToByteArrayString(valid_key_sec));
                byte[] buff1 = DigitalEncrypt.getMD5ByteArray(tmpStr.getBytes());
                log("0xE026 of md5: " + tmpStr + "len: " + tmpStr.length());
                dataOs.write(buff1,0,buff1.length);

                /*write 0xE029*/
                tmpStr = new String(WMMPRMS.getRmsVal(PARAM_DOWNLINKPWD));
                tmpStr.concat(intToByteArrayString(valid_key_sec));
                buff1 = DigitalEncrypt.getMD5ByteArray(tmpStr.getBytes());
                log("0xE029 of md5: " + tmpStr + "len: " + tmpStr.length());
                dataOs.write(buff1,0,buff1.length);

                /*write 0xE03A*/
                //Md5(0xE036+0xE038)
                tmpStr = new String(BaseCipherKey);
                tmpStr.concat(intToByteArrayString(valid_key_sec));
                buff1 = DigitalEncrypt.getMD5ByteArray(tmpStr.getBytes());
                log("0xE03A of md5: " + tmpStr + "len: " + tmpStr.length());
                dataOs.write(buff1,0,buff1.length);

                /*write IMEI*/
                dataOs.write(WMMPRMS.getRmsVal(PARAM_IMEI).getBytes(),0,WMMPRMS.getRmsVal(PARAM_IMEI).length());
                if(WMMPRMS.getRmsVal(PARAM_IMEI).length() == 15)
                {
                    dataOs.writeByte(0x00);
                }

                /*write IMSI*/
                dataOs.write(WMMPRMS.getRmsVal(PARAM_IMSI).getBytes(),0,WMMPRMS.getRmsVal(PARAM_IMSI).length());
            }
            break;

            case WMMPConnection.REG_TYPE_4:
            {}
            break;

            default:break;
        }

        if(bArrOs == null || dataOs ==null)
        {
            throw new IOException();
        }

        sendMsg(bArrOs.toByteArray(),bArrOs.size());

        try
        {
            bArrOs.close();
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }

        try
        {
            dataOs.close();
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }

        waitResponse();

        log("register result :" + getOpResult());

        /*Version 3.1:treat 0x6 as registered one*/
        if(getOpResult() == 0x06)
        {
            log("terminal has registered!");
            WMMPRMS.setRmsVal(PARAM_REGISTERSTATE,"true");
        }

        return getOpResult();
    }

    public int login(char varied[],boolean encrypt_session,boolean clrVars) throws IOException
    {
        byte[] buff;
        ByteArrayOutputStream bArrOs = null;
        DataOutputStream dataOs = null;
        int gramlen =0;
        byte safeId =0;

        ensureOpen();

        /*first step: check register*/

        /*ÏÈ²»Òª¿É±ä²¿·Ö,²»´øE03AºÍE020£¬Ö»´øE021*/
        /*length: 28+8+4+20 = 60*/
        gramlen = 60;
        if(encrypt_session) /*E03A*/
        {
            gramlen += 20;
        }
        if(clrVars) /*E020*/
        {
            gramlen += 20;
        }
        bArrOs = new ByteArrayOutputStream(gramlen);
        dataOs = new DataOutputStream(bArrOs);

        /*write gram head*/
        buff = getGramHead(gramlen,WMMPConnection.WMMP_VERSION,WMMPConnection.LOGIN,getFlowId(true),(byte)0x00);
        dataOs.write(buff,0,buff.length);

        /*term software version,8 byte*/
        dataOs.write(WMMPRMS.getRmsVal(PARAM_TERMSOFTVER).getBytes(),0,WMMPRMS.getRmsVal(PARAM_TERMSOFTVER).length());

        /*CRC32(E025),4 byte,ÔÝÊ±Ð´0*/
        dataOs.writeInt(0x00);
        dataOs.flush();

        if(encrypt_session)
        {
            buff = getTlvE03A();
            dataOs.write(buff,0,buff.length);
            dataOs.flush();
        }

        /*get E021,MD5(gram + 0x00000000,)*/
        buff = bArrOs.toByteArray();
        gramlen = gramlen -20;
        log("in login,hope len is " + gramlen + ",in fact len is:" + buff.length);
        buff = getTlvE021(buff,0x00000000,WMMPRMS.getRmsVal(PARAM_UPLINKPWD).getBytes());
        dataOs.write(buff,0,buff.length);
        dataOs.flush();


        /*send msg out*/
        sendMsg(bArrOs.toByteArray(),bArrOs.size());

        try
        {
            dataOs.close();
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }

        try
        {
            bArrOs.close();
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }

        waitResponse();
        log("login result :" + getOpResult());
        if(getOpResult() ==0x00)
        {
            islogin = true;
        }

        return getOpResult();
    }

    public void login() throws WMMPException
    {
        try
        {
            login(null,false,false);
        }
        catch(Exception e)
        {
            throw new WMMPException();
        }
    }

    public void logout(short type) throws WMMPException
    {
        byte[] buff;
        ByteArrayOutputStream bArrOs = null;
        DataOutputStream dataOs = null;
        int gramlen =0;
        byte safeId =0;

        try{
            ensureOpen();
        }
        catch(Exception e){
            throw new WMMPException();
        }

        /*first step: check register*/

        try{
            /*length: 28+1 = 29*/
            gramlen = 29;
            bArrOs = new ByteArrayOutputStream(gramlen);
            dataOs = new DataOutputStream(bArrOs);

            /*write gram head*/
            buff = getGramHead(gramlen,WMMPConnection.WMMP_VERSION,WMMPConnection.LOGOUT,getFlowId(true),(byte)0x00);
            dataOs.write(buff,0,buff.length);
            dataOs.writeByte(type);
            dataOs.flush();

            /*send msg out*/
            sendMsg(bArrOs.toByteArray(),bArrOs.size());
        }
        catch(Exception e){
            throw new WMMPException();
        }

        try{
            dataOs.close();
            bArrOs.close();
        }
        catch(IOException e){
            throw new WMMPException();
        }

        waitResponse();
        log("login result :" + getOpResult());
        islogin = false;

        //return getOpResult();
    }

    public int sendConfigTrap(byte[] tlvs) throws IOException
    {
        byte[] buff;
        ByteArrayOutputStream bArrOs = null;
        DataOutputStream dataOs = null;
        int gramlen =0;
        byte safeId =0;

        ensureOpen();

        /*first step: check register*/


        /*base length: 28+20 = 48*/
        gramlen = 28;
        gramlen += 20;  //e021
        if(tlvs != null)
        {
            gramlen = gramlen + tlvs.length;
        }
        bArrOs = new ByteArrayOutputStream(gramlen);
        dataOs = new DataOutputStream(bArrOs);

        /*write gram head*/
        buff = getGramHead(gramlen,WMMPConnection.WMMP_VERSION,WMMPConnection.CONFIG_TRAP,getFlowId(true),(byte)0x00);
        dataOs.write(buff,0,buff.length);
        /*write tlvs data*/
        if(tlvs != null)
        {
            dataOs.write(tlvs,0,tlvs.length);
        }
        /*write 0xE021*/
        buff = getTlvE021(buff,link_timestamp,WMMPRMS.getRmsVal(PARAM_UPLINKPWD).getBytes());
        dataOs.write(buff,0,buff.length);
        dataOs.flush();

        /*send msg out*/
        sendMsg(bArrOs.toByteArray(),bArrOs.size());

        try
        {
            dataOs.close();
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }

        try
        {
            bArrOs.close();
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }

        //fuck...this is a bug.nix at 2014.1.13
        //comment waitResponse and fake an false result.because CONFIG_TRAP_ACK is in problem.
        //
        /*
        waitResponse();
        */
        setOpResult(0);
        setCurLinkState(STATE_LINKING);
        setContinusFailTimes(0);
        
        log("login result :" + getOpResult());

        return getOpResult();
    }

    /**
     * send TransParent data to other equipment for application called
     *
     * @param    Dest    the destination address of the data to be send
     *
     * @param    src     the source address of the data send from
     *
     * @param    data    the data need to be send
     *
     * @param    mode    the data send via sms or gprs
     *
     * @return   data send result
     *
     */
    public int sendTransParentData(String Dest,String src,byte[] data,int mode)
    {
        return 0;
    }

    /*The API contains register-login-config_trap flow,package them!*/
    public boolean packageLogin()
    {
        int ret =0;

        /*check register*/
        /*
        if(!checkIfRegister())
        {
            try
            {
                ret = register(WMMPConnection.REG_TYPE_1);
            }
            catch(IOException e)
            {
                e.printStackTrace();
                ret = -1;
            }
            log("Register ret: " + ret);
        }

        if(ret !=0 && ret !=6)
        {
            log("Register fail!");
            return false;
        }*/

        try
        {
            ret = login(null,false,false);
        }
        catch(IOException e)
        {
            e.printStackTrace();
            ret = -1;
        }
        log("login result is:" + ret);

        if(ret !=0 )
        {
            log("login fail!");
            return false;
        }

        log("sendConfigTrap ");
        try
        {
            ret = sendConfigTrap(null);
        }
        catch(IOException e)
        {
            e.printStackTrace();
            ret = -1;
        }

        if(ret !=0 && ret != 1)
        {
            log("sendConfigTrap fail!");
            return false;
        }

        return true;
    }

    /*send heart beat*/
    public void sendHeartBeatMsg() throws IOException
    {
        /*with gramHead ,no gram body!*/
        byte[] gramHead;

        gramHead = getGramHead(28,WMMPConnection.WMMP_VERSION,WMMPConnection.HEART_BEAT,getFlowId(true),(byte)0x00);

        sendMsg(gramHead,gramHead.length);
    }

    public void sendRemoteControlACK(int result,byte[] tlvs,int tlvslen,boolean safetag,int sequenceid)throws IOException
    {
        byte[] buff;
        ByteArrayOutputStream bArrOs = null;
        DataOutputStream dataOs = null;
        int gramlen =0;
        long curTime = 0;
        int cur = 0;

        /*first step: check register*/
        ensureOpen();

        /*datagram length: ±¨ÎÄÍ·+1+tlvs³¤¶È+°²È«ÕªÒªÌå */
        //±¨ÎÄÍ· + ½á¹û
        gramlen = 28 + 1;
        //tlvs³¤¶È
        if(tlvslen>0)
        {
            gramlen = gramlen + tlvslen;
        }
        //°²È«ÕªÒªÌå
        if(safetag == true)
        {
            gramlen = gramlen + 20;
        }

        bArrOs = new ByteArrayOutputStream(gramlen);
        dataOs = new DataOutputStream(bArrOs);

        /*write gram head*/
        buff = getGramHead(gramlen,0x0301,WMMPConnection.REMOTE_CTRL_ACK,sequenceid,(byte)0x00);
        dataOs.write(buff,0,buff.length);

        /*write ACK result value*/
        dataOs.writeByte(result);

        /*if need to send tlvs data, add tlvs to send buff*/
        if(tlvslen > 0)
        {
            dataOs.write(tlvs,0,tlvslen);
        }

        /*add safe tag 0xE021 to send buff*/
        if(safetag == true)
        {
            buff = bArrOs.toByteArray();
            log("buff.length="+buff.length);
            curTime = System.currentTimeMillis()/1000;
            log("curTime="+curTime);
            cur = (int)curTime;
            log("cur="+cur);
            log("link_timestamp="+link_timestamp);
            //write 0xE021 MD5(±¨ÎÄ+Timestamp+IMEI+IMSI+ÉÏÐÐ½ÓÈëÃÜÂë)
            log("======WMMPRMS.getRmsVal(PARAM_UPLINKPWD)=" + WMMPRMS.getRmsVal(PARAM_UPLINKPWD));
            buff = getTlvE021(buff,link_timestamp,WMMPRMS.getRmsVal(PARAM_UPLINKPWD).getBytes());
            dataOs.write(buff,0,buff.length);
        }

        /*send msg out*/
        sendMsg(bArrOs.toByteArray(),bArrOs.size());

        try
        {
            dataOs.close();
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }

        try
        {
            bArrOs.close();
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }
    }


    public void sendTransParentDataACK(int result,byte[] tlvs,int tlvslen,boolean safetag,int sequenceid)throws IOException
    {
        byte[] buff;
        ByteArrayOutputStream bArrOs = null;
        DataOutputStream dataOs = null;
        int gramlen =0;
        long curTime = 0;
        int cur = 0;

        /*first step: check register*/
        ensureOpen();

        /*datagram length: ±¨ÎÄÍ·+1+tlvs³¤¶È+°²È«ÕªÒªÌå */
        //±¨ÎÄÍ· + ½á¹û
        gramlen = 28 + 1;
        //tlvs³¤¶È
        if(tlvslen>0)
        {
            gramlen = gramlen + tlvslen;
        }
        //°²È«ÕªÒªÌå
        if(safetag == true)
        {
            gramlen = gramlen + 20;
        }

        bArrOs = new ByteArrayOutputStream(gramlen);
        dataOs = new DataOutputStream(bArrOs);

        /*write gram head*/
        buff = getGramHead(gramlen,0x0301,WMMPConnection.REMOTE_CTRL_ACK,sequenceid,(byte)0x00);
        dataOs.write(buff,0,buff.length);

        /*write ACK result value*/
        dataOs.writeByte(result);

        /*if need to send tlvs data, add tlvs to send buff*/
        if(tlvslen > 0)
        {
            dataOs.write(tlvs,0,tlvslen);
        }

        /*add safe tag 0xE021 to send buff*/
        if(safetag == true)
        {
            buff = bArrOs.toByteArray();
            log("buff.length="+buff.length);
            curTime = System.currentTimeMillis()/1000;
            log("curTime="+curTime);
            cur = (int)curTime;
            log("cur="+cur);
            log("link_timestamp="+link_timestamp);
            //write 0xE021 MD5(±¨ÎÄ+Timestamp+IMEI+IMSI+ÉÏÐÐ½ÓÈëÃÜÂë)
            log("======WMMPRMS.getRmsVal(PARAM_UPLINKPWD)=" + WMMPRMS.getRmsVal(PARAM_UPLINKPWD));
            buff = getTlvE021(buff,link_timestamp,WMMPRMS.getRmsVal(PARAM_UPLINKPWD).getBytes());
            dataOs.write(buff,0,buff.length);
        }

        /*send msg out*/
        sendMsg(bArrOs.toByteArray(),bArrOs.size());

        try
        {
            dataOs.close();
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }

        try
        {
            bArrOs.close();
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }
    }


    /*handle remote control TLV*/
    public int handleRemoteControl(byte[]  tlvBuffer,int length)
    {
        int filter = 0xffff;
        int tagId = 0;
        int dataLen = 0;
        int pos = 0; //identify current read position
        log("handleRemoteControl called length:" + length);
        while(pos<(length-4))
        {
            tagId = 0;
            dataLen = 0;
            log("handleRemoteControl pos:" + pos);
            tagId = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
            tagId = tagId&filter;
            pos = pos+2;
            log("handleRemoteControl tagId:" + tagId);

            switch(tagId)
            {
                case WMMPConnection.RC_MAIN_COMMAND:
                {
                    //RC_MAIN_COMMAND Êý¾Ý³¤¶ÈÎª1
                    dataLen = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control RC_MAIN_COMMAND fix=1 dataLen=" + dataLen);
                    byte cmdId = tlvBuffer[pos];
                    pos = pos + 1;
                    log("remote control RC_MAIN_COMMAND cmd=" + cmdId);
                    if(cmdId == 0x02)
                    {
                        log("remote contorl request restart command recevied");
                    }
                    else if(cmdId == 0x06)
                    {
                        log("remote contorl send config trap command recevied");
                        //TODO: after send ACK back , send config trap back
                        Timer timer = new Timer();
                        timer.schedule(new sendConfigTrapTask(), 5000);
                    }
                }
                break;
                case WMMPConnection.RC_LINK_TYPE:
                {
                    //RC_LINK_TYPE Êý¾Ý³¤¶ÈÎª1
                    dataLen = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control RC_LINK_TYPE fix=1 dataLen=" + dataLen);
                    byte linkType = tlvBuffer[pos];
                    pos = pos + 1;
                    log("remote control RC_LINK_TYPE linkType=" + linkType);
                }
                break;
                case WMMPConnection.RC_DATA_TRANSFER_TYPE:
                {
                    //RC_DATA_TRANSFER_TYPE Êý¾Ý³¤¶ÈÎª1
                    dataLen = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control RC_DATA_TRANSFER_TYPE fix=1 dataLen=" + dataLen);
                    byte dataTransferType = tlvBuffer[pos];
                    pos = pos + 1;
                    log("remote control RC_DATA_TRANSFER_TYPE dataTransferType=" + dataTransferType);
                }
                break;
                case WMMPConnection.RC_HEART_BEAT:
                {
                    //RC_HEART_BEAT Êý¾Ý³¤¶ÈÎª2
                    dataLen = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control RC_HEART_BEAT fix=2 dataLen=" + dataLen);
                    int heartBeat = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control RC_HEART_BEAT heartBeat=" + heartBeat);
                }
                break;
                case WMMPConnection.RC_BUSINESS_DATA_TRANSFER_TYPE:
                {
                    //RC_BUSNIESS_DATA_TRANSFER_TYPE Êý¾Ý³¤¶ÈÎª1
                    dataLen = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                     log("remote control RC_BUSNIESS_DATA_TRANSFER_TYPE fix=1 dataLen:"+dataLen);
                    byte businessDataTransferType = tlvBuffer[pos];
                    pos = pos + 1;
                    log("remote control RC_BUSNIESS_DATA_TRANSFER_TYPE businessDataTransferType=" + businessDataTransferType);
                }
                break;
                case WMMPConnection.RC_DATA_TRANSFER_MODE:
                {
                    //RC_DATA_TRANSFER_MODE Êý¾Ý³¤¶ÈÎª2
                    dataLen = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control RC_DATA_TRANSFER_MODE fix=2 dataLen=" + dataLen);
                    byte iptype = tlvBuffer[pos];
                    byte protocol = tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control RC_DATA_TRANSFER_MODE iptype=" + iptype + " protocol= " + protocol);
                }
                break;
                case WMMPConnection.RC_USER_DATA_TAG:
                {
                    //RC_USER_DATA_TAG Êý¾Ý³¤¶È²»¶¨³¤,ÔòÎªdataLen
                    dataLen = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control RC_USER_DATA_TAG dataLen=" + dataLen);
                    byte[] userTag = new byte[dataLen];
                    for(int i=0;i<dataLen;i++)
                    {
                        userTag[i] = tlvBuffer[pos+i];
                        log("RC_USER_DATA_TAG user tag["+i+"]"+userTag[i]);
                    }
                    pos = pos + dataLen;
                }
                break;
                case WMMPConnection.RC_DATA_DISTRIBUTE_PARAMETER:
                {
                    //RC_DATA_DISTRIBUTE_PARAMETER Êý¾Ý³¤¶È²»¶¨³¤,ÔòÎªdataLen
                    dataLen = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control RC_DATA_DISTRIBUTE_PARAMETER dataLen=" + dataLen);
                    byte[] distributePara = new byte[dataLen];
                    for(int i=0;i<dataLen;i++)
                    {
                        distributePara[i] = tlvBuffer[pos+i];
                        log("user tag["+i+"]"+distributePara[i]);
                    }
                    pos = pos + dataLen;
                }
                break;
                case WMMPConnection.RC_SERVER_PLATFORM_SET_PARAMETER:
                {
                    //RC_SERVER_PLATFORM_SET_PARAMETER Êý¾Ý³¤¶ÈÎª1
                    dataLen = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control RC_SERVER_PLATFORM_SET_PARAMETER fix=1 dataLen=" + dataLen);
                    byte serverPlatformSetPara = tlvBuffer[pos];
                    pos = pos + 1;
                    log("remote control RC_SERVER_PLATFORM_SET_PARAMETER serverPlatformSetPara=" + serverPlatformSetPara);
                }
                break;
                case WMMPConnection.RC_CLIENT_PLATFORM_SET_PARAMETER:
                {
                    //RC_CLIENT_PLATFORM_SET_PARAMETER Êý¾Ý³¤¶ÈÎª5
                    dataLen = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control RC_CLIENT_PLATFORM_SET_PARAMETER received fix=5 dataLen:" + dataLen);
                    byte clientPlatformSetPara = tlvBuffer[pos];
                    byte[]  clientFlowId = new byte[4];
                    for(int i=0;i<4;i++)
                    {
                        clientFlowId[i] = tlvBuffer[pos+i+1];
                        log("clientFlowId["+i+"] "+clientFlowId[i] );
                    }
                    pos = pos + 5;
                }
                break;
                case WMMPConnection.RC_CLIENT_MANUAL_SET_APPLY_TAG:
                {
                    //RC_SERVER_PLATFORM_SET_PARAMETER Êý¾Ý³¤¶ÈÎª0
                    log("remote control RC_CLIENT_MANUAL_SET_APPLY_TAG fix=0 recevied dataLen is 0");
                }
                break;
                case WMMPConnection.RC_TRANSPARENT_FROM_SERIAL_NUM:
                {
                    //RC_TRANSPARENT_FROM_SERIAL_NUM Êý¾Ý³¤¶ÈÎª16
                    dataLen = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control RC_TRANSPARENT_FROM_SERIAL_NUM fix=16 dataLen:"+dataLen);
                    byte[]  fromSerailNum = new byte[16];
                    for(int i=0;i<16;i++)
                    {
                        fromSerailNum[i] = tlvBuffer[i+pos];
                        log("fromSerailNum["+i+"]="+fromSerailNum[i]);
                    }
                    pos = pos + 16;
                }
                break;
                case WMMPConnection.RC_TRANSPARENT_TO_SERIAL_NUM:
                {
                    //RC_TRANSPARENT_TO_SERIAL_NUM Êý¾Ý³¤¶ÈÎª16
                    dataLen = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control RC_TRANSPARENT_TO_SERIAL_NUM fix=16 dataLen:"+dataLen);
                    byte[]  toSerailNum = new byte[16];
                    for(int i=0;i<16;i++)
                    {
                        toSerailNum[i] = tlvBuffer[i+pos];
                        log("toSerailNum["+i+"]="+toSerailNum[i]);
                    }
                    pos = pos + 16;
                }
                break;
                case WMMPConnection.RC_TRANSPARENT_TO_EC:
                {
                    //RC_TRANSPARENT_TO_EC Êý¾Ý³¤¶ÈÎª²»¶¨³¤
                    dataLen = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control RC_TRANSPARENT_TO_EC received dataLen:" + dataLen);
                    byte[]  toEC = new byte[dataLen];
                    for(int i=0;i<dataLen;i++)
                    {
                        toEC[i] = tlvBuffer[i+pos];
                        log("toEC["+i+"]="+toEC[i]);
                    }
                    pos = pos + dataLen;
                }
                break;
                case WMMPConnection.RC_TRANSPARENT_FROM_EC:
                {
                    //RC_TRANSPARENT_FROM_EC Êý¾Ý³¤¶ÈÎª²»¶¨³¤
                    dataLen = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control RC_TRANSPARENT_FROM_EC received dataLen:" + dataLen);
                    byte[]  fromEC = new byte[dataLen];
                    for(int i=0;i<dataLen;i++)
                    {
                        fromEC[i] = tlvBuffer[i+pos];
                        log("fromEC["+i+"]="+fromEC[i]);
                    }
                    pos = pos + dataLen;
                }
                break;
                case WMMPConnection.RC_TRANSPARENT_FIX_TAG:
                {
                    //RC_TRANSPARENT_FIX_TAG Êý¾Ý³¤¶ÈÎª²»¶¨³¤
                    dataLen = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control RC_TRANSPARENT_FIX_TAG received dataLen:"+dataLen);
                    byte[]  fixTag = new byte[dataLen];
                    for(int i=0;i<dataLen;i++)
                    {
                        fixTag[i] = tlvBuffer[i+pos];
                        log("fixTag["+i+"]="+fixTag[i]);
                    }
                    pos = pos + dataLen;
                }
                break;
                case WMMPConnection.RC_SAFE_CHECK_CRC32:
                {
                    //RC_SAFE_CHECK_CRC32 Êý¾Ý³¤¶ÈÎª4
                    dataLen = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control RC_SAFE_CHECK_CRC32 fix=4 dataLen:" + dataLen);
                    byte[]  crc32 = new byte[4];
                    for(int i=0;i<4;i++)
                    {
                        crc32[i] = tlvBuffer[i+pos];
                        log("crc32["+i+"]="+crc32[i]);
                    }
                    pos = pos + 4;
                }
                break;
                case WMMPConnection.RC_CLEINT_EQUIPMENT_CONTROL:
                {
                    //RC_CLEINT_EQUIPMENT_CONTROL Êý¾Ý³¤¶ÈÎª²»¶¨³¤
                    dataLen = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control RC_CLEINT_EQUIPMENT_CONTROL received dataLen:" +dataLen);
                    byte[]  equipment = new byte[dataLen];
                    for(int i=0;i<dataLen;i++)
                    {
                        equipment[i] = tlvBuffer[i+pos];
                        log("equipment["+i+"]="+equipment[i]);
                    }
                    pos = pos + dataLen;
                }
                break;
                case WMMPConnection.MD5_CHECK_TAG:
                {
                    //MD5_CHECK_TAG Êý¾Ý³¤¶ÈÎª16
                    dataLen = (tlvBuffer[pos]<<8) + tlvBuffer[pos+1];
                    pos = pos + 2;
                    log("remote control MD5_CHECK_TAG fix=16 datalen:"+ dataLen);
                    byte[]  md5buffer = new byte[dataLen];
                    for(int i=0;i<16;i++)
                    {
                        md5buffer[i] = tlvBuffer[i+pos];
                        log("md5buffer["+i+"]="+md5buffer[i]);
                    }
                    pos = pos + 16;
                }
                break;
                default:
                {
                    //½âÎöÊ§°Ü£¬·µ»Ø´íÎó
                    log("remote control tlvs parse error");
                    return -1;
                }
            }
        }
        return 0;
    }

    /*handle remote control TLV*/
    public int handleTransparentData(byte[]  tlvBuffer,int length)
    {
        ByteArrayInputStream bArrIs = new ByteArrayInputStream(tlvBuffer,0,length);
        DataInputStream dataIs      = new DataInputStream(bArrIs);
        int crc = 0;
        int tag,len;
        byte[] val;
        try {
            crc = dataIs.readChar();
            tag = dataIs.readChar();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        return 0;
    }

    /*handle remote control TLV*/
    public int handleTransparentDataACK(byte[]  tlvBuffer,int length)
    {

        return 0;
    }

/*
    private static class Md5Util
    {
        protected   static  MessageDigest messagedigest =  null ;
        static
        {
            try
            {
                messagedigest = MessageDigest.getInstance("MD5");
            }
            catch(NoSuchAlgorithmException nsaex)
            {
                log("" + "MessageDigest is not support md5" );
                nsaex.printStackTrace();
            }
        }

        public static String getMD5String(byte[] bytes)
        {
            messagedigest.update(bytes);
            return  new String(messagedigest.digest());
        }

        public static byte[] getMD5ByteArray(byte[] bytes)
        {
            byte[] tmp;
            messagedigest.update(bytes);
            tmp = messagedigest.digest();
            return tmp;
        }
    }
*/
    private static void log(String str)
    {
        if(true)
            System.out.println("wmmp_protocol-" + str);
    }

    private static void logByteArr(byte[] arr)
    {
        int i=0;
        if(false)
        {
            System.out.print("wmmp_protocol-");
            for(i=0;i<arr.length;i++)
            {
                System.out.print(" " + arr[i]);
            }
            log("");
        }
    }

    /*implement of new Methods!*/
    public boolean checkDataItem(WMMPDataItem item) throws NullPointerException,WMMPException
    {
        // TODO Auto-generated method stub
        if(item == null){
            throw new NullPointerException();
        }
        return (item.getType() != WMMPDataItem.TLV_INVALID);
    }

    public boolean isAlive() {
        // TODO Auto-generated method stub
        return islogin;//connectionOpen;
    }

    public WMMPDataItem[] requestConfig(int[] itemTags) throws NullPointerException,WMMPException
    {
        // TODO Auto-generated method stub
        byte[] buff;
        ByteArrayOutputStream bArrOs = null;
        DataOutputStream dataOs = null;
        int gramlen =0;
        byte safeId =0;

        if(itemTags ==null) {
            throw new NullPointerException();
        }

        ensureOpen();
        ensureLogin();

        /*first step: check register*/

        /*base length: 28+20 = 48*/
        gramlen = 28;

        gramlen = gramlen + itemTags.length * 4;

        bArrOs = new ByteArrayOutputStream(gramlen);
        dataOs = new DataOutputStream(bArrOs);

        /*write gram head*/
        try{
            buff = getGramHead(gramlen,WMMPConnection.WMMP_VERSION,WMMPConnection.CONFIG_REQ,getFlowId(true),(byte)0x00);
            dataOs.write(buff,0,buff.length);
            /*write tlvs data*/
            for(int i=0;i<itemTags.length;i++) {
                dataOs.writeChar(itemTags[i]);
                dataOs.writeChar(0);
            }

            /*write 0xE021*/
            //buff = getTlvE021(buff,link_timestamp,WMMPRMS.getRmsVal(PARAM_UPLINKPWD).getBytes());
            //dataOs.write(buff,0,buff.length);
            dataOs.flush();
        } catch(Exception e) {
            throw new WMMPException();
        }

        /*send msg out*/
        sendMsg(bArrOs.toByteArray(),bArrOs.size());

        try {
            dataOs.close();
            bArrOs.close();
        } catch(IOException e) {
            e.printStackTrace();
            throw new WMMPException();
        }

        waitResponse();
        log("request config result :" + getOpResult());

        if(recvTlvDataStack != null && recvTlvDataStack.getCurStackCmd() == WMMPConnection.CONFIG_REQ_ACK) {
            int[] tags = recvTlvDataStack.getTagsSet();
            log("recvTlvDataStack and Cmd == CONFIG_REQ_ACK" + tags.length);
            if(tags != null && tags.length > 0 && itemTags.length>0) {
                log("tags and itemTags >0");
                for(int i=0;i<itemTags.length;i++) {
                    for(int j=0;j<tags.length;j++) {
                        log("itemTags["+i+"]="+itemTags[i]+"tags["+j+"]=" + tags[j]);
                        if(itemTags[i] == tags[j]) {
                            log("reuqest config  return recvTlvDataStack.getAllDataItem(); ");
                            return recvTlvDataStack.getAllDataItem();
                        }
                    }
                }
            }
        }
        log("reuqest config return null");
        return null;
    }

    public void sendData(byte destType, byte[] data, String dest) throws NullPointerException,WMMPException
    {
        // TODO Auto-generated method stub
        byte[] buff;
        ByteArrayOutputStream bArrOs = null;
        DataOutputStream dataOs = null;
        int gramlen =0;
        byte safeId =0;

        if(data == null || dest == null) {
            throw new NullPointerException("send data should not null");
        }
        
        if(data.length == 0 || dest.length() == 0) {
            throw new WMMPException("send data should not null");
        }

        ensureOpen();
        ensureLogin();

        gramlen = 28 + 2 + 4 + dest.length() + 4 + data.length;
        bArrOs = new ByteArrayOutputStream(gramlen);
        dataOs = new DataOutputStream(bArrOs);

        /*write gram head*/
        try {
            log("sendData == 1");
            buff = getGramHead(gramlen,WMMPConnection.WMMP_VERSION,WMMPConnection.TRANSPARENT_DATA,getFlowId(true),(byte)0x00);
            log("sendData == 2");
            dataOs.write(buff,0,buff.length);
            log("sendData == 3");
            CRC16 crcv = new CRC16(data);
            log("sendData == 4");
            byte[] crca = crcv.getCRC();
            log("sendData == 5");
            dataOs.write(crca,0,crca.length);
            log("sendData == 6");
            WMMPDataItem witem = null;
            if(destType == WMMPConnection.EC_TYPE)
            {
                log("sendData == 7");
                witem = new WMMPDataItem(0x4014,dest);
                log("sendData == 8");
            }
            else if(destType == WMMPConnection.TERMINAL_TYPE)
            {
                log("sendData == 9");
                witem = new WMMPDataItem(0x4012,dest);
                log("sendData == 10");
            }
            else
            {
                
            }
            log("sendData == 11");
            dataOs.write(witem.getTlvRawData(),0,witem.getTlvRawData().length);
            log("sendData == 12");
            witem = new WMMPDataItem(0x4007,data);
            log("sendData == 13");
            dataOs.write(witem.getTlvRawData(),0,witem.getTlvRawData().length);
            log("sendData == 14");

            dataOs.flush();
            log("sendData == 15");
            sendMsg(bArrOs.toByteArray(),bArrOs.size());
            log("sendData == 16");
            dataOs.close();
            log("sendData == 16.4");
            bArrOs.close();
            log("sendData == 16.8");
        } catch (IOException e) {
            // TODO Auto-generated catch block
            log("sendData == 17 e="+e);
            throw new WMMPException();
        }

        waitResponse();
        log("login result :" + getOpResult());
        return;
    }

    public WMMPListener getWMMPListener() {
        // TODO Auto-generated method stub
        return gwmmpListener;
    }

    public void setWMMPListener(WMMPListener listener) {
        // TODO Auto-generated method stub
        gwmmpListener = listener;
    }

    public void handleWMMPListener(int cmd,WMMPDataItem[] ditem)
    {
        if(gwmmpListener != null && ditem != null)
            gwmmpListener.onDataReceived(cmd,ditem);
    }

    public int[] trapConfig(WMMPDataItem[] items) throws NullPointerException,WMMPException
    {
        // TODO Auto-generated method stub

        int i = 0,ret =0;

        if(items ==null) {
            throw new NullPointerException();
        }

        ensureOpen();
        ensureLogin();

        ByteArrayOutputStream bArrOs = new ByteArrayOutputStream();
        DataOutputStream dataOs = new DataOutputStream(bArrOs);

        for(i=0;i<items.length;i++)
        {
            try {
                if(!checkDataItem(items[i]))
                {
                    continue;
                }
                dataOs.writeChar(items[i].getTag());
                dataOs.writeChar(items[i].getLen());
                dataOs.write(items[i].getRawData());
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                continue;
            }
        }

        try {
            ret = sendConfigTrap(bArrOs.toByteArray());
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            throw new WMMPException();
        }
        log("in trapConfig,sendConfigTrap ret:" + ret);
        try {
            dataOs.close();
            bArrOs.close();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
            throw new WMMPException();
        }
        if(recvTlvDataStack != null && recvTlvDataStack.getCurStackCmd() == WMMPConnection.CONFIG_TRAP_ACK) {
            
            return recvTlvDataStack.getTagsSet();
        } else {
            return null;
        }
    }

    //for what ???
    public void writeStackDataItem(WMMPDataItem item) throws NullPointerException,WMMPException
    {
        // TODO Auto-generated method stub
        if(item ==null){
            throw new NullPointerException();
        }

        ensureLogin();

        int tag = item.getTag();

        Integer key = new Integer(tag);

        if(!isSupportedTag(tag))
            throw new WMMPException();

        if(wmmpStackTable.containsKey(key))
        {
            wmmpStackTable.remove(key);
        }

        wmmpStackTable.put(key,item);
    }

    public WMMPDataItem readStackDataItem(int itemTag) throws IllegalArgumentException,WMMPException
    {
        WMMPDataItem wditem;
        //simplely check now
        if(!isSupportedTag(itemTag))
            throw new IllegalArgumentException();

        ensureLogin();

        // TODO Auto-generated method stub
        Integer key = new Integer(itemTag);

        WMMPDataItem item = (WMMPDataItem)wmmpStackTable.get(key);

        if(item ==null)
            throw new WMMPException();

        return item;
    }

    public void sendDownloadInfoAck(int resl)
    {
        byte[] buff;
        ByteArrayOutputStream bArrOs = null;
        DataOutputStream dataOs = null;
        int gramlen =0;

        try {
            ensureOpen();
        } catch (WMMPException e1) {
            // TODO Auto-generated catch block
            e1.printStackTrace();
        }

        /*first step: check register*/

        /*ÏÈ²»Òª¿É±ä²¿·Ö,²»´øE03AºÍE020£¬Ö»´øE021*/
        /*length: 28+8+4+20 = 60*/
        gramlen = 28 + 1;
        bArrOs = new ByteArrayOutputStream(gramlen);
        dataOs = new DataOutputStream(bArrOs);

        /*write gram head*/
        try {
            buff = getGramHead(gramlen,WMMPConnection.WMMP_VERSION,WMMPConnection.URL_INFO_ACK,getFlowId(true),(byte)0x00);
            dataOs.write(buff,0,buff.length);

            dataOs.writeByte(resl);
            dataOs.flush();
        } catch (IOException e1) {
            // TODO Auto-generated catch block
            e1.printStackTrace();
        }

        /*get E021,MD5(gram + 0x00000000,)
        buff = bArrOs.toByteArray();
        gramlen = gramlen -20;
        log("in login,hope len is " + gramlen + ",in fact len is:" + buff.length);
        buff = getTlvE021(buff,0x00000000,WMMPRMS.getRmsVal(PARAM_UPLINKPWD).getBytes());
        dataOs.write(buff,0,buff.length);
        */

        /*send msg out*/
        sendMsg(bArrOs.toByteArray(),bArrOs.size());
        setCurLinkState(STATE_LINKING);

        try
        {
            dataOs.close();
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }

        try
        {
            bArrOs.close();
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }
    }
}
