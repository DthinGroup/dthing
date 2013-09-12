#ifndef __VMTIME_H__
#define __VMTIME_H__

#include <vm_common.h>

#ifdef ARCH_X86
#define SCHD_OpenTIMER	vmtime_startTimer
#define SCHD_StopTIMER	vmtime_stopTimer
#else

#endif

void vmtime_init(void);
void vmtime_term(void);
u8   vmtime_getTickCount(void);

#ifdef ARCH_X86
void vmtime_startTimer(void);
void vmtime_stopTimer(void);
#endif

#endif
