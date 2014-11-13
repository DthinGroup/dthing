#ifndef __AMS_H__
#define __AMS_H__

#include <std_global.h>
#include <eventbase.h>
#include <opl_file.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * PRE-DEFINITION OF APPLET PROPERTIES:
 */
/**
 * Manifest relevant definitions and applet pre-defines propertes.
 */
/* manifest file name */
#define MANIFEST_MF "MANIFEST.MF"

/* Applet name */
#define APP_NAME "Applet-Name"
/* Applet version */
#define APP_VERSION "Applet-Version"
/* Applet vendor */
#define APP_VENDOR "Applet-Vendor"
/* Main class */
#define MAIN_CLASS "Main-Class"


/**
 * MAX NUMBER OF ISNTALLED APPLET:
 * define a maximum number of installed applets.
 * For easily manage the installed applications and avoid memory fragement.
 * if the real installed application number is bigger than this, a realloc or
 * simuator solution should be replace the implentation.
 */
#define MAX_APPS_NUM (16)

/**
 * MAX LENGTH OF APPLET PROPERTIES:
 * maxinum lenght of applet properties for displaying.
 * including: name, vender, version.
 */
#define MAX_APP_PROPS_LEN (64)


/* define a safe buffer for easily finalizing */
typedef void (*BufferFree)(void *);
typedef struct SafeBuffer_e
{
    BufferFree buffer_free;
    int32_t    bytes; /* available bytes size */
    uint8_t   *pBuf;
} SafeBuffer;


/**
 * APPLET PROPS DATA STRUCTURE:
 * Applet properties data struture defintions.
 */
typedef struct APPLET_PROPS_T {
    int32_t   id;
    uint8_t   name[MAX_APP_PROPS_LEN];
    uint8_t   vendor[MAX_APP_PROPS_LEN];
    uint8_t   version[MAX_APP_PROPS_LEN];
    uint8_t   mainCls[MAX_APP_PROPS_LEN];
    uint16_t  fname[MAX_APP_PROPS_LEN];
    uint8_t*  fpath;
    bool_t    isRunning;
    struct APPLET_PROPS_T* nextRunning;
} AppletProps;


/* The UNUSED ID definition: */
#define PROPS_UNUSED (-1)

typedef enum 
{
	ATYPE_MIN =0,
	ATYPE_NAMS=0,	//native ams,not support fro now
	ATYPE_RAMS,		//remote ams,controlled by remote http server
	ATYPE_SAMS,		//SMS ams,
	ATYPE_AAMS,		//AT ams
	ATYPE_MAX = ATYPE_AAMS
}AMS_TYPE_E;

#define AMS_MODULE_MARK (1<<9)
#define AMS_MODULE_NAMS (AMS_MODULE_MARK | 0)
#define AMS_MODULE_RAMS (AMS_MODULE_MARK | 1)
#define AMS_MODULE_SAMS (AMS_MODULE_MARK | 2)
#define AMS_MODULE_AAMS (AMS_MODULE_MARK | 3)

enum {
	AMS_FASM_STATE_GET_LIST =0x01,
	AMS_FASM_STATE_GET_RUN,
	AMS_FASM_STATE_GET_DELETE,
	AMS_FASM_STATE_GET_DESTROY,
	AMS_FASM_STATE_GET_OTA,
	
	AMS_FASM_STATE_ACK_MASK = 0x80,//use for mask,not real state
	AMS_FASM_STATE_ACK_LIST =0x81,
	AMS_FASM_STATE_ACK_RUN,
	AMS_FASM_STATE_ACK_DELETE,
	AMS_FASM_STATE_ACK_DESTROY,
	AMS_FASM_STATE_ACK_OTA,
};

/* Remote Control Command for AMS Access */
typedef enum _RemoteCommandType
{
    RCMD_ACK  = 1,
    RCMD_LIST,
    RCMD_RUN,
    RCMD_INSTALL,
    RCMD_OTA,
    RCMD_DELETE,
    RCMD_DELETEALL,
    RCMD_DESTROY,
    RCMD_LOGIN,
    RCMD_CFGURL,
    RCMD_CFGACCOUNT,
    RCMD_INIT,    
    RCMD_STATUS,
    RCMD_RESET,
    RCMD_OSGI,
    RCMD_CANCEL,
    RCMD_CANCELALL,
    RCMD_MAX
} RemoteCommandType;

typedef struct AmsCBData_s
{
	uint32_t module;			//
	uint32_t step;				//reserved!
	RemoteCommandType cmd;		//operate
	uint32_t result;			//op result
	void * exptr;				//extend data
}AmsCBData;
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
typedef void(*AmsCrtlCBFunc)(AmsCBData *);

/**
 * create dalvik VM thread to handle event. After this API is called, the new 
 * thread will start automatically without extra work.
 * @ramsThreadFunc, the new thread process funtion. refer to above definition of
                    RAMSThreadFunc.
 * @argc, the arguments number which passes to new thread.
 * @argv, the arguments list which passes to new thread.
 * @return, the identifier of this created thread.
 */
int32_t Ams_createVMThread(DVMThreadFunc pDvmThreadProc, int argc, void* argv[]);

int32_t VMThreadProc(int argc, char* argv[]);

void Ams_init();
void Ams_final();
bool_t Ams_regModuleCallBackHandler(AMS_TYPE_E type, AmsCrtlCBFunc modFunc);
bool_t Ams_unregModuleCallBackHandler(AMS_TYPE_E type);
//APIs
int32_t Ams_lifecycleProcess(Event *evt, void *userData);

void Ams_setCurCrtlModule(AMS_TYPE_E type);
int Ams_getCurCrtlModule();
AMS_TYPE_E Ams_getATypeByModule(int module);

void Ams_handleAck(int module,int cmd,void * data);

void Ams_listApp(AMS_TYPE_E type,AmsCrtlCBFunc func);
int Ams_runApp(int id,AMS_TYPE_E type,AmsCrtlCBFunc func);
int Ams_deleteApp(int id,AMS_TYPE_E type,AmsCrtlCBFunc func);
int Ams_otaApp(uint8_t* url,AMS_TYPE_E type,AmsCrtlCBFunc func);
int Ams_destoryApp(int id,AMS_TYPE_E type,AmsCrtlCBFunc func);

/**
 * AMS Remote Control API
 */
int Ams_handleRemoteCmdSync(int cmdId, AMS_TYPE_E cmdType, int suiteId, char *data, char** ppout);

#ifdef __cplusplus
}
#endif

#endif //__AMS_H__