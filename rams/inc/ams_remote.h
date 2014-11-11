#ifndef __AMS_REMOTE_H__
#define __AMS_REMOTE_H__

#include <std_global.h>
#include <eventbase.h>
#include <ams.h>

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
#define EVT_CMD_OSGI	 (10)
#define EVT_CMD_VIEWINFO (11)

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
 * MAX LENGH OF COMMAND EVNENT QUEUE:
 * The event queue is cicular queue.
 */
#define EVT_QUEUE_LEN (8)


typedef enum _RemoteCommandType
{
    RCMD_ACK  = 1,
    RCMD_LOGIN,
    RCMD_CFGURL,
    RCMD_CFGACCOUNT,
    RCMD_INIT,
    RCMD_LIST,
    RCMD_INSTALL,
    RCMD_OTA,
    RCMD_DELETE,
    RCMD_DELETEALL,
    RCMD_RUN,
    RCMD_DESTROY,
    RCMD_STATUS,
    RCMD_RESET,
    RCMD_VIEW,
    RCMD_ACTIVATE,
    RCMD_REMOVE,
    RCMD_OSGI,
    RCMD_CANCEL,
    RCMD_CANCELALL,
    RCMD_MAX
} RemoteCommandType;


/* Definitions for other remote controllers */
typedef struct RMTConfig
{
  char* addr;
  char* port;
  char* initData;
  char* user;
  char* pwd;
} RMTConfig;

#define RCMD_OK (0)
#define RCMD_FAILURE (-1)


AppletProps* getAppletPropById(int32_t id);
void writebeIU32(uint8_t* p, uint32_t num);
int32_t readbeIU32(const uint8_t* p);

int32_t ams_remote_lifecycleProcess(Event *evt, void *userData);

bool_t ams_remote_list();
bool_t ams_remote_deleteAppletById(int32_t id);
bool_t ams_remote_runApplet(int32_t id);
bool_t ams_remote_destroyApplet(int32_t id);
bool_t ams_remote_ota(char * url);
bool_t ams_remote_sendBackExecResult(int32_t cmd, bool_t res);

bool_t ramsClient_isVMActive(void);

#ifdef __cplusplus
}
#endif

#endif