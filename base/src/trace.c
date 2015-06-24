#include <stdio.h>
#include <stdarg.h>

#include "vm_common.h"
#include "trace.h"
#include "crtl.h"
//#include "os_api.h"

#define MSG_BUFFER_LEN  255

static unsigned int trace_level = TRACE_LEV;

int SetDthingTraceLevel(unsigned int level)
{
  // return 0 or negative means failure, others success
  if(level<5) 
  {
     trace_level = level;
     return 1;
  }
  return 0;
}

void DthingTrace(const char* tag, const char* fmt, va_list argList)
{
    char msgBuf[MSG_BUFFER_LEN + 1] = {0};
    int len = 0;

    CRTL_memset(msgBuf, 0, sizeof(msgBuf));
    CRTL_sprintf(msgBuf, tag);
    len = CRTL_strlen(msgBuf);
    _vsnprintf(msgBuf + len, MSG_BUFFER_LEN - len, fmt, argList);

#if defined(ARCH_X86)
    CRTL_printf(msgBuf);
#elif defined(ARCH_ARM_SPD)
    #if defined(WIN32)
        SCI_Trace(msgBuf);
    #else
        //SCI_TRACE_LOW(msgBuf);
        Dthing_log(msgBuf);
    #endif    
#else
    #error "unsupport!"
#endif
}

void DVMTraceDbg(const char * fmt,...)
{
    va_list argList = {0};
    if(trace_level < LEVEL_3) return;
    va_start(argList, fmt);/*lint !e718 !e64*/
    DthingTrace("DTHING D:", fmt, argList);
    va_end(argList);    
}

void DVMTraceInf(const char * fmt,...)
{
    va_list argList = {0};
    if(trace_level < LEVEL_4) return;
    va_start(argList, fmt);/*lint !e718 !e64*/
    DthingTrace("DTHING I:", fmt, argList);
    va_end(argList);    
}

void DVMTraceWar(const char * fmt,...)
{
    va_list argList = {0};
    if(trace_level < LEVEL_2) return;
    va_start(argList, fmt);/*lint !e718 !e64*/
    DthingTrace("DTHING W:", fmt, argList);
    va_end(argList);    
}

void DVMTraceErr(const char * fmt,...)
{
    va_list argList = {0};
    if(trace_level < LEVEL_1) return;
    va_start(argList, fmt);/*lint !e718 !e64*/
    DthingTrace("DTHING E:", fmt, argList);
    va_end(argList);    
}
