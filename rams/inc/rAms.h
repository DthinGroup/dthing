#ifndef __REMOTE_AMS_CLIENT_H__
#define __REMOTE_AMS_CLIENT_H__

#include <std_global.h>

#if defined(__cplusplus)
extern "C" {
#endif
/**
 * COMMAND TYPES:
 * The global definition of command types. Corresponding to server side definition.
 * In order to easily extend the commands, use integer sequences as the value.
 */
#define CMD_NONE     0
#define CMD_INSTALL  1
#define CMD_DELETE   2
#define CMD_RUN      3
#define CMD_LIST     4
#define CMD_OTA      5
#define CMD_DESTROY  6
#define CMD_VIEW     7
#define CMD_ACTIVATE 8
#define CMD_REMOVE   9
#define CMD_OSGI	 10
#define CMD_VIEWINFO 11


/**
 * client acknowledgement response code.
 */
#define ACK_LIST               0x04
#define ACK_RECV_WITHOUT_EXEC  0x11
#define ACK_RECV_EXEC          0x12
#define ACK_SHUTDOWN_NW        0x13
/* event queue size for c->s */
#define EVT_QUEUE_SIZE         8

/**
 * COMMNAD EXECUTION RESULT:
 */
#define FAIL        (0)
#define OK          (1)

/**
 * REMOTE SERVER INFORMATION DEFINTIONS:
 */
/* remote server address */
//#define RS_ADDRESS  0x2a79123e
#define RS_ADDRESS  0x0a0f04dd
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

/**
 * COMMAND EVENT DATA STRUCTURE:
 * it's used for CTS(client to server). 
 */
typedef struct EVENT_CMD_T {
    int32_t  length;
    uint8_t* evtData;
}EventCmd;

/**
 * launch remote AMS client. What is done is in this API:
 * 1. check the APP list whether is available, if No, load the APP list.
 * 2. create event queue for events process.
 * 3. launch Daemon thread to handle remote server commands.
 * @ramsIsolate, true means AMS client runs in a separate task. Otherwise,
 *               RAMS runs in current task.
 */
bool_t launchRemoteAMSClient(bool_t ramsIsolate, int32_t argc, uint8_t* argv[]);

/**
 * Delete an applet by specified ID.
 * @id, applet id while list returned.
 * @return true means success, otherwise false.
 */
extern bool_t deleteAppletById(int32_t id);

/**
 * Run an applet by specified ID.
 * @id, applet id while list returned.
 * @return true means success, otherwise false.
 */
extern bool_t runApplet(int32_t id);

extern bool_t destroyApplet(int32_t id);

extern bool_t sendBackExecResult(int32_t cmd, bool_t res);

#if defined(__cplusplus)
}
#endif

#endif
