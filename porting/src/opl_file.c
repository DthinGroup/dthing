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
#include <std_global.h>
#include <opl_file.h>

#ifdef WIN32
/* platform header files */
#include <windows.h>
#endif

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
}

int32_t file_open(const uint16_t* name, int32_t nameLen, int32_t mode, int32_t *handle)
{
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
}

int32_t file_delete(const uint16_t* name, int32_t nameLen)
{
    //TODO: to implement
    return 0;
}


int32_t file_read(int32_t handle, char * readBuf, int32_t bufSize)
{
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
}

int32_t file_write(int32_t handle, char * writeBuf, int32_t bufSize)
{
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
}

int32_t file_truncate(int32_t handle, int32_t value)
{
    //TODO: to implement
    return 0;
}

int32_t file_seek(int32_t handle, int32_t value)
{
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
}


int32_t file_getLengthByFd(int32_t handle)
{
    int32_t len;

    len = (int32_t) GetFileSize((HANDLE)handle, NULL);
    DVMTraceInf("file_getLengthByFd - Length = %d\n", len);

    return len;
}

int32_t file_flush(int32_t handle)
{
    //TODO: to implement
    return 0;
}


int32_t file_close(int32_t handle)
{
    if (CloseHandle((HANDLE)handle))
    {
        DVMTraceInf("file_close handle=0x%x success", handle);
        return FILE_RES_SUCCESS;
    }

    DVMTraceErr("file_close handle=0x%x failed error=%d", handle, GetLastError());
    return FILE_RES_FAILURE;
}



int32_t file_mkdir(const uint16_t* name, int32_t nameLen)
{
    //TODO: to implement
    return 0;
}

