#include <stdio.h>
#include <stdarg.h>

#include "vm_common.h"
#include "trace.h"
#include "crtl.h"

#define MSG_BUFFER_LEN  255

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
		Dthing_log(msgBuf);
	#endif    
#else
	#error "unsupport!"
#endif
}

void DthingTraceD(const char * fmt,...)
{
	va_list argList = {0};

	va_start(argList, fmt);/*lint !e718 !e64*/
	DthingTrace("[DTHING Debug]:", fmt, argList);
	va_end(argList);    
}

void DthingTraceI(const char * fmt,...)
{
	va_list argList = {0};

	va_start(argList, fmt);/*lint !e718 !e64*/
	DthingTrace("[DTHING Info]:", fmt, argList);
	va_end(argList);    
}

void DthingTraceW(const char * fmt,...)
{
	va_list argList = {0};

	va_start(argList, fmt);/*lint !e718 !e64*/
	DthingTrace("[DTHING Warning]:", fmt, argList);
	va_end(argList);    
}

void DthingTraceE(const char * fmt,...)
{
	va_list argList = {0};

	va_start(argList, fmt);/*lint !e718 !e64*/
	DthingTrace("[DTHING Error]:", fmt, argList);
	va_end(argList);    
}
