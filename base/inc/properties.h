#ifndef __DVM_PROPERTIES_H__
#define __DVM_PROPERTIES_H__

#include <std_global.h>

#ifdef __cplusplus
extern "C" {
#endif

void props_startup();

void props_shutdown();

const char* props_getValue(const char* key);

const char* props_setValue(const char* key, const char* val);

#ifdef __cplusplus
}
#endif

#endif //__DVM_PROPERTIES_H__