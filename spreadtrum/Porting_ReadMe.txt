1.��Ҫ�ĺ����ѡ��

1.1 оƬƽ̨�л���  

ĿǰDthing֧�� X86/ARM ����ƽ̨���룬������ƿ��ض����� vm_common.h

typedef enum
{
    ARCH_X86,
    ARCH_ARM_SPD
}ARCH_E;

���� ARCH_X86 Ϊ X86ƽ̨����, ARCH_ARM_SPD Ϊ ARM/չѶƽ̨����


1.2 ����Log���ƿ��� 

��Trace.h �ļ��������� TRACE_LEV �� ������Ϣ����ǼǺ꿪��

#define TRACE_LEV 3

֧��1,2,3,4һ��4������,�ֱ� ������� ����/����/����/��ͨ �ĸ���Ϣ�ȼ�

1.3 Remote Server�� Ip/Port ����

�� rasmclient.h �ļ�������ĺ�:

// remote ip
#define RS_ADDRESS  0x2a79123e

/* remote server port */
#define RS_PORT     7899


�ֱ��ʾRemote��Ip �Ͷ˿ں�

1.4 ��Ҫ�Զ���DthingӦ������Ŀ¼

�� properties.c �ļ���ָ��������� builtinProperties ����:

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

������Լ���Ҫ����Ŀ¼(��������)



2.��չѶƽ̨����ֲ


2.1  export

Ŀǰ�Ѿ������չѶ����IDH�汾��dthing��ֲ: 

PDA�ֻ� - 

������ - UPDATE_KX8800

�� Dthing����/spreadtrum/Ŀ¼��,

(export Ŀ¼���޸ĵ� IDH���ļ�)

���� bat �ű��� 
     
     movefile_board_kx8800.bat for update_kx8800
     
     movefile_phone.bat for pda�ֻ�

�ű������ǽ� dthing�ļ������� չѶIDH����.

set SRCDIR=D:\nix.long\ReDvmAll\dvm

set DESDIR=D:\WORK\Spreadtrum\KX8800B_CODE_UPDATE_1112\MS_Code\BASE

�޸�����2�䣬Ϊ�Լ��� ·������.

IDH����,dthing ����λ��  BASE\dthing Ŀ¼��



2.2 ��չѶ��ms_ref/source/base/src/init.c ���� dthing ��������

Dthing_startVM

������  SCI_InitRef ����ĩβ ���� SCI_ApplicationDefine �� ����


2.3 build 

��export�ļ���Ϻ󣬾Ϳ�����չѶIDH����������image������ǰչѶ��Ŀһ��





