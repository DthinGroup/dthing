#include "vm_common.h"
#include "ams_utils.h"
#include "opl_file.h"
#include "trace.h"
#include "ams.h"
#include "vm_app.h"

#define MAX_PATH_LENGTH   255
#define MAX_FILE_BUFF_LEN 128
#define DEFAULT_RMT_CONFIG_FILE  L"D:/RemoteConfig.cfg"
#define DEFAULT_RMT_CONFIG_FILE_PATH_LEN 19
#define DEFAULT_SERVER "42.121.18.62"
#define DEFAULT_PORT "8888"
//#define DEFAULT_SERVER "218.206.176.236"
//#define DEFAULT_PORT "7777"
#define DEFAULT_USER_NAME "test_username"
#define DEFAULT_PASSWORD "test_password"

/*******************************************************
 * String Utils
 *******************************************************/
char* amsUtils_strdup(const char *src)
{
  char *result = NULL;
  int len = 0;

  if (src == NULL)
  {
    goto end;
  }
  len = strlen(src);
  result = malloc(len + 1);
  memset(result, 0x0, len + 1);
  memcpy(result, src, len);

end:
  return result;
}

char* amsUtils_strappend(char **str, char *append)
{
  char *newStr = NULL;
  int sLen = 0;
  int aLen = 0;

  if ((str == NULL) || (append == NULL))
  {
    return NULL;
  }

  sLen = (*str != NULL)? strlen((char *)*str) : 0;
  aLen = strlen(append);
  newStr = malloc(sLen + aLen + 1);
  memset(newStr, 0x0, sLen + aLen + 1);

  if ((*str != NULL) || (strlen(*str) != 0))
  {
    memcpy(newStr, (char *)*str, sLen);
  }

  memcpy(newStr + sLen, append, aLen);
  free((char *)*str);
  *str = newStr;
  return newStr;
}

char* amsUtils_strconcat(char **str, char*fmt, ...)
{
  char *newStr = NULL;
  char buf[255] = {0};
  va_list lst;

  va_start(lst, fmt);
  sprintf(buf, fmt, lst);
  va_end(lst);
  newStr = amsUtils_strappend(str, buf);
  return newStr;
}

/*******************************************************
 * Remote Config Data Utils
 *******************************************************/
bool_t amsUtils_initConfigData(const char *pInitData)
{
  bool_t ret = FALSE;

#if defined(ARCH_ARM_SPD)
  char content[MAX_PATH_LENGTH] = {0};
  bool_t needCleanInitData = FALSE;
  RMTConfig *cfgData = NULL;

  memset(content, 0x0, MAX_PATH_LENGTH);

  if ((pInitData == NULL) || strlen(pInitData) == 0)
  {
    needCleanInitData = TRUE;
  }

  ret = amsUtils_readConfigData(&cfgData);

  if (ret)
  {
    sprintf(content, "%s|%s|%s%s|%s|%s", cfgData->addr, cfgData->port, needCleanInitData? "" : "s:",
    needCleanInitData? "0" : pInitData, cfgData->user, cfgData->pwd);
    ret = amsUtils_writeConfigData(content);
    amsUtils_releaseConfigData(&cfgData);
  }
  else
  {
    sprintf(content, "%s|%s|%s%s|%s|%s", DEFAULT_SERVER, DEFAULT_PORT, needCleanInitData? "" : "s:",
    needCleanInitData? "0" : pInitData, DEFAULT_USER_NAME, DEFAULT_PASSWORD);
    ret = amsUtils_writeConfigData(content);
  }
#endif

  return ret;
}

bool_t amsUtils_readConfigData(RMTConfig **pp_cfg)
{
  RMTConfig *config = NULL;
  bool_t ret = FALSE;

#if defined(ARCH_ARM_SPD)
  int file_readLen = 0;
  int result = FILE_RES_SUCCESS;
  int32_t sfsHandle = 0;
  char file_buff[MAX_FILE_BUFF_LEN] = {0};
  char *content = NULL;

  int i = 0;

  //to parse remoteconfig.txt
  result = file_open(DEFAULT_RMT_CONFIG_FILE, DEFAULT_RMT_CONFIG_FILE_PATH_LEN, FILE_MODE_RD, &sfsHandle);

  if(sfsHandle != INVALID_HANDLE_VALUE)
  {
    DthingTraceD("==RMT== amsUtils_readConfigData() get config file");
    content = malloc(MAX_FILE_BUFF_LEN);
    memset(content, 0x0, MAX_FILE_BUFF_LEN);
    file_readLen = file_read(sfsHandle, content, MAX_FILE_BUFF_LEN);
    if(file_readLen > 0)
    {
      char addr[128] = {0};
      char port[128] = {0};
      char initData[128] = {0};
      char user[128] = {0};
      char pwd[128] = {0};

      //TODO: Check if initData is NULL, how much params would be returned by sscanf, 4 or 5
      if (sscanf(content, "%[^|]|%[^|]|%[^|]|%[^|]|%s", addr, port, initData, user, pwd) < 4)
      {
        DthingTraceD("==RMT== amsUtils_readConfigData() error data format %s in file", content);
        goto end;
      }

      config = malloc(sizeof(RMTConfig));
      memset(config, 0x0, sizeof(RMTConfig));

      config->addr = amsUtils_strdup(addr);
      config->port = amsUtils_strdup(port);
      config->initData = amsUtils_strdup(initData);
      config->user = amsUtils_strdup(user);
      config->pwd = amsUtils_strdup(pwd);
      DthingTraceD("==RMT== amsUtils_readConfigData() read data: %s", content);
      ret = TRUE;
    }
    free(content);
    file_close(sfsHandle);
  }
#endif

end:
  *pp_cfg = config;
  return ret;
}

bool_t amsUtils_writeConfigData(char *cfg)
{
  bool_t ret = FALSE;

#if defined(ARCH_ARM_SPD)
  int file_writeLen = 0;
  int result = FILE_RES_SUCCESS;
  int32_t sfsHandle = 0;

  if (!cfg)
  {
    DthingTraceD("==RMT== amsUtils_writeConfigData() write config file failed");
    return ret;
  }

  //DeleteFile first then we can create new one
  result = file_delete(DEFAULT_RMT_CONFIG_FILE, DEFAULT_RMT_CONFIG_FILE_PATH_LEN);
  //to parse remoteconfig.cfg
  result = file_open(DEFAULT_RMT_CONFIG_FILE, DEFAULT_RMT_CONFIG_FILE_PATH_LEN, FILE_MODE_RDWR, &sfsHandle);
  if(sfsHandle != INVALID_HANDLE_VALUE)
  {
    DthingTraceD("==RMT== amsUtils_writeConfigData() write data: %s", cfg);
    file_writeLen = file_write(sfsHandle, cfg, strlen(cfg));
    if(file_writeLen > 0)
    {
      DthingTraceD("==RMT== amsUtils_writeConfigData() write config file success");
      ret = TRUE;
    }
    else
    {
      DthingTraceD("==RMT== amsUtils_writeConfigData() write config file failed");
    }
    file_close(sfsHandle);
  }
  else
  {
    DthingTraceD("==RMT== amsUtils_writeConfigData() create config file failed");
  }
#endif

  return ret;
}

void amsUtils_releaseConfigData(RMTConfig **pp_cfg)
{
#if defined(ARCH_ARM_SPD)
  RMTConfig *cfg = NULL;

  if (pp_cfg)
  {
    if (*pp_cfg)
    {
      cfg = *pp_cfg;
      if (cfg->addr)
      {
        free(cfg->addr);
      }
      if (cfg->port)
      {
        free(cfg->port);
      }
      if (cfg->initData)
      {
        free(cfg->initData);
      }
      if (cfg->user)
      {
        free(cfg->user);
      }
      if (cfg->pwd)
      {
        free(cfg->pwd);
      }
      free(*pp_cfg);
      *pp_cfg = NULL;
    }
  }
#endif
}

/*******************************************************
 * AMS Utils
 *******************************************************/
char* amsUtils_getAppletList(bool_t isRunning)
{
  char* list = NULL;

#if defined(ARCH_ARM_SPD)
  AppletProps *curApp = vm_getCurApplist(TRUE);

  while(curApp != NULL)
  {
    if ((curApp->isRunning == isRunning) && (curApp->id != PROPS_UNUSED))
    {
      amsUtils_strconcat(&list, "%d %s\r\n", curApp->id, curApp->fname);
    }
    curApp = curApp->nextRunning;
  }

  if (NULL == list)
  {
    list = amsUtils_strdup(isRunning? "No Active Applet" : "No Applet");
  }
#endif
  return list;
}

/**
 * @brief cancel specified applets from auto-start applet list in config file
 * @param pData char* should be formatted as "<suiteId>" or "<suiteId-1>,<suiteId-2>"
 *
 * @return bool_t TRUE while successfully, otherwise FALSE
 */
bool_t amsUtils_cancelDefaultApp(const char* pData)
{
    bool_t ret = FALSE;
#if defined(ARCH_ARM_SPD)
    char init1[16] = {0};
    char init2[16] = {0};
    RMTConfig *cfgData = NULL;
    char content[MAX_PATH_LENGTH] = {0};

    DthingTraceD("=== RemoteCmd CMD_CANCEL - data = %s\n", pData);

    if ((pData == NULL) || (strlen(pData) == 0))
    {
        return FALSE;
    }

    ret = amsUtils_readConfigData(&cfgData);

    if (ret)
    {
        sscanf(cfgData->initData, "%[^,],%s", init1, init2);

        if (init1 && (strcmp(init1, pData) != 0))
        {
            sprintf(content, "%s", init1);
        }

        if (init2 && (strcmp(init2, pData) != 0))
        {
            if (strlen(content) > 0)
            {
                sprintf(content, "%s,%s", content, init2);
            }
            else
            {
                sprintf(content, "%s", init2);
            }
        }
        ret = amsUtils_initConfigData(content);
    }

#endif
    return ret;
}

/**
 * @brief config user and password in config file
 * @param pData char* should be formatted as "<user>|<password>"
 *
 * @return bool_t TRUE while successfully, otherwise FALSE
 */
bool_t amsUtils_configAccount(const char* pData)
{
    bool_t ret = FALSE;
#if defined(ARCH_ARM_SPD)
    char content[MAX_PATH_LENGTH] = {0};
    RMTConfig *cfgData = NULL;

    DthingTraceD("=== ReceiveRemoteCmd CMD_CFGACCOUNT - url = %s\n", pData);

    if ((pData == NULL) || (strlen(pData) == 0))
    {
      return FALSE;
    }

    ret = amsUtils_readConfigData(&cfgData);

    if (ret)
    {
        sprintf(content, "%s|%s|%s|%s", cfgData->addr, cfgData->port, cfgData->initData, pData);
        ret = amsUtils_writeConfigData(content);
        amsUtils_releaseConfigData(&cfgData);
    }
    else
    {
        sprintf(content, "%s|%s|%s|%s", DEFAULT_SERVER, DEFAULT_PORT, "s:0", pData);
        ret = amsUtils_writeConfigData(content);
    }
#endif
    return ret;
}

/**
 * @brief config server and port in config file
 * @param pData char* should be formatted as "<server>|<port>"
 *
 * @return bool_t TRUE while successfully, otherwise FALSE
 */
bool_t amsUtils_configAddress(const char* pData)
{
    bool_t ret = FALSE;
#if defined(ARCH_ARM_SPD)
    char content[MAX_PATH_LENGTH] = {0};
    RMTConfig *cfgData = NULL;
    DthingTraceD("=== ReceiveRemoteCmd CMD_CFGURL - account = %s", pData);
    ret = amsUtils_readConfigData(&cfgData);

    if (ret)
    {
        sprintf(content, "%s|%s|%s|%s", pData, cfgData->initData, cfgData->user, cfgData->pwd);
        ret = amsUtils_writeConfigData(content);
        amsUtils_releaseConfigData(&cfgData);
    }
    else
    {
        sprintf(content, "%s|%s|%s|%s", pData, "s:0", DEFAULT_USER_NAME, DEFAULT_PASSWORD);
        ret = amsUtils_writeConfigData(content);
    }
#endif
    return ret;
}