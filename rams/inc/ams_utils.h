#ifndef __AMS_UTILS_H__
#define __AMS_UTILS_H__

#include <std_global.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Definitions for other remote controllers */
typedef struct RMTConfig
{
  char* addr;
  char* port;
  char* initData;
  char* user;
  char* pwd;
} RMTConfig;

/**
 * String Utils
 */
char* amsUtils_strdup(const char *src);
char* amsUtils_strappend(char **str, char *append);
char* amsUtils_strconcat(char **str, char*fmt, ...);

/**
 * Remote Config Utils
 */
bool_t amsUtils_initConfigData(char *pInitData);
bool_t amsUtils_writeConfigData(char *cfg);
bool_t amsUtils_readConfigData(RMTConfig **pp_cfg);
void amsUtils_releaseConfigData(RMTConfig **pp_cfg);

/**
 * AMS Utils
 */
char* amsUtils_getAppletList(bool_t isRunning);



#ifdef __cplusplus
}
#endif

#endif //__AMS_UTILS_H__