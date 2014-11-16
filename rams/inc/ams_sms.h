#ifndef __AMS_SMS_H__
#define __AMS_SMS_H__

#include <std_global.h>
#include <eventbase.h>
#include <ams.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Bcd Values for Dial Number storage */
#define   DIALBCD_0                 0x0
#define   DIALBCD_1                 0x1
#define   DIALBCD_2                 0x2
#define   DIALBCD_3                 0x3
#define   DIALBCD_4                 0x4
#define   DIALBCD_5                 0x5
#define   DIALBCD_6                 0x6
#define   DIALBCD_7                 0x7
#define   DIALBCD_8                 0x8
#define   DIALBCD_9                 0x9
#define   DIALBCD_STAR              0xA
#define   DIALBCD_HASH              0xB
#define   DIALBCD_PAUSE             0xC
#define   DIALBCD_WILD              0xD
#define   DIALBCD_EXPANSION         0xE
#define   DIALBCD_FILLER            0xF

#define PACKED_LSB_FIRST    0  // LSB first (1234 = 0x21, 0x43)
#define PACKED_MSB_FIRST    1  // MSB first (1234 = 0x12, 0x34)
#define UNPACKED            2  // unpacked  (1234 = 0x01, 0x02, 0x03, 0x04)

/**
*operation result value
*/
#define CMD_RESULT_OK 0
#define CMD_RESULT_FAILED -1

void smsc_registerEvent();
void smsc_callBack(AmsCBData* data);



#ifdef __cplusplus
}
#endif

#endif //__AMS_H__