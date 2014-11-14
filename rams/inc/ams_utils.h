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
bool_t amsUtils_initConfigData(const char *pInitData);
bool_t amsUtils_writeConfigData(char *cfg);
bool_t amsUtils_readConfigData(RMTConfig **pp_cfg);
void amsUtils_releaseConfigData(RMTConfig **pp_cfg);

/**
 * AMS Utils
 */
char* amsUtils_getAppletList(bool_t isRunning);
/**
 * @brief cancel specified applets from auto-start applet list in config file
 * @param pData char* should be formatted as "<suiteId>" or "<suiteId-1>,<suiteId-2>"
 *
 * @return bool_t TRUE while successfully cancelled from config file, otherwise FALSE
 */
bool_t amsUtils_cancelDefaultApp(const char* pData);

/**
 * @brief config user and password in config file
 * @param pData char* should be formatted as "<user>|<password>"
 *
 * @return bool_t TRUE while successfully, otherwise FALSE
 */
bool_t amsUtils_configAccount(const char* pData);

/**
 * @brief config server and port in config file
 * @param pData char* should be formatted as "<server>|<port>"
 *
 * @return bool_t TRUE while successfully, otherwise FALSE
 */
bool_t amsUtils_configAddress(const char* pData);


#ifdef __cplusplus
}
#endif

#endif //__AMS_UTILS_H__