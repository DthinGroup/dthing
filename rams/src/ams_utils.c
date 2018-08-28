#include "vm_common.h"
#include "ams_utils.h"
#include "opl_file.h"
#include "trace.h"
#include "ams.h"
#include "vm_app.h"
#include "string.h"
#include "assert.h"
#include <stdlib.h>

#define MAX_PATH_LENGTH   255
#define MAX_FILE_BUFF_LEN 128
#define DEFAULT_RMT_CONFIG_FILE  L"D:/RemoteConfig.cfg"
#define DEFAULT_RMT_CONFIG_FILE_PATH_LEN 19
#define DEFAULT_SERVER "42.121.18.62"
#define DEFAULT_PORT "10000"
//#define DEFAULT_SERVER "218.206.176.236"
//#define DEFAULT_PORT "7777"
#define DEFAULT_USER_NAME "admin"
#define DEFAULT_PASSWORD "password"
#define DEFAULT_APPNAME "test_app "
#define RCMD_CANCELALL_CFG "rcmd_cancelall_cfg" 

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
    free((char *)*str);
  }

  memcpy(newStr + sLen, append, aLen);
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
  int appId_1= -1;
  int appId_2 = -1;
  AppletProps *pAppProp_1;
  AppletProps *pAppProp_2;

#if defined(ARCH_ARM_SPD)
  char content[MAX_PATH_LENGTH] = {0};
  if (CRTL_strcmp(pInitData,RCMD_CANCELALL_CFG)==0)
  {
       sprintf(content, "%s|%s|s:%s|%s|%s|%s", DEFAULT_SERVER, DEFAULT_PORT,  "-" , DEFAULT_USER_NAME, DEFAULT_PASSWORD,DEFAULT_APPNAME);
       ret = amsUtils_writeConfigData(content);
       return ret;
  }
  sscanf(pInitData, "%d,%d", &appId_1,&appId_2);
   if(appId_1 == appId_2){
	appId_2 = -1;
   }
   vm_getCurApplist(TRUE); 
   pAppProp_1= getAppletPropById(appId_1);
   pAppProp_2 = getAppletPropById(appId_2);
  	if(pAppProp_1 != NULL){
		ret = initAndCheckCfgDatas(pAppProp_1->frealName);
	}
	if(pAppProp_2 != NULL){
		ret = initAndCheckCfgDatas(pAppProp_2->frealName);
	}
#endif

  return ret;
}

/*when delete a app ,you should check the configdata about this app*/
bool_t amsUtils_checkConfigData(char* name)
{
  bool_t ret = FALSE;
  int appId= -1;
  AppletProps *pAppProp;
  char ** appNames[MAX_APPS_NUM];
  char *defaultSplitChar = "#";
  char * buffer;

#if defined(ARCH_ARM_SPD)
  char content[MAX_PATH_LENGTH] = {0};
  bool_t needCleanInitData = FALSE;
  RMTConfig *cfgData = NULL;
  bool_t needCleanInitName = FALSE;

  memset(content, 0x0, MAX_PATH_LENGTH);
  ret = amsUtils_readConfigData(&cfgData);

  if (ret && (name != NULL))
  {
	int i = 0;
	int num = -1;
	int nums = 0;
	buffer = malloc(strlen(cfgData->appName)+1);
                     memset(buffer, 0x0, strlen(cfgData->appName)+1);
			strcpy(buffer,cfgData->appName);
       amsRemote_split(appNames, cfgData->appName, defaultSplitChar);
	for (i=0; i < MAX_APPS_NUM; i++)
	{
		if (getAppletPropByName(appNames[i]) == NULL){
			nums = i;
  			break;
		}	       	
		 if(strcmp(name,appNames[i])==0){
			num = i;
		}
	}
	if(num > -1){
		 char *appNameChar = NULL;	
		 if(num > 0){
			appNameChar = amsUtils_join(defaultSplitChar, appNames[num]);
		 }else{
		       if(nums > 1){
			       appNameChar = amsUtils_join(appNames[num],defaultSplitChar);
			   }else{		     
			       appNameChar = appNames[num];
			   }			
		 }
		amsUtils_del(buffer,appNameChar);
		if(buffer == NULL || strlen(buffer) == 0){
			needCleanInitName = TRUE;
			needCleanInitData = TRUE;
		}
		sprintf(content, "%s|%s|%s|%s|%s|%s", DEFAULT_SERVER, DEFAULT_PORT, needCleanInitData? "s:-" : cfgData->initData, 
       	DEFAULT_USER_NAME, DEFAULT_PASSWORD,needCleanInitName? DEFAULT_APPNAME:buffer);
      	ret = amsUtils_writeConfigData(content);
		amsUtils_releaseConfigData(&cfgData);	
		free(buffer);
		
	}else{
		
	}
			
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

  //DVMTraceErr("==RMT== amsUtils_readConfigData()_test_log_2_defined(ARCH_ARM_SPD)");
  //to parse remoteconfig.txt
  result = file_open(DEFAULT_RMT_CONFIG_FILE, DEFAULT_RMT_CONFIG_FILE_PATH_LEN, FILE_MODE_RD, &sfsHandle);

  if(sfsHandle != INVALID_HANDLE_VALUE)
  {
    //DVMTraceErr("==RMT== amsUtils_readConfigData() get 000000000 file");
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
      char appName[128] = {0};

      //TODO: Check if initData is NULL, how much params would be returned by sscanf, 4 or 5
      if (sscanf(content, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^|]", addr, port, initData, user, pwd,appName) < 4)
      {
        DVMTraceErr("==RMT== amsUtils_readConfigData() error data format %s in file", content);
        goto end;
      }

      config = malloc(sizeof(RMTConfig));
      memset(config, 0x0, sizeof(RMTConfig));

      config->addr = amsUtils_strdup(addr);
      config->port = amsUtils_strdup(port);
      config->initData = amsUtils_strdup(initData);
      config->user = amsUtils_strdup(user);
      config->pwd = amsUtils_strdup(pwd);
	  config->appName = amsUtils_strdup(appName);

	  DVMTraceInf(">>>>> Server - %s:%s ",       config->addr, config->port);
	//  	  DVMTraceInf(">>>>> Server port:%s ",       config->port);
	  //DVMTraceInf(">>>>> Server data:%s ",       config->initData);
	  //DVMTraceInf(">>>>> Server user:%s ",       config->user);
	  //DVMTraceInf(">>>>> Server pwd :%s ",       config->pwd);
	  //DVMTraceInf(">>>>> Server app :%s ",       config->appName);
	  
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
    DVMTraceDbg("==RMT== amsUtils_writeConfigData() write config file failed(null cfg)");
    return ret;
  }

  //DeleteFile first then we can create new one
  result = file_delete(DEFAULT_RMT_CONFIG_FILE, DEFAULT_RMT_CONFIG_FILE_PATH_LEN);
  //to parse remoteconfig.cfg
  result = file_open(DEFAULT_RMT_CONFIG_FILE, DEFAULT_RMT_CONFIG_FILE_PATH_LEN, FILE_MODE_RDWR, &sfsHandle);
  if(sfsHandle != INVALID_HANDLE_VALUE)
  {
 	file_writeLen = file_write(sfsHandle, cfg, strlen(cfg));
    if(file_writeLen > 0)
    {
      //DVMTraceDbg("==RMT== amsUtils_writeConfigData() write config file success");
      ret = TRUE;
    }
    else
    {
      DVMTraceDbg("==RMT== amsUtils_writeConfigData() write config file failed(negative len)");
    }
    file_close(sfsHandle);
  }
  else
  {
    DVMTraceDbg("==RMT== amsUtils_writeConfigData() create config file failed(invalid sfs handle)");
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
      if (cfg->appName)
      {
        free(cfg->appName);
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
  char app[255] = {0};
  char *plist = NULL;
  char *temp = NULL;
  int alen = 0;
  int llen = 0;
  AppletProps *activeApp = NULL;
  AppletProps *header = NULL;

#if defined(ARCH_ARM_SPD)
  AppletProps *curApp = NULL;
  //TODISCUSS: Seems not the best way to get applist. This implementation is
  //           better to put into vm_apps in order to be called by other modules.
  curApp = vm_getCurApplist(TRUE);
  //AppletProps *curApp = vm_getCurApplist(FALSE);
  while(curApp != NULL)
  {
    if ((curApp->isRunning || (curApp->isRunning == isRunning)) && (curApp->id != PROPS_UNUSED))
    {
      memset(app, 0x0, 255);
      sprintf(app, "%d %s\r\n", curApp->id, curApp->name);
      alen = strlen(app);
      llen = (plist == NULL)? 0 : strlen(plist);
      temp = malloc(alen + llen + 1);
      memset(temp, 0x0, alen + llen + 1);	  
      memcpy(temp, plist, llen);
      memcpy(temp + llen, app, alen);
      free(plist);
      plist = temp;
    }
    curApp = curApp->nextRunning;
  }

  if (NULL == plist)
  {
    plist = amsUtils_strdup(isRunning? "No Active Applet" : "No Applet");
  }

#endif
  return plist;
}

/**
 * @brief cancel specified applets from auto-start applet list in config file
 * @param pData char* should be formatted as "<suiteId>" or "<suiteId-1>,<suiteId-2>"
 *
 * @return bool_t TRUE while successfully, otherwise FALSE
 */
bool_t amsUtils_cancelDefaultApp(const int pData)
{
    bool_t ret = FALSE;
#if defined(ARCH_ARM_SPD)
    char init1[16] = {0};
    char init2[16] = {0};
    RMTConfig *cfgData = NULL;
    char content[MAX_PATH_LENGTH] = {0};
    AppletProps *curApp;

   /* if ((pData == NULL) || (pData < 0))
    {
       DVMTraceErr("=== RemoteCmd CMD_CANCEL - data = err\n");
        return FALSE;
    }*/
  
	
     if((curApp = getAppletPropById(pData)) == NULL){
	   	return FALSE;
	 }else{
		ret= amsUtils_checkConfigData(curApp->frealName);
	 }
   

    if (ret)
    {
      /*  sscanf(cfgData->initData, "%*[s:]%[^,],%s", init1, init2);

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
        ret = amsUtils_initConfigData(content);*/
	
	  
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
    if ((pData == NULL) || (strlen(pData) == 0))
    {
      return FALSE;
    }

    ret = amsUtils_readConfigData(&cfgData);

    if (ret)
    {
        sprintf(content, "%s|%s|%s|%s|%s", cfgData->addr, cfgData->port, cfgData->initData, pData,cfgData->appName);
        ret = amsUtils_writeConfigData(content);
        amsUtils_releaseConfigData(&cfgData);
    }
    else
    {
        sprintf(content, "%s|%s|%s|%s|%s", DEFAULT_SERVER, DEFAULT_PORT, "s:-", pData,DEFAULT_APPNAME);
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
    ret = amsUtils_readConfigData(&cfgData);

    if (ret)
    {
        sprintf(content, "%s|%s|%s|%s|%s", pData, cfgData->initData, cfgData->user, cfgData->pwd,cfgData->appName);
        ret = amsUtils_writeConfigData(content);
        amsUtils_releaseConfigData(&cfgData);
    }
    else
    {
        sprintf(content, "%s|%s|%s|%s|%s", pData, "s:-", DEFAULT_USER_NAME, DEFAULT_PASSWORD,DEFAULT_APPNAME);
        ret = amsUtils_writeConfigData(content);
    }
#endif
    return ret;
}
void amsUtils_split( char **arr, char *str, const char *del)
{
 char *s =NULL; 

 s=strtok(str,del);
 while(s != NULL)
 {
  *arr++ = s;
  s = strtok(NULL,del);
 }
}
char* amsUtils_join(char *s1, char *s2)  
{  
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator  
    //in real code you would check for errors in malloc here  
    if (result == NULL) exit (1);  
  
    strcpy(result, s1);  
    strcat(result, s2);  
  
    return result;  
}  
int amsUtils_del(char* str,const char* sub)
{
    size_t i = 0; 
    size_t str_len = strlen(str); 
    size_t sub_len = strlen(sub); 
    while((str_len-i) >= sub_len) 
    {
             if(memcmp(str+i, sub, sub_len) == 0){
           strcpy(str+i, str+i+sub_len); 
           str_len -= sub_len; 
             }else {
                   ++i; 
          }
      }
	return 0;
} 
bool_t initAndCheckCfgDatas(char *pAppPropName){
	 RMTConfig *cfgData = NULL;
	 char content[MAX_PATH_LENGTH] = {0};
	 bool_t ret = FALSE;
	 bool_t IsInitNameExist = FALSE;
	 char ** appNames[MAX_APPS_NUM];
	 char *defaultSplitChar = "#";
        char *appNamesBuffer;
	AppletProps *pAppProp = NULL;
	
	pAppProp = getAppletPropByName(pAppPropName);
	 ret = amsUtils_readConfigData(&cfgData);
	 if(ret){
		if(strcmp(cfgData->appName,DEFAULT_APPNAME) == 0 ){
			sprintf(content, "%s|%s|s:%d|%s|%s|%s", DEFAULT_SERVER, DEFAULT_PORT,pAppProp->id, 
        			DEFAULT_USER_NAME, DEFAULT_PASSWORD,pAppProp->frealName);
			 ret = amsUtils_writeConfigData(content);
        		  amsUtils_releaseConfigData(&cfgData);
		}else{
			int i = 0;
			memset(appNames, 0x0, sizeof(appNames));
			appNamesBuffer = malloc(strlen(cfgData->appName)+1); 
		    if (appNamesBuffer == NULL) exit (1);  
		    strcpy(appNamesBuffer, cfgData->appName);  
			amsUtils_split(appNames, cfgData->appName, defaultSplitChar);
			for (i=0; i<MAX_APPS_NUM; i++)
			{
				if(appNames[i] == NULL){
					break;
				}
				if((strcmp(pAppProp->frealName,appNames[i])== 0) ){
					IsInitNameExist = TRUE;
					DVMTraceErr("==DVM== This app has been set to the start!");
					break;
				}
			}
			cfgData->appName = appNamesBuffer;
			if(!IsInitNameExist){
				char* appNamesChar=NULL;			
				appNamesChar = amsUtils_join(defaultSplitChar, pAppProp->frealName);
				appNamesChar = amsUtils_join(cfgData->appName , appNamesChar);
				cfgData->appName = appNamesChar;
				sprintf(content, "%s|%s|s:%d|%s|%s|%s", DEFAULT_SERVER, DEFAULT_PORT,pAppProp->id, 
        		DEFAULT_USER_NAME, DEFAULT_PASSWORD,cfgData->appName);
				ret = amsUtils_writeConfigData(content);
        		amsUtils_releaseConfigData(&cfgData);
				IsInitNameExist = TRUE;
				free(appNamesBuffer);
			}
		}
	 }else{
		sprintf(content, "%s|%s|s:%d|%s|%s|%s", DEFAULT_SERVER, DEFAULT_PORT,pAppProp->id, 
        DEFAULT_USER_NAME, DEFAULT_PASSWORD,pAppProp->frealName);
		ret = amsUtils_writeConfigData(content);
        amsUtils_releaseConfigData(&cfgData);
	 }
       
	return ret;	 
}

static bool_t 		gRemoteServerLoaded = FALSE;
static uint32_t  	gRemoteServerIp  	= 0x00;
static uint16_t  	gRemoteServerPort  	= 0x00;
static uint8_t		gRemoteUsername[32];
static uint8_t		gRemotePassword[32];

char * amsUtils_getRemoteUsername(void){
	if(gRemoteServerLoaded){
		return &gRemoteUsername[0];
	} else {
		return NULL;
	}
}

char * amsUtils_getRemotePassword(void){
	if(gRemoteServerLoaded){
		return &gRemotePassword[0];
	} else {
		return NULL;
	}
}

bool_t amsUtils_loadRemoteServerAndPort(uint32_t * ip, uint16_t * port){
	bool_t ret;
	RMTConfig *cfgData = NULL;	
	int res,ip1,ip2,ip3,ip4;
	

	assert(ip != NULL);
	assert(port != NULL);	


	if(gRemoteServerLoaded && (gRemoteServerIp !=0) && (gRemoteServerPort != 0x00)){
		*ip = gRemoteServerIp;
		*port = gRemoteServerPort;

		return TRUE;
	}

	
	ret = amsUtils_readConfigData(&cfgData);

	if(ret != TRUE){
		DVMTraceWar(">>>>>loadRemoteServerAndPort: read data fail...");
		return FALSE;
	}

	res = sscanf(cfgData->addr, "%d.%d.%d.%d", &ip1,&ip2,&ip3,&ip4);

	if(res != 4){
		DVMTraceWar(">>>>>loadRemoteServerAndPort: invalid ip format...");		
		return FALSE;
	}

	if((ip1 < 0 || ip1 > 255) || 
	   (ip2 < 0 || ip2 > 255) || 
	   (ip3 < 0 || ip3 > 255) || 
	   (ip4 < 0 || ip4 > 255) 
	){
		DVMTraceWar(">>>>>loadRemoteServerAndPort: invalid ip value...");			
		return FALSE;
	}

	*ip = (uint32_t) ( (ip1 << 24) | (ip2 <<16) | (ip3 << 8) | ip4 );

	
	res = sscanf(cfgData->port, "%d", &ip1);

	if(res != 1 || ip1 < 0 || ip1 > 65535){
		DVMTraceWar(">>>>>loadRemoteServerAndPort: invalid port format...");				
		return FALSE;
	}
	*port = (uint16_t) ip1;



	CRTL_memset(&gRemoteUsername[0],0,32);
	CRTL_memset(&gRemotePassword[0],0,32);

	sscanf(cfgData->user, "%s", &gRemoteUsername[0]);
	sscanf(cfgData->pwd , "%s", &gRemotePassword[0]);	

	{
		gRemoteServerLoaded = TRUE;
		gRemoteServerIp		= *ip;
		gRemoteServerPort	= *port; 
	}

	DVMTraceWar(">>>>>loadRemoteServer: ip:%d, port:%d, user:%s, pwd:%s ...", gRemoteServerIp, gRemoteServerPort, &gRemoteUsername[0], &gRemotePassword[0]);				

	

	return TRUE;
}

  