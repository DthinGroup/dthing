#ifndef __VMTIME_H__
#define __VMTIME_H__

#include <vm_common.h>

#if defined(ARCH_X86)
#include <Windows.h>
#endif

#define SCHD_OpenTIMER	vmtime_startTimer
#define SCHD_StopTIMER	vmtime_stopTimer


void vmtime_init(void);
void vmtime_term(void);
uint64_t vmtime_getTickCount(void);

void vmtime_startTimer(void);
void vmtime_stopTimer(void);

#endif
