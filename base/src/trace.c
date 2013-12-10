#include <stdio.h>
#include <stdarg.h>

#include "vm_common.h"
#include "trace.h"
#include "crtl.h"

#define MSG_BUFFER_LEN  256

void DthingTraceE(const char * fmt,...)
{
    char msg_buffer[MSG_BUFFER_LEN + 1] = {0};
    va_list arg_list = {0};
    CRTL_memset(msg_buffer, 0, MSG_BUFFER_LEN);
    sprintf(msg_buffer, "[DTHING Error]:");
                
    va_start(arg_list, fmt);/*lint !e718 !e64*/
    _vsnprintf(msg_buffer+CRTL_strlen(msg_buffer),MSG_BUFFER_LEN - strlen(msg_buffer), fmt, arg_list);

#if defined(ARCH_X86)
    printf(msg_buffer);
#elif defined(ARCH_ARM_SPD)
    #if defined(WIN32)
        SCI_Trace(msg_buffer);
    #else
        SCI_TraceLow(msg_buffer);
    #endif    
#else
    #error "unsupport!"
#endif

    va_end(arg_list);    
}

void DthingTraceW(const char * fmt,...)
{
    char msg_buffer[MSG_BUFFER_LEN + 1] = {0};
    va_list arg_list = {0};
    CRTL_memset(msg_buffer, 0, MSG_BUFFER_LEN);
    sprintf(msg_buffer, "[DTHING Warning]:");
                
    va_start(arg_list, fmt);/*lint !e718 !e64*/
    _vsnprintf(msg_buffer+CRTL_strlen(msg_buffer),MSG_BUFFER_LEN - strlen(msg_buffer), fmt, arg_list);

#if defined(ARCH_X86)
    printf(msg_buffer);
#elif defined(ARCH_ARM_SPD)
    #if defined(WIN32)
        SCI_Trace(msg_buffer);
    #else
        SCI_TraceLow(msg_buffer);
    #endif    
#else
    #error "unsupport!"
#endif

    va_end(arg_list);    
}

void DthingTraceD(const char * fmt,...)
{
    char msg_buffer[MSG_BUFFER_LEN + 1] = {0};
    va_list arg_list = {0};
    CRTL_memset(msg_buffer, 0, MSG_BUFFER_LEN);
    sprintf(msg_buffer, "[DTHING Debug]:");
                
    va_start(arg_list, fmt);/*lint !e718 !e64*/
    _vsnprintf(msg_buffer+CRTL_strlen(msg_buffer),MSG_BUFFER_LEN - strlen(msg_buffer), fmt, arg_list);

#if defined(ARCH_X86)
    printf(msg_buffer);
#elif defined(ARCH_ARM_SPD)
    #if defined(WIN32)
        SCI_Trace(msg_buffer);
    #else
        SCI_TraceLow(msg_buffer);
    #endif    
#else
    #error "unsupport!"
#endif

    va_end(arg_list);    
}

void DthingTraceI(const char * fmt,...)
{
    char msg_buffer[MSG_BUFFER_LEN + 1] = {0};
    va_list arg_list = {0};
    CRTL_memset(msg_buffer, 0, MSG_BUFFER_LEN);
    sprintf(msg_buffer, "[DTHING Info]:");
                
    va_start(arg_list, fmt);/*lint !e718 !e64*/
    _vsnprintf(msg_buffer+CRTL_strlen(msg_buffer),MSG_BUFFER_LEN - strlen(msg_buffer), fmt, arg_list);

#if defined(ARCH_X86)
    printf(msg_buffer);
#elif defined(ARCH_ARM_SPD)
    #if defined(WIN32)
        SCI_Trace(msg_buffer);
    #else
        SCI_TraceLow(msg_buffer);
    #endif    
#else
    #error "unsupport!"
#endif

    va_end(arg_list);    
}