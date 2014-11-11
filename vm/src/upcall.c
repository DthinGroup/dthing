/**
 * Copyright (C) 2014 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2014/04/16 $
 * Last modified:	$Date: 2014/04/16 $
 * Version:         $ID: upcall.c#1
 */

/**
 * up call.
 */


#include <std_global.h>
#include <dthing.h>
#include <class.h>
#include <ams_remote.h>
#include <upcall.h>

static Object* curActiveApplet;

/* upcall.h */
void saveCurrentActiveApplet(Object* obj)
{
    curActiveApplet = obj;
}

static uint8_t* formatClazzSignature(uint8_t* sig)
{
    int32_t i, j;
    int32_t len = CRTL_strlen(sig);
    static uint8_t formatStr[64] = {0x0,};
    
    formatStr[0] = 'L';
    for (i = 0, j = 1; i < len; i++, j++)
    {
        if (sig[i] == '.') {
            formatStr[j] = '/';
            continue;
        }
        formatStr[j] = sig[i];
    }
    formatStr[j] = ';';

    return formatStr;
}

/* upcall.h */
bool_t upcallDestroyApplet(AppletProps *pap)
{
    ClassObject* clsObj = NULL;
    Method* destroyMeth = NULL;
    
    if (pap == NULL)
    {
        DVMTraceWar("upcallDestroyApplet pap is null\n");
        return FALSE;
    }
    clsObj = dvmFindClass(formatClazzSignature(pap->mainCls));
    if (clsObj != curActiveApplet->clazz)
    {
        DVMTraceWar("upcallDestroyApplet wrong class object\n");
        return FALSE;
    }

    /* supper hard code "12" again.*/
    destroyMeth = curActiveApplet->clazz->vtable[12];
    dthread_create(destroyMeth, curActiveApplet);
    return TRUE;
}

/* upcall.h */
void upcall_final()
{
    curActiveApplet = NULL;
}



