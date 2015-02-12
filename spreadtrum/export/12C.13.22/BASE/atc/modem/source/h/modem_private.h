#ifndef MODEM_PRIVATE_H
#define MODEM_PRIVATE_H

#include <stdio.h>
#include <string.h>

#include "modem_public.h"

#include "os_api.h"
#include "Priority_app.h"
#include "sfs.h"
#include "sci_types.h"

/**
 * standard zmodem file protocol macros
 */
#define ZPAD '*'  /* 052 Padding character begins frames */
#define ZDLE 030  /* Ctrl-X Zmodem escape - `ala BISYNC DLE */
#define ZDLEE (ZDLE^0100) /* Escaped ZDLE as transmitted */
#define ZBIN 'A'  /* Binary frame indicator */
#define ZHEX 'B'  /* HEX frame indicator */
#define ZBIN32 'C'  /* Binary frame with 32 bit FCS */

/* Frame types (see array "frametypes" in zm.c) */
#define ZRQINIT 0 /* Request receive init */
#define ZRINIT  1 /* Receive init */
#define ZSINIT 2  /* Send init sequence (optional) */
#define ZACK 3    /* ACK to above */
#define ZFILE 4   /* File name from sender */
#define ZSKIP 5   /* To sender: skip this file */
#define ZNAK 6    /* Last packet was garbled */
#define ZABORT 7  /* Abort batch transfers */
#define ZFIN 8    /* Finish session */
#define ZRPOS 9   /* Resume data trans at this position */
#define ZDATA 10  /* Data packet(s) follow */
#define ZEOF 11   /* End of file */
#define ZFERR 12  /* Fatal Read or Write error Detected */
#define ZCRC 13   /* Request for file CRC and response */
#define ZCHALLENGE 14 /* Receiver's Challenge */
#define ZCOMPL 15 /* Request is complete */
#define ZCAN 16   /* Other end canned session with CAN*5 */
#define ZFREECNT 17 /* Request for free bytes on filesystem */
#define ZCOMMAND 18 /* Command from sending program */
#define ZSTDERR 19  /* Output to standard error, data follows */

/* ZDLE sequences */
#define ZCRCE 'h' /* CRC next, frame ends, header packet follows */
#define ZCRCG 'i' /* CRC next, frame continues nonstop */
#define ZCRCQ 'j' /* CRC next, frame continues, ZACK expected */
#define ZCRCW 'k' /* CRC next, ZACK expected, end of frame */
#define ZRUB0 'l' /* Translate to rubout 0177 */
#define ZRUB1 'm' /* Translate to rubout 0377 */

/* zdlread return values (internal) */
/* -1 is general error, -2 is timeout */
#define GOTOR 0400
#define GOTCRCE (ZCRCE|GOTOR) /* ZDLE-ZCRCE received */
#define GOTCRCG (ZCRCG|GOTOR) /* ZDLE-ZCRCG received */
#define GOTCRCQ (ZCRCQ|GOTOR) /* ZDLE-ZCRCQ received */
#define GOTCRCW (ZCRCW|GOTOR) /* ZDLE-ZCRCW received */
#define GOTCAN  (GOTOR|030) /* CAN*5 seen */

/* Byte positions within header array */
#define ZF0 3 /* First flags byte */
#define ZF1 2
#define ZF2 1
#define ZF3 0
#define ZP0 0 /* Low order 8 bits of position */
#define ZP1 1
#define ZP2 2
#define ZP3 3 /* High order 8 bits of file position */

/* Bit Masks for ZRINIT flags byte ZF0 */
#define CANFDX  0x01  /* Rx can send and receive true FDX */
#define CANOVIO 0x02  /* Rx can receive data during disk I/O */
#define CANBRK  0x04  /* Rx can send a break signal */
#define CANCRY  0x08  /* Receiver can decrypt */
#define CANLZW  0x10  /* Receiver can uncompress */
#define CANFC32 0x20  /* Receiver can use 32 bit Frame Check */
#define ESCCTL  0x40  /* Receiver expects ctl chars to be escaped */
#define ESC8    0x80  /* Receiver expects 8th bit to be escaped */
/* Bit Masks for ZRINIT flags byze ZF1 */
#define ZF1_CANVHDR  0x01  /* Variable headers OK, unused in lrzsz */
#define ZF1_TIMESYNC 0x02 /* nonstandard, Receiver request timesync */

/* Parameters for ZSINIT frame */
#define ZATTNLEN 32 /* Max length of attention string */
/* Bit Masks for ZSINIT flags byte ZF0 */
#define TESCCTL 0100  /* Transmitter expects ctl chars to be escaped */
#define TESC8   0200  /* Transmitter expects 8th bit to be escaped */

/* Parameters for ZFILE frame */
/* Conversion options one of these in ZF0 */
#define ZCBIN 1 /* Binary transfer - inhibit conversion */
#define ZCNL  2 /* Convert NL to local end of line convention */
#define ZCRESUM 3 /* Resume interrupted file transfer */
/* Management include options, one of these ored in ZF1 */
#define ZF1_ZMSKNOLOC   0x80 /* Skip file if not present at rx */
/* Management options, one of these ored in ZF1 */
#define ZF1_ZMMASK      0x1f /* Mask for the choices below */
#define ZF1_ZMNEWL         1 /* Transfer if source newer or longer */
#define ZF1_ZMCRC          2 /* Transfer if different file CRC or length */
#define ZF1_ZMAPND         3 /* Append contents to existing file (if any) */
#define ZF1_ZMCLOB         4 /* Replace existing file */
#define ZF1_ZMNEW          5 /* Transfer if source newer */
  /* Number 5 is alive ... */
#define ZF1_ZMDIFF         6 /* Transfer if dates or lengths different */
#define ZF1_ZMPROT         7 /* Protect destination file */
#define ZF1_ZMCHNG         8 /* Change filename if destination exists */

/* Transport options, one of these in ZF2 */
#define ZTLZW 1 /* Lempel-Ziv compression */
#define ZTCRYPT 2 /* Encryption */
#define ZTRLE 3 /* Run Length encoding */
/* Extended options for ZF3, bit encoded */
#define ZXSPARS 64  /* Encoding for sparse file operations */

/* Parameters for ZCOMMAND frame ZF0 (otherwise 0) */
#define ZCACK1  1 /* Acknowledge, then do command */

/* Default XModem Control Flag */
#define XSOH 0x01
#define XSTX 0x02
#define XEOT 0x04
#define XACK 0x06
#define XNAK 0x15
#define XCAN 0x18
#define XCTRLZ 0x1A

#define ZFLAG_RZ "rz"
#define ZFLAG_SZ "sz"
#define YCRC 'C'

#define MAX_FILENAME_LEN 128
#define MAX_BUFFER_LEN 2048
#define MAX_PACKAGE_SIZE_128 128
#define MAX_PACKAGE_SIZE_1024 1024

#if 0
#define MDEBUG SCI_TRACE_LOW
#else
#define MDEBUG //
#endif

#define MLOG SCI_TRACE_LOW

typedef enum
{
  MODE_DEFAULT = 0,
  MODE_CRC
} WaitingMode;

typedef enum
{
  SEND_REQUEST = 0,
  SEND_FILENAME_REQUEST,
  SEND_FILELEN_REQUEST,
  SEND_CRC_REQUEST,
  SEND_DATA_REQUEST,
  ERROR_SEND_DATA_REQUEST,
  SEND_OVER_REQUEST,
  OTHER_ERROR_REQUEST,
  OTHER_REQUEST
} SendRequestType;

typedef enum
{
  MSTATE_INIT = 0,       // no NAK or C request(default)
  MSTATE_WAIT,           // send NAK or C and wait for file transfer
  MSTATE_READY,          // received SOH
  MSTATE_PREVERIFY,      // preverify file name crc
  MSTATE_RECEIVING_DATA, // receiving data
  MSTATE_CANCEL,         // canceled by sender
  MSTATE_FINISH,         // finish data transfer
  MSTATE_ERROR,          // any errors
  MSTATE_END             // end
} ModemProtocolState;

typedef enum
{
  MTYPE_UNKNOWN = 0,
  MTYPE_XMODEM,
  MTYPE_YMODEM,
  MTYPE_YMODEM_G,
  MTYPE_ZMODEM
} ModemProtocolType;

/**
 * own defined struct
 */
typedef struct ModemRequest
{
  int linkid;
  int handle;
  int blockid;
  int bufpos; //point to position not parsed
  int buflen; //total data length in buffer
  char buf[MAX_BUFFER_LEN];
  char filename[MAX_FILENAME_LEN];
  char filelen[8];
  int expectlen;
  int writelen;
  SendRequestType type;
  ModemProtocolState state;
  ModemProtocolType protocol;
  int crc[2];
  WaitingMode mode;
} ModemRequest;

typedef struct ModemResponse
{
  int handle;
  int type;
  int length;
  char* data;
} ModemResponse;

typedef enum
{
  MRESULT_SUCCESS = 0,
  MRESULT_BLOCK_SUCCESS,
  MRESULT_ERROR,
  MRESULT_ERROR_WRITE_FILE,
  MRESULT_RETRY,
  MRESULT_PENDING
} ModemResult;

int modem_at_response(void* atc_config_ptr, unsigned char flag, ATResponseCB cb);

/* modem control api */
int modem_control_main(void* atc_config_ptr, const char* data, int datalen, int linkid, ATResponseCB cb);
void ATC_StartFileListener();
void ATC_StopFileListener();

/* modem parser api */
int modem_parse_data(ModemRequest* request);
int modem_parse_header(ModemRequest* request);
//return offset of SOH, -1 means no found
int modem_check_header(ModemRequest* request, const char* data, int datalen);
int modem_check_cancel(const char* data, int datalen);
int modem_check_end(const char* data, int datalen);
int modem_preverify_filename(ModemRequest* request);

/* modem composer api */
int modem_composer_response(const char* data, ModemResponse* response);

/* modem preverify api */
int modem_preverify_data(const char* data);

/* modem file api */
int modem_fs_open_file(const char* fname);
int modem_fs_write_file(int handle, char* data, int len);
int modem_fs_append_file(int handle, char* data, int len);
int modem_fs_close_file(int handle);
int modem_fs_delete_file(const char* fname);

#endif //MODEM_PRIVATE_H