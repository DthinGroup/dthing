
package jp.co.cmcc.wmmp;

import java.io.*;
import java.net.*;

//import javax.microedition.io.*;



/*所有的wmmp协议接口都是序列化的，不能被同时调用，必须一个接口有了返回才能调用下一个接口
 *协议只有一个实例，不能再不同线程里面同时调用相同或者不同接口
 */

/*NOTICE:
 *1.UDP本身可作为短连接，可以通过心跳支持长连接，Tcp目前只支持长而不支持短连接，没有办法监听远程M2M请求(无SMS).
 */
public interface WMMPConnection
{
    public static final boolean ARM_VERSION = true;
    public static final boolean FAKE_DOWNLOAD = true;
    //wmmp version
    public static final int WMMP_VERSION = 0x0300;

    public static final byte EC_TYPE = 69;
    public static final byte TERMINAL_TYPE = 84;

    //field option
    public static final byte FRAGMENT  = 1;
    public static final byte KEEPALIVE = 2;
    public static final byte LINGER    = 3;

    //default is 60 seconds,only use in long connection
    public static final String HEART_BEAT_INTERVAL = "wmmp.heart_beat_interval" ;
    //default is 15 seconds,use in long/short connection
    public static final String HEART_BEAT_TIMEOUT  = "wmmp.heart_beat_timeout" ;
    //default is 3 number of times,use in long/short connection
    public static final String HEART_BEAT_TIMES    = "wmmp.heart_beat_times" ;

    public static final String WMMP_SCHEME      = "wmmp";
    public static final String WMMP_BEARER_KEY  = "bearer_type";
    public static final String WMMP_BEARER_TCP  = "tcp";
    public static final String WMMP_BEARER_UDP  = "udp";
    public static final String WMMP_BEARER_SMS  = "sms";

    public static final int TCP_PORT = 9991;
    public static final int UDP_PORT = 9992;

    /*command id*/
    public static final int LOGIN                = 0x0001;
    public static final int LOGIN_ACK            = 0x8001;
    public static final int LOGOUT               = 0x0002;
    public static final int LOGOUT_ACK           = 0x8002;
    public static final int HEART_BEAT           = 0x0003;
    public static final int HEART_BEAT_ACK       = 0x8003;
    public static final int TRANSPARENT_DATA     = 0x0004;
    public static final int TRANSPARENT_DATA_ACK = 0x8004;
    public static final int CONFIG_GET           = 0x0005;
    public static final int CONFIG_GET_ACK       = 0x8005;
    public static final int CONFIG_SET           = 0x0006;
    public static final int CONFIG_SET_ACK       = 0x8006;
    public static final int CONFIG_TRAP          = 0x0007;
    public static final int CONFIG_TRAP_ACK      = 0x8007;
    public static final int REGISTER             = 0x0008;
    public static final int REGISTER_ACK         = 0x8008;
    public static final int CONFIG_REQ           = 0x000a;
    public static final int CONFIG_REQ_ACK       = 0x800a;
    public static final int REMOTE_CTRL          = 0x000b;
    public static final int REMOTE_CTRL_ACK      = 0x800b;
    public static final int URL_INFO             = 0x000c;
    public static final int URL_INFO_ACK         = 0x800c;
    public static final int FILE_REQ             = 0x000d;
    public static final int FILE_REQ_ACK         = 0x800d;
    public static final int SECURITY_CONFIG      = 0x000e;
    public static final int SECURITY_CONFIG_ACK  = 0x800e;
    public static final int TRANSPARENT_CMD      = 0x000f;
    public static final int TRANSPARENT_CMD_ACK  = 0x800f;


    /*some import default params in terminal*/
    public  static final int PARAM_REGISTERSTATE = 0x01;
    public  static final int PARAM_TERMSEQSTR    = 0x02;
    public  static final int PARAM_UPLINKPWD     = 0x03;
    public  static final int PARAM_DOWNLINKPWD   = 0x04;
    public  static final int PARAM_IMEI          = 0x05;
    public  static final int PARAM_IMSI          = 0x06;
    public  static final int PARAM_TERMSOFTVER   = 0x07;

    public static final String TERMSEQSTR     = "A00MS8011390002S";      //length is 16  //终端序列接入号
    public static final String UPLINKPWD      = "vkwF2SJa";              //len is 8,上行接入密码
    public static final String DOWNLINKPWD    = "Uo3aHCbG";              //len is 8,下行接入密码
    public static final String IMEI           = "354494040518570\0";     //imei,16,extra 0
    public static final String IMSI           = "460020084678120";       //imei,15
    public static final String TERMSOFTVER    = "12345678";              //terminal software version
    public static final String BaseCipherKey  = "addhkfekw";             //基础密钥，不定长

    /*操作错误码,每次注册登录等操作后，如果返回失败，可以立刻读取错误值，这里是错误码定义*/
    public  static final int OPCODE_NONE     = 0x0000;

    /*注册类型*/
    public static final byte REG_TYPE_0 = 0x00;
    public static final byte REG_TYPE_1 = 0x01;
    public static final byte REG_TYPE_2 = 0x02;
    public static final byte REG_TYPE_3 = 0x03;
    public static final byte REG_TYPE_4 = 0x04;

    /*login type*/
    public static final byte LOGIN_TYPE_0 = 0x00;
    public static final byte LOGIN_TYPE_1 = 0x01;

    /*错误码*/
    public static final int ERR_NET_EXCEPTION = -1;

    /*REMOTE CONTROL TAG*/
    public static final int RC_MAIN_COMMAND                                    = 0x4001; //设置命令
    public static final int RC_LINK_TYPE                                              = 0x4002;//终端连接方式
    public static final int RC_DATA_TRANSFER_TYPE                         = 0x4003;//数据传送方式
    public static final int RC_HEART_BEAT                                          = 0x4004;//心跳间隔
    public static final int RC_BUSINESS_DATA_TRANSFER_TYPE       = 0x4005;//业务数据传送方式
    public static final int RC_DATA_TRANSFER_MODE                        = 0x4006; //M2M平台传输层和应用层协议
    public static final int RC_USER_DATA_TAG                                    = 0x4007;//用户数据标识
    public static final int RC_DATA_DISTRIBUTE_PARAMETER             = 0x4008;//分包机制参数
    public static final int RC_SERVER_PLATFORM_SET_PARAMETER = 0x4009;//平台参数配置选项
    public static final int RC_CLIENT_PLATFORM_SET_PARAMETER    = 0x40010;//终端执行平台参数配置选项
    public static final int RC_CLIENT_MANUAL_SET_APPLY_TAG         = 0x40011;//终端应用本地人工配置参数生效上报通知标识
    public static final int RC_TRANSPARENT_FROM_SERIAL_NUM       = 0x40012;//M2M终端/EC请求平台转发报文或应用数据的目的终端的序列号
    public static final int RC_TRANSPARENT_TO_SERIAL_NUM            = 0x40013;//M2M终端/EC接收到平台转发报文或应用数据的来源终端的序列号
    public static final int RC_TRANSPARENT_TO_EC                            = 0x40014;//M2M终端请求平台转发报文或应用数据的目的EC的代码
    public static final int RC_TRANSPARENT_FROM_EC                       = 0x40015;//M2M终端收到平台转发报文或应用数据来源的EC的代码
    public static final int RC_TRANSPARENT_FIX_TAG                          = 0x40016;//透传的控制命令的固定参数部分
    public static final int RC_SAFE_CHECK_CRC32                               = 0x40017;//配置参数TLV的CRC32校验，按TAG的递增排序，用于平台下发终端参数配置结果的校验。
    public static final int RC_CLEINT_EQUIPMENT_CONTROL                = 0x40021;//控制终端的外设

    public static final int MD5_CHECK_TAG                = 0xE021;//MD5 安全检查部分


    public int getOption(byte option) throws IOException;

    public void setOption(byte option,int value);

    //set C(heart_beat_interval)/T(heart_beat_timeout)/N(heart_beat_times)
    public void setHeartBeatVal(String key,int value) throws IOException;

    public int getHeartBeatVal(String key) throws IOException;




    /*抽象协议接口: 将协议分割为协议规范实现和API两部分，规范实现隐藏协议细节，
     *如同http等其他协议一样；API为提供给上层用户编写基于Wmmp协议的应用接口，
     *这些接口可以完全反映整个协议.
     *接口内部尽力全部实现为同步接口
     */

    //1.所有的交互握手细节均不在API反映,实现在规范内部
    //2.难点之一是过多的承载协议，tcp和udp有些相似之处能做到较为统一；但跟sms一起抽象应有点麻烦


    /*定义返回*/
    public static final int WMMP_OK = 0;

    //define in wmmp protocol
    public boolean isAlive();

    public void login() throws WMMPException;

    public void logout(short type) throws WMMPException;

    public int[] trapConfig(WMMPDataItem[] items) throws NullPointerException,WMMPException;

    public WMMPDataItem[] requestConfig(int[] itemTags) throws NullPointerException,WMMPException;

    public void sendData(byte destType,byte[] data,String dest) throws NullPointerException, WMMPException;

    public WMMPDataItem readStackDataItem(int itemTag) throws IllegalArgumentException,WMMPException;

    public void writeStackDataItem(WMMPDataItem item) throws NullPointerException,WMMPException;

    public boolean checkDataItem(WMMPDataItem item) throws NullPointerException, WMMPException;

    public void setWMMPListener(WMMPListener listener);

    public WMMPListener getWMMPListener();


    //extend the APIs

    //
    /*wmmpconnection = Connector.open(URL)之后*/

    //
    /*检查终端是否已经注册*/
    public boolean checkIfRegister();

    //带预制密码的注册
    public int register(byte type) throws IOException ;

    public int login(char varied[],boolean encrypt_session,boolean clrVars) throws IOException;

    public int sendConfigTrap(byte[] tlvs) throws IOException;

    /*The API contains register-login-config_trap flow,package them!*/
    public boolean packageLogin();

    public void sendDownloadInfoAck(int i);
    
    public void close() throws WMMPException;
    public void open() throws WMMPException;
}
