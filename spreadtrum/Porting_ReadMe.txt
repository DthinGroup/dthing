1.重要的宏编译选项

1.1 芯片平台切换宏  

目前Dthing支持 X86/ARM 两个平台编译，编译控制开关定义在 vm_common.h

typedef enum
{
    ARCH_X86,
    ARCH_ARM_SPD
}ARCH_E;

其中 ARCH_X86 为 X86平台编译, ARCH_ARM_SPD 为 ARM/展讯平台编译


1.2 调试Log控制开关 

在Trace.h 文件，定义了 TRACE_LEV 的 调试信息输出登记宏开关

#define TRACE_LEV 3

支持1,2,3,4一共4个级别,分别 代表输出 错误/警告/调试/普通 四个信息等级

1.3 Remote Server的 Ip/Port 配置

在 rasmclient.h 文件，定义的宏:

// remote ip
#define RS_ADDRESS  0x2a79123e

/* remote server port */
#define RS_PORT     7899


分别表示Remote的Ip 和端口号

1.4 需要自定义Dthing应用运行目录

在 properties.c 文件的指针数组变量 builtinProperties 定义:

static const char * const builtinProperties[] =
{
    #define DEF_PROPERTY(k, v)  (k "\0" v)
#ifdef ARCH_X86 
    //DEF_PROPERTY("appdb.dir", "D:/dvm/appdb/"),
    DEF_PROPERTY("appdb.dir", "D:/nix.long/ReDvmAll/dvm/appdb/"),
#elif defined (ARCH_ARM_SPD)
    DEF_PROPERTY( "appdb.dir", "D:/dthing/" ),
#endif
    DEF_PROPERTY( "manufacture", "helio" )
};

请根据自己需要更改目录(现在这样)



2.向展讯平台的移植


2.1  export

目前已经完成向展讯两个IDH版本的dthing移植: 

PDA手机 - 

开发板 - UPDATE_KX8800

在 Dthing代码/spreadtrum/目录下,

(export 目录是修改的 IDH包文件)

存在 bat 脚本： 
     
     movefile_board_kx8800.bat for update_kx8800
     
     movefile_phone.bat for pda手机

脚本作用是将 dthing文件拷贝到 展讯IDH包中.

set SRCDIR=D:\nix.long\ReDvmAll\dvm

set DESDIR=D:\WORK\Spreadtrum\KX8800B_CODE_UPDATE_1112\MS_Code\BASE

修改如上2句，为自己的 路径即可.

IDH包中,dthing 代码位于  BASE\dthing 目录下



2.2 在展讯的ms_ref/source/base/src/init.c 加入 dthing 启动函数

Dthing_startVM

可以在  SCI_InitRef 函数末尾 或者 SCI_ApplicationDefine 内 均可


2.3 build 

在export文件完毕后，就可以在展讯IDH包编译生成image，跟以前展讯项目一致





