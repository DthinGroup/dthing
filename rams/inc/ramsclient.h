
#ifndef __AMS_CLIENT_H__
#define __AMS_CLIENT_H__

#include <std_global.h>
#include <eventbase.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RAMS_INTERNAL_MARK (1<<8)

/**
 * COMMAND TYPES:
 * The global definition of command types. 
 * Corresponding to server side definition.
 * In order to easily extend the commands, 
 * use integer sequences as the value.
 */
#define EVT_CMD_NONE     (0)
#define EVT_CMD_INSTALL  (1)
#define EVT_CMD_DELETE   (2)
#define EVT_CMD_RUN      (3)
#define EVT_CMD_LIST     (4)
#define EVT_CMD_OTA      (5)
#define EVT_CMD_DESTROY  (6)
#define EVT_CMD_VIEW     (7)
#define EVT_CMD_ACTIVATE (8)
#define EVT_CMD_REMOVE   (9)
#define EVT_CMD_INIT	 (10)
#define EVT_CMD_CANCEL (11)
#define EVT_CMD_CANCELALL (12)
#define EVT_CMD_DELETEALL (14)
#define EVT_CMD_STATUS (15)
#define EVT_CMD_RESET (16)
#define EVT_CMD_CFGURL (17)
#define EVT_CMD_CFGACCOUNT (18)
#define EVT_CMD_OSGI (19)
#define EVT_CMD_VIEWINFO (20)

/* internal eventy commands */
#define EVT_CMD_DECLARE  (0|RAMS_INTERNAL_MARK)
#define EVT_CMD_PARSER   (1|RAMS_INTERNAL_MARK)


/**
 * client acknowledgement response code.
 */
#define ACK_CMD_LIST           0x04
#define ACK_RECV_WITHOUT_EXEC  0x11
#define ACK_RECV_AND_EXEC      0x12
#define ACK_SHUTDOWN_NETWORK   0x13


/**
 * REMOTE SERVER INFORMATION DEFINTIONS:
 */
/* remote server address */
//#define RS_ADDRESS  0x2a79123e
//#define RS_ADDRESS  0x0a0f04dd
#define RS_ADDRESS  0x7f000001

/* remote server port */
#define RS_PORT     8888


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

/**
 * MAX LENGH OF COMMAND EVNENT QUEUE:
 * The event queue is cicular queue.
 */
#define EVT_QUEUE_LEN (8)


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


/* The UNUSED ID definition: */
#define PROPS_UNUSED (-1)

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


int32_t ramsClient_lifecycleProcess(Event* evt, void* userData);

/**
 * Delete an applet by specified ID.
 * @id, applet id while list returned.
 * @return true means success, otherwise false.
 */
bool_t ramsClient_deleteAppletById(int32_t id);

/**
 * Run an applet by specified ID.
 * @id, applet id while list returned.
 * @return true means success, otherwise false.
 */
bool_t ramsClient_runApplet(int32_t id);

/**
 * Destroy an running app by specified ID.
 * @param id, applet id of app list.
 * @return true means success, otherwise false.
 */
bool_t ramsClient_destroyApplet(int32_t id);

bool_t ramsClient_ota(char * url);
bool_t ramsClient_install(char * url);

void sendOTAExeResult(int res);

//attention: not be protected by mutext,it's thread-unsafe
void refreshInstalledApp(void);

/**
 * Send back execution result for some async commands.
 * main 2 comands: EVT_CMD_RUN and EVT_CMD_DESTROY
 * @param cmd, current execution result for the comand.
 * @param res, the execution result.
 * @return true means success, otherwise false.
 */
bool_t ramsClient_sendBackExecResult(int32_t cmd, bool_t res);

unsigned char ramsClient_receiveRemoteCmd(int cmd, int suiteId, char *pData);

unsigned char ramsClient_receiveRemoteCmdEx(int cmd, int suiteId, char *pData, char **ppOutStr);

bool_t ramsClient_isVMActive(void);

#ifdef __cplusplus
}
#endif

#endif//__AMS_CLIENT_H__