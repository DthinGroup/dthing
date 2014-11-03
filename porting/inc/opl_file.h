/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/06/25 $
 * Last modified:	$Date: 2013/06/28 $
 * Version:         $ID: opl_file.h#1
 */


/**
 * This file implement the File porting on WIN32 platform.
 * API abstract is likely standard C file APIs.
 */

#ifndef __OPL_FILE_H__
#define __OPL_FILE_H__

#if defined(__cplusplus)
extern "C" {
#endif

/* file operation success */
#define FILE_RES_SUCCESS  0
/* file operation failure */
#define FILE_RES_FAILURE  -1
/*invalid file handle*/
#define INVALID_HANDLE_VALUE -1

/** Type value of a regular file or file name. */
#define FILE_RES_ISREG    -2
/** Type value of a directory or directory name. */
#define FILE_RES_ISDIR    -3

/** Indicates that a file_read() call reached the end of the file.*/
#define FILE_RES_EOF      -4


/** file pointer whence definitions */
#define FILE_SEEK_BEGIN   0
#define FILE_SEEK_CURRENT 1
#define FILE_SEEK_END     2


/* file open mode */
#define FILE_MODE_RD  1
#define FILE_MODE_WR  2
#define FILE_MODE_RDWR  (FILE_MODE_RD | FILE_MODE_WR)


/* maxinum file name length */
#define MAX_FILE_NAME_LEN  (256)


void file_startup();
void file_shutdown(); 

int64_t file_storageSize(const uint16_t* name, int32_t nameLen);

int64_t file_freeSize(const uint16_t* name, int32_t nameLen);

int32_t file_getLengthByName(const uint16_t* name, int32_t nameLen);

int32_t file_listOpen(const uint16_t* prefix, int32_t prefixLen, int32_t* session);

int32_t file_listNextEntry(const uint16_t* prefix, int32_t prefixLen, uint16_t* entry, int32_t entrySize, int32_t* session);

int32_t file_listclose(int32_t* session);

int32_t file_rename(const uint16_t* oldName, int32_t oldNameLen, const uint16_t* newName, int32_t newNameLen);

int32_t file_exists(const uint16_t* name, int32_t nameLen);

int32_t file_open(const uint16_t* name, int32_t nameLen, int32_t mod, int32_t *handle);

int32_t file_delete(const uint16_t* name, int32_t nameLen);

int32_t file_read(int32_t handle, char * readBuf, int32_t bufSize);

int32_t file_write(int32_t handle, char * writeBuf, int32_t bufSize);

int32_t file_truncate(int32_t handle, int32_t value);

int32_t file_getsize(int32_t handle,int32_t * size);

int32_t file_seekex(int32_t handle, int32_t value, int32_t whence);

int32_t file_seek(int32_t handle, int32_t value);

int32_t file_getLengthByFd(int32_t handle);

int32_t file_flush(int32_t handle);

int32_t file_close(int32_t handle);

int32_t file_mkdir(const uint16_t* name, int32_t nameLen);

uint16_t * file_getDthingWDir();

bool_t file_isFSRegistered(void);


#if defined(__cplusplus)
}
#endif

#endif //__OPL_FILE_H__
