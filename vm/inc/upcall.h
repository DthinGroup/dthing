
#ifndef __UPCALL_H__
#define __UPCALL_H__

#include <std_global.h>
#include <dthing.h>
#include <class.h>

#ifdef __cplusplus
extern "C" {
#endif

void saveCurrentActiveApplet(Object* obj);

bool_t upcallDestroyApplet(AppletProps *pap);

void upcall_final();

#ifdef __cplusplus
}
#endif

#endif