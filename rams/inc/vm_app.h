#ifndef __VM_APP_H__
#define __VM_APP_H__

#include <std_global.h>
#include <eventbase.h>
#include <ams.h>

#ifdef __cplusplus
extern "C" {
#endif

void SafeBufferFree(void *p);
SafeBuffer* CreateSafeBuffer(SafeBuffer* src);
SafeBuffer* CreateSafeBufferByBin(uint8_t *buf, int32_t buf_size);
int16_t readleIU16(const uint8_t* p);
int32_t readleIU32(const uint8_t* p);
int16_t readbeIU16(const uint8_t* p);
int32_t readbeIU32(const uint8_t* p);
void writebeIU16(uint8_t* p, uint16_t num);
void writebeIU32(uint8_t* p, uint32_t num);
uint8_t* trim(uint8_t* value);
uint8_t* combineAppPath(uint16_t* appName);
uint16_t* getDefaultInstalledPath();
uint16_t* getFileNameFromPath(uint16_t* path);
AppletProps* getAppletPropById(int32_t id);
bool_t parseAppletProps(uint8_t* data, int32_t dataBytes, AppletProps* appProp);

AppletProps* vm_getCurApplist(bool_t refresh);
AppletProps* vm_getCurActiveApp(void);
void vm_clearApplist(void);
void vm_setCurActiveApp(AppletProps * app);
void vm_setCurActiveAppState(bool_t state);

bool_t vm_runApp(int appid);
bool_t vm_deleteApp(int id);
bool_t vm_destroyApp(int id);
bool_t vm_otaApp(char * url);
bool_t vm_tckApp(char * url);

/*ota handle*/
void vm_ota_init();
void vm_ota_final();
void vm_ota_set(bool_t flag,char * url);
bool_t vm_ota_get();
void vm_create_otaTask();

/*tck handle*/
void vm_tck_init();
void vm_tck_final();
void vm_tck_set(bool_t flag, char * url);
bool_t vm_tck_get();
void vm_create_tckTask();

#ifdef __cplusplus
}
#endif

#endif //__AMS_H__