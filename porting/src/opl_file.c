/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/06/25 $
 * Last modified:	$Date: 2013/06/28 $
 * Version:         $ID: opl_file.c#1
 */


/**
 * This file implement the File porting on WIN32 platform.
 * API abstract is likely standard C file APIs.
 */
 #include <vm_common.h>
#include <std_global.h>
#include <opl_file.h>

#if defined(ARCH_X86)
/* platform header files */
#include <windows.h>

#elif defined(ARCH_ARM_SPD)
#include <sfs.h>
#include <os_api.h>
#endif


#ifdef  MAX
#undef  MAX
#endif
#ifdef  MIN
#undef  MIN
#endif
#define MAX(a,b)                        (((a)>(b))?(a):(b))
#define MIN(a,b)                        (((a)<(b))?(a):(b))

#ifdef MAX_FILENAME_LEN
#undef MAX_FILENAME_LEN
#endif

#define MAX_FILENAME_LEN (256)

#if defined(ARCH_X86)
/**
 * Convert a Java string into a nul terminated unicode string to pass
 * onto the Windows APIs.
 * @param fileName    Source uint16_t array of file name.
 * @param nameLen   Number of characters in fileName
 * @param buffer Buffer to store terminated string in (at least MAX_PATH)
 * @return Length of filename in characters (excl. nul), or -1 on failure.
 */
LOCAL int32_t convertFileName(const uint16_t *fileName, int32_t nameLen,
    uint16_t *buffer)
{
    int i;

    if (nameLen >= MAX_FILE_NAME_LEN-1)
    {
        DVMTraceErr("convertFileName failed, too long: %d", nameLen);
        return -1;
    }
    CRTL_memcpy(buffer, fileName, nameLen * sizeof(uint16_t));

    buffer[nameLen] = 0;

    for(i = 0; i < nameLen; i++)
    {
        if(buffer[i] == '/')
            buffer[i] = '\\';
    }

    return nameLen;
}
#elif defined(ARCH_ARM_SPD)

static uint16_t hostNames[2][256];

LOCAL uint16_t* hostNameConvert(const uint16_t* name,
                        int32_t nameLen,
                        int32_t index)
{
    uint16_t * hostName = NULL;

    //Check if name pointer is not the released pointer
    if((name != NULL) && (((uint16_t)(*name)) != 0x0) && (nameLen > 0))
    {
        int32_t copyLen = MIN(MAX_FILENAME_LEN - 1, nameLen); // avoid exceeding array bound because of nameChars is too big
        hostName = hostNames[index];
        CRTL_memcpy(hostName, name, copyLen<<1);
        hostName[copyLen] = '\0';
    }
    return hostName;
}
#endif

LOCAL const char *getAsciiName(const uint16_t *strData, int32_t strLength)
{
    static char tbuf[MAX_FILE_NAME_LEN];
    char        *p = tbuf;
    bool_t      canConvert = TRUE;

    if (strLength > MAX_FILE_NAME_LEN - 1)
    {
        strLength = MAX_FILE_NAME_LEN - 1;
    }

    while (--strLength >= 0)
    {
        if (*strData >= 0x7F)
        {
            canConvert = FALSE;
            break;
        }
        *p++ = (char)*strData++;
    }
    if (!canConvert)
    {   
        char * showInfo = "None-Ascii-Name";
        CRTL_memcpy(p, showInfo, CRTL_strlen(showInfo));
        p += CRTL_strlen(showInfo);

        DVMTraceWar("getAsciiName, None-Ascii file name;\n");
    }

    *p = 0;

    return tbuf;
}

void file_startup()
{
}

void file_shutdown() 
{
}


int64_t file_storageSize(const uint16_t* name, int32_t nameLen)
{
    //TODO: to implement
    return 0;
}

int64_t file_freeSize(const uint16_t* name, int32_t nameLen)
{
    //TODO: to implement
    return 0;
}


int32_t file_getLengthByName(const uint16_t* name, int32_t nameLen)
{
    //TODO: to implement
    return 0;
}

int32_t file_listOpen(const uint16_t* prefix, int32_t prefixLen, int32_t* session)
{
    //TODO: to implement
    return 0;
}

int32_t file_listNextEntry(const uint16_t* prefix, int32_t prefixLen, uint16_t* entry, int32_t entrySize, int32_t* session)
{
    //TODO: to implement
    return 0;
}

int32_t file_listclose(int32_t* session)
{
    //TODO: to implement
    return 0;
}

int32_t file_rename(const uint16_t* oldName, int32_t oldNameLen, const uint16_t* newName, int32_t newNameLen)
{
    //TODO: to implement
    return 0;
}

int32_t file_exists(const uint16_t* name, int32_t nameLen)
{
#ifdef ARCH_X86
    uint16_t fname[MAX_FILE_NAME_LEN];
    int32_t  fnameLen;
    DWORD    atts;

    DVMTraceInf("file_exists: %s", getAsciiName(name, nameLen));

    if ((fnameLen = convertFileName(name, nameLen, fname)) <= 0)
    {
        DVMTraceErr("file_exists: bad filename");
        return FILE_RES_FAILURE;
    }

    /* Remove any trailing '\' but not from 'x:\'*/
    if (((fnameLen > 3) || ((fnameLen == 3) && (fname[1] != ':'))) &&
            (fname[fnameLen - 1] == '\\'))
        fname[--fnameLen] = 0;

    /* Docs say that GetFileAttributes returns INVALID_FILE_ATTRIBUTES on fail,
     * but that doesn't seem to exist. Seems to return ~0
     */
    if ((atts = GetFileAttributesW(fname)) != ~0)
    {
        DVMTraceInf("file_exists: %s is %s", getAsciiName(name, nameLen),
            (atts & FILE_ATTRIBUTE_DIRECTORY) ? "directory" : "file");

        return (atts & FILE_ATTRIBUTE_DIRECTORY) ? FILE_RES_ISDIR : FILE_RES_ISREG;
    }
    else
    {
        DVMTraceErr("file_exists: %s does not exist", getAsciiName(name, nameLen));
    }

    return FILE_RES_FAILURE;
#elif defined(ARCH_ARM_SPD)
    uint16_t * hostName = hostNameConvert(name, nameLen, 0);
    uint16_t   attr = 0;

    if (nameLen == 3)
    {
        /* Since the API SFS_GetAttr cannot be used with the parameter only
         * contains device name, like "D:/", below implementation uses
         * SFS_GetDeviceStatus to check whether the storage is exists, if yes,
         * return CPL_FILE_ISDIR, otherwise, return failure.
         */
        return FILE_RES_ISDIR;
    }

    if(NULL == hostName)
    {
        DVMTraceInf("CPL_file_exists - ERROR: get host name error!!!");
        return FILE_RES_FAILURE;
    }

    if (hostName[nameLen-1] == '/')
    {
        /* The API SFS_GetAttr doesn't accept the directory path
         * end with '/', otherwise, the API will return error.
         */
        hostName[nameLen-1] = '\0';
    }

    if (SFS_GetAttr(hostName, &attr) != SFS_NO_ERROR)
    {
        DVMTraceInf("CPL_file_exists - ERROR: get attributes failure!!");
        return FILE_RES_FAILURE;
    }

    if ((uint16_t)(attr & SFS_ATTR_DIR) == SFS_ATTR_DIR)
    {
        return FILE_RES_ISDIR;
    }

    return FILE_RES_ISREG;    
#endif 
    return 0;    
}

int32_t file_open(const uint16_t* name, int32_t nameLen, int32_t mode, int32_t *handle)
{
#ifdef ARCH_X86
    HANDLE res;
    DWORD dwDesiredAccess = 0;
    DWORD dwCreationDispostion = 0;
    uint16_t fname[MAX_FILE_NAME_LEN];

    if (convertFileName(name, nameLen, fname) < 0)
    {
        DVMTraceErr("file_open failed, convertFileName error!\n");
        return -1;
    }

    if (mode & FILE_MODE_RD)
    {
        dwDesiredAccess |= GENERIC_READ;
        dwCreationDispostion = OPEN_EXISTING;
    }

    if (mode & FILE_MODE_WR)
    {
        dwDesiredAccess |= GENERIC_WRITE;
        dwCreationDispostion = OPEN_ALWAYS;
    }

    /* TBD: Fix opening always with sharing */
    res = CreateFileW(
        fname,
        dwDesiredAccess,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        dwCreationDispostion,
        FILE_ATTRIBUTE_NORMAL,
        NULL);

    if (res == INVALID_HANDLE_VALUE)
    {
        DVMTraceErr("CPL_file_open: open %s %d failed error=%d",
            getAsciiName(name, nameLen), mode, GetLastError());
    }
    else /* Success */
    {
        DVMTraceInf("CPL_file_open: _open %s %d returned 0x%x",
            getAsciiName(name, nameLen), mode, ((int32_t)res));

        *handle = (int32_t)res;

        return FILE_RES_SUCCESS;
    }

    return FILE_RES_FAILURE;
#elif defined(ARCH_ARM_SPD)
    uint16_t * hostName = hostNameConvert(name, nameLen, 0);
    uint16_t   fmode = 0;
    SFS_HANDLE sfsHandle;

    if(NULL == hostName)
    {
        DVMTraceErr("CPL_file_open - ERROR: get host name error!!!");
        return FILE_RES_FAILURE;
    }
    
    if (hostName[nameLen] == '/' || hostName[nameLen] == '\\')
    {
        //ensure no file separator at end of the file path to make
        //platform API's happy.
        hostName[nameLen] == '\0';
    }

    switch(mode)
    {
    case FILE_MODE_RD:
        fmode = SFS_MODE_SHARE_READ | SFS_MODE_OPEN_EXISTING;
        break;
    case FILE_MODE_WR:
        fmode = SFS_MODE_SHARE_WRITE | SFS_MODE_OPEN_ALWAYS;
        break;
    default:
        fmode = SFS_MODE_SHARE_READ  | SFS_MODE_SHARE_WRITE | SFS_MODE_OPEN_ALWAYS;
        break;
    }
    sfsHandle = SFS_CreateFile(hostName, fmode, 0, 0);

    if (sfsHandle == 0)
    {
        DVMTraceErr("CPL_file_open - ERROR: open file failure");
        return FILE_RES_FAILURE;
    }

    *handle = sfsHandle;

    return FILE_RES_SUCCESS;
#endif
    return 0;    
}

int32_t file_delete(const uint16_t* name, int32_t nameLen)
{
    //TODO: to implement
    return 0;
}


int32_t file_read(int32_t handle, char * readBuf, int32_t bufSize)
{
#ifdef ARCH_X86
    DWORD got;

    if (ReadFile((HANDLE)handle, readBuf, bufSize, &got, NULL) != 0)
    {
        DVMTraceInf("CPL_file_read handle=0x%x read %d bytes", handle, got);
        return got > 0 ? got : FILE_RES_EOF;
    }
    else
    {
        DVMTraceErr("CPL_file_read failed error=%d", GetLastError());
    }

    return FILE_RES_FAILURE;
#elif defined(ARCH_ARM_SPD)
    uint32_t gotten = 0;
    int32_t  result = FILE_RES_FAILURE;
    int32_t  relPos = 0;

    if(SFS_ReadFile((SFS_HANDLE)handle, readBuf, bufSize, (uint32 *)&gotten, NULL) == SFS_NO_ERROR)
    {
        result = ((int32_t)gotten > 0 ? (int32_t)gotten : (int32_t)gotten == 0 ? FILE_RES_EOF : FILE_RES_FAILURE);
    }

    if ((result == FILE_RES_FAILURE) &&
        (SFS_GetFilePointer(handle, SFS_SEEK_END, (int32 *)&relPos) == SFS_NO_ERROR) &&
        (relPos == 0))
    {
        result = FILE_RES_EOF;
    }

    return result;
#endif
    return 0;    
}

int32_t file_write(int32_t handle, char * writeBuf, int32_t bufSize)
{
#ifdef ARCH_X86
    DWORD written;

    if (WriteFile((HANDLE)handle, writeBuf, (DWORD)bufSize, &written, NULL) == 0)
    {
        DVMTraceErr("CPL_file_write handle=0x%x error=%d", handle, GetLastError());
    }
    else /* Success */
    {
        DVMTraceInf("CPL_file_write handle=0x%X wrote %d bytes", handle, written);

        return written;
    }

    return FILE_RES_FAILURE;
#elif defined(ARCH_ARM_SPD)    
    uint32 written = 0;

    if (SFS_WriteFile((SFS_HANDLE)handle, writeBuf, bufSize, &written, NULL) !=  SFS_NO_ERROR)
    {
        DVMTraceErr("CPL_file_write - ERROR:  file write failure!");
        return FILE_RES_FAILURE;
    }

    return written;
#endif
    return 0;    
}

int32_t file_truncate(int32_t handle, int32_t value)
{
    //TODO: to implement
    return 0;
}

int32_t file_seek(int32_t handle, int32_t value)
{
#ifdef ARCH_X86
    int32_t prevSize, delta;

    /* Docs say returns INVALID_SET_FILE_POINTER on error, but that doesn't
     * seem to exist ?
     */

    prevSize = (int32_t) GetFileSize((HANDLE)handle, NULL);
    delta = value - prevSize;


    if (value < 0)
    {
        DVMTraceDbg("file_seek: handle=0x%x negative value (%d)", handle, value);
    }
    else if (SetFilePointer((HANDLE)handle, value, NULL, FILE_BEGIN) < 0)
    {
        DVMTraceErr("file_seek: handle=0x%x failed - error=%d", handle, GetLastError());
    }
    else
    {
        if (delta > 0)
        {
            /* Increase file length */
            SetEndOfFile((HANDLE)handle);
        }
        return FILE_RES_SUCCESS;
    }

    return FILE_RES_FAILURE;
#elif defined(ARCH_ARM_SPD)
    int32 fileSize;

    if (SFS_GetFileSize((SFS_HANDLE)handle, (uint32*)&fileSize) != SFS_NO_ERROR)
    {
        DVMTraceErr("CPL_file_setPosition - ERROR: get file size error!!");
        return FILE_RES_FAILURE;
    }

    if (fileSize <= value)
    {
        if (SFS_SetFileSize((SFS_HANDLE)handle, value) != SFS_NO_ERROR)
        {
            DVMTraceErr("CPL_file_setPosition - ERROR: set file size error!!");
            return FILE_RES_FAILURE;
        }

        if (SFS_SetFilePointer((SFS_HANDLE)handle, (int64)0, SFS_SEEK_END) != SFS_NO_ERROR)
        {
            DVMTraceErr("CPL_file_setPosition - ERROR: set file pointer to end error!!");
            return FILE_RES_FAILURE;
        }
        DVMTraceErr("CPL_file_setPosition - INFO: handle=%d success to %d", handle, value);
        return FILE_RES_SUCCESS;
    }
    else if (SFS_SetFilePointer((SFS_HANDLE)handle, (int64)value, SFS_SEEK_BEGIN) != SFS_NO_ERROR)
    {
        DVMTraceErr("CPL_file_setPosition - ERROR: set file pointer to value error!!");
        return FILE_RES_FAILURE;
    }

    return FILE_RES_SUCCESS;
#endif    
    return 0;
}


int32_t file_getLengthByFd(int32_t handle)
{
#ifdef ARCH_X86
    int32_t len;

    len = (int32_t) GetFileSize((HANDLE)handle, NULL);
    DVMTraceInf("file_getLengthByFd - Length = %d\n", len);

    return len;
#elif defined(ARCH_ARM_SPD)
    int32_t len =0;
    SFS_ERROR_E ret = SFS_GetFileSize(handle,&len);
    if(ret == SFS_NO_ERROR)
        return len;
    return 0;
#endif    
    return 0;
}

int32_t file_flush(int32_t handle)
{
    //TODO: to implement
    return 0;
}


int32_t file_close(int32_t handle)
{
#ifdef ARCH_X86
    if (CloseHandle((HANDLE)handle))
    {
        DVMTraceInf("file_close handle=0x%x success", handle);
        return FILE_RES_SUCCESS;
    }

    DVMTraceErr("file_close handle=0x%x failed error=%d", handle, GetLastError());
    return FILE_RES_FAILURE;
#elif defined(ARCH_ARM_SPD)
    if (SFS_CloseFile((SFS_HANDLE)handle) != SFS_NO_ERROR)
    {
        DVMTraceErr("file_close - ERROR: file close error!!");
        return FILE_RES_FAILURE;
    }

    return FILE_RES_SUCCESS;
#endif    
    return 0;
}



int32_t file_mkdir(const uint16_t* name, int32_t nameLen)
{
    //TODO: to implement
    return 0;
}

