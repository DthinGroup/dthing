#ifndef __INIT_H__
#define __INIT_H__

#include "stdint.h"
#include "vm_common.h"

#define NEXT_STATE_NULL 0x00
#define NEXT_STATE_TCK  0x01

int32_t DVM_main(int32_t argc, char * argv[]);

void setNextSchedulerState(int32_t state);

int32_t getNextSchedulerState(void);

#endif