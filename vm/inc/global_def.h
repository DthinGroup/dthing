
#ifndef __GLOBAL_DEF_H__
#define __GLOBAL_DEF_H__


typedef enum
{
    ARCH_X86,
    ARCH_ARM_SPD
}ARCH_E;

//Keep ARM relase as default
#define ARCH_X86

#ifndef ARCH_X86 
#define ARCH_ARM_SPD
#endif

//#define NOT_LAUNCH_NET_TASK

#define INLINE

// MSM_WIS_DEBUG this macro define wis_debug  for qijun board, if longshang cannot use this macro, default was not defined
#define MSM_WIS_DEBUG

//For qijun board, GPS is treated as common com, just pull GPIO60 to high before access GPS
//For longshang 8800 board, should enable this feature.
//For other gps, need drive gps in spd gps module first, then enable this feature
#undef SUPPORT_GPS_BY_MODULE

#define _TEST_ED_    //test edward code of classloader



#endif /*__GLOBAL_DEF_H__*/