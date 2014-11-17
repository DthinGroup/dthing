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

#define OPEN_ERROR 0

static bool_t isRegistered = FALSE;


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
#if defined(ARCH_ARM_SPD)

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


LOCAL const char *trstr(const uint16_t *strData, int32_t strLength)
{
    char tbuf[MAX_FILENAME_LEN];
    char *p = tbuf;

    if (strLength > MAX_FILENAME_LEN-1)
        strLength = MAX_FILENAME_LEN-1;

    while (--strLength >= 0)
        *p++ = (char)*strData++;

    *p = 0;

    return tbuf;
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
    UNUSED(prefix);
    UNUSED(prefixLen);
    
    *session = (int32_t)INVALID_HANDLE_VALUE;
    return FILE_RES_SUCCESS;
}

int32_t file_listNextEntry(const uint16_t* prefix, int32_t prefixLen, uint16_t* entry, int32_t entrySize, int32_t* session)
{
#ifdef ARCH_X86
    HANDLE handle = (HANDLE)*session;
    uint16_t fname[MAX_FILE_NAME_LEN];
    int32_t  len;
    WIN32_FIND_DATAW  fData;     /* Data for find results */
    bool_t   isDirectory;

    if ((len = convertFileName(prefix, prefixLen, fname)) <= 0 ||
        (len+2) >= MAX_FILE_NAME_LEN)    /* Ensure space for wildcard */
    {
        DVMTraceInf("file_listNextEntry: bad filename\n");
        return FILE_RES_FAILURE;
    }

    for(;;) /* Loop until we find an appropriate file */
    {
        if (handle == INVALID_HANDLE_VALUE)
        {
            /* Append a "*" wildcard to the prefix */
            fname[len  ] = (uint16_t)'*';
            fname[len+1] = (uint16_t)'\0';

            handle = FindFirstFileW(fname, &fData);
            if (handle == INVALID_HANDLE_VALUE)
            {
                if (GetLastError() == ERROR_NO_MORE_FILES)
                {
                    DVMTraceInf("CPL_file_listNextEntry: no files\n");
                    *entry = '\0';
                    return 0;
                }

                DVMTraceErr("CPL_file_listNextEntry: findFirst failed err %d\n",
                    GetLastError());
                return FILE_RES_FAILURE;
            }
            *session = (int32_t)handle;
        }
        else /* Find the next file... */
        {
            if (!FindNextFileW(handle, &fData))
            {
                if (GetLastError() == ERROR_NO_MORE_FILES)
                {
                    DVMTraceInf("CPL_file_listNextEntry: no more files\n");
                    *entry = '\0';
                    return 0;
                }
                DVMTraceErr("CPL_file_listNextEntry: findNext failed err %d\n",
                    GetLastError());
                return FILE_RES_FAILURE;
            }
        }

        /* We have found a matching entry */
        if (fData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            /* Ignore "." and ".." */
            if ((CRTL_wcscmp(fData.cFileName, L".") == 0) ||
                    (CRTL_wcscmp(fData.cFileName, L"..") == 0))
                continue;

            isDirectory = TRUE;
        }
        else
            isDirectory = FALSE;

        /* Find the directory name in the prefix, use fname for indexing to
         *  handle cases where '/' have been converted to '\' */
        while (prefixLen > 0)
        {
            if (fname[prefixLen - 1] == '\\')
                break;

            prefixLen--;
        }

        /* Check the total filename length */
        len = CRTL_wcslen(fData.cFileName);
        if ((int32_t)((prefixLen + len + (isDirectory ? 1 : 0))*sizeof(uint16_t)) > entrySize)
        {
            DVMTraceInf("ALWAYS: CPL_file_listNextEntry: filename too long\n");
            continue;
        }

        CRTL_memcpy(entry, prefix, prefixLen*sizeof(uint16_t));
        CRTL_memcpy(&entry[prefixLen], fData.cFileName, len*sizeof(uint16_t));

        if (isDirectory)
            entry[prefixLen + len++] = '/';

        DVMTraceInf("CPL_file_listNextEntry: found %s\n",
            trstr(entry, prefixLen + len));

        return prefixLen + len;
    }

    return FILE_RES_FAILURE;
    
#elif defined(ARCH_ARM_SPD)
	SFS_HANDLE      sfsHandle;
    SFS_FIND_DATA_T sfsAttr;
    int32_t         fnameSize = 0;
    int32_t         splashPos = 0;
    uint16_t 		fname[MAX_FILE_NAME_LEN] = {0};
    uint8_t 		asciiname[MAX_FILE_NAME_LEN]={0};
    int32_t  		len =0;    

	DVMTraceInf("file_listNextEntry in\n");
	if(NULL == prefix)
    {
        DVMTraceInf("CPL_file_listNextEntry - ERROR: get host name error!!!");
        return FILE_RES_FAILURE;
    }
    
	if ((len = convertFileName(prefix, prefixLen, fname)) <= 0 ||
        (len+2) >= MAX_FILE_NAME_LEN)    /* Ensure space for wildcard */
    {
        DVMTraceInf("file_listNextEntry: bad filename\n");
        return FILE_RES_FAILURE;
    }

    fname[prefixLen] = '*'; //add matching rule.
    fname[prefixLen+1] = '\0';
    
    CRTL_memset(asciiname,0,sizeof(asciiname));
    CRTL_wstrtoutf8(asciiname,prefixLen,fname,prefixLen);
	DVMTraceInf("CPL_file_listNextEntry - match expr:%s,handle:%d\n", asciiname,*session);
	
    sfsHandle = *session;
    if (*session == -1)
    {
        if ((sfsHandle = SFS_FindFirstFile(fname, &sfsAttr)) == 0)
        {
            DVMTraceInf("CPL_file_listNextEntry - INFO: first NO FILE found!");
            *entry = '\0';
            return 0;
        }
        *session = sfsHandle;
    }
    else if (SFS_FindNextFile(sfsHandle, &sfsAttr) != SFS_NO_ERROR)
    {
        DVMTraceInf("CPL_file_listNextEntry - INFO: next NO FILE found!");
        *entry = '\0';
        return 0;
    }

    //find the last separating character.
    for(splashPos = prefixLen; fname[splashPos] != '\\'; )
    {
         --splashPos;
    }
    splashPos++;

    fnameSize = splashPos + CRTL_wcslen(sfsAttr.name);

    if (fnameSize >= entrySize)
    {
        DVMTraceInf("CPL_file_listNextEntry - ERROR: entrySize is too small!!\n");
        return 0;
    }

    CRTL_memcpy(entry, fname, splashPos<<1);
    CRTL_memcpy(&entry[splashPos], sfsAttr.name, CRTL_wcslen(sfsAttr.name)*sizeof(uint16_t));

    if (sfsAttr.attr & SFS_ATTR_DIR)
    {
        entry[fnameSize++] = '/';
    }

    entry[fnameSize] = '\0';
    CRTL_memset(asciiname,0,sizeof(asciiname));
    CRTL_wstrtoutf8(asciiname,fnameSize,entry,fnameSize);

    DVMTraceInf("CPL_file_listNextEntry - INFO: file:%s,fnameSize (%d)\n", asciiname,fnameSize);
    return fnameSize;
#endif    
	return -1;
}

int32_t file_listclose(int32_t* session)
{
#ifdef ARCH_X86
    HANDLE handle = (HANDLE)*session;

    DVMTraceInf("CPL_file_listClose: handle 0x%08x\n", handle);

    if (handle != INVALID_HANDLE_VALUE)
        FindClose(handle);

    return FILE_RES_SUCCESS;
#elif defined(ARCH_ARM_SPD)
	SFS_HANDLE sfsHandle = * session;
	
	DVMTraceInf("CPL_file_listClose: handle 0x%08x\n", sfsHandle);
	if (sfsHandle != INVALID_HANDLE_VALUE)
		SFS_FindClose(sfsHandle);
	return FILE_RES_FAILURE;
#endif    
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

    DVMTraceInf("file_exists: %s\n", getAsciiName(name, nameLen));

    if ((fnameLen = convertFileName(name, nameLen, fname)) <= 0)
    {
        DVMTraceErr("file_exists: bad filename\n");
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
        DVMTraceInf("file_exists: %s is %s\n", getAsciiName(name, nameLen),
            (atts & FILE_ATTRIBUTE_DIRECTORY) ? "directory" : "file");

        return (atts & FILE_ATTRIBUTE_DIRECTORY) ? FILE_RES_ISDIR : FILE_RES_ISREG;
    }
    else
    {
        DVMTraceErr("file_exists: %s does not exist\n", getAsciiName(name, nameLen));
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
	
	DVMTraceDbg("file_open mode:%d ,name:%s \n",mode,getAsciiName(name,nameLen));
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
        DVMTraceErr("CPL_file_open: open %s %d failed error=%d\n",
            getAsciiName(name, nameLen), mode, GetLastError());
    }
    else /* Success */
    {
        DVMTraceInf("CPL_file_open: _open %s %d returned 0x%x\n",
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
    
	DVMTraceDbg("file_open mode:%d ,name:%s \n",mode,getAsciiName(name,nameLen));
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
        DVMTraceErr("====file_open - ERROR: open file failure");
        return FILE_RES_FAILURE;
    }

    *handle = sfsHandle;

    return FILE_RES_SUCCESS;
#endif
    return 0;    
}

int32_t file_delete(const uint16_t* name, int32_t nameLen)
{
    uint16_t fname[MAX_FILE_NAME_LEN];
#ifdef ARCH_X86
    DWORD attribs;
    if (convertFileName(name, nameLen, fname) <= 0)
    {
        DVMTraceErr("file_delete: bad filename\n");
    }
    else if ((attribs = GetFileAttributesW(fname)) == ~0)
    {
        DVMTraceErr("file_delete: failed to get attributes for %s\n", fname);
    }
    else if ((attribs & FILE_ATTRIBUTE_DIRECTORY) && (!RemoveDirectoryW(fname)))
    {
        DVMTraceErr("file_delete: failed to delete directory %s error (%d)\n", fname, GetLastError());
    }
    else if (!(attribs & FILE_ATTRIBUTE_DIRECTORY) && (!DeleteFileW(fname)))
    {
        DVMTraceErr("file_delete: failed to delete file %s errno(%d)\n", fname, GetLastError());
    }
    else /* Success */
    {
        return FILE_RES_SUCCESS;
    }
   
#elif defined(ARCH_ARM_SPD)
    int32_t res;
    uint16_t * hostName = hostNameConvert(name, nameLen, 0);
    if (hostName==NULL)
    {
        DVMTraceErr("file_delete: bad filename\n");
    }
    res = file_exists(name, nameLen);

    if (res == FILE_RES_ISDIR && SFS_DeleteDirectory(hostName) != SFS_NO_ERROR)
    {
        DVMTraceErr("file_delete: failed to remove directory!\n");
    }
    else if (res == FILE_RES_ISREG && SFS_DeleteFile(hostName, NULL) != SFS_NO_ERROR)
    {
        DVMTraceErr("file_delete: failed to delete file name!\n");
    }
    else
    {
        return FILE_RES_SUCCESS;
    }

#endif
    return FILE_RES_FAILURE;
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
#ifdef ARCH_X86

#elif defined(ARCH_ARM_SPD)
	if (SFS_SetFileSize((SFS_HANDLE)handle, value) != SFS_NO_ERROR)
	{
		return FILE_RES_FAILURE;
	}
	return FILE_RES_SUCCESS;
#endif
    return 0;
}

int32_t file_getsize(int32_t handle,int32_t * size)
{
#ifdef ARCH_X86
	*size = (int32_t) GetFileSize((HANDLE)handle, NULL);
	if(*size == INVALID_FILE_SIZE)
	{
		return FILE_RES_FAILURE;
	}
	return FILE_RES_SUCCESS;
#elif defined(ARCH_ARM_SPD)
	if(SFS_GetFileSize(handle, size)!= SFS_NO_ERROR)
	{
		return FILE_RES_FAILURE;
	}
	return FILE_RES_SUCCESS;
#endif
}

//return :actual value of seek/skip
int32_t file_seekex(int32_t handle, int32_t value, int32_t whence)
{
	int32_t skip=0;
#ifdef ARCH_X86
    int32_t prevSize, delta;
    bool_t  setEnd = FALSE;
    DWORD   winWhence;  

    /* Docs say returns INVALID_SET_FILE_POINTER on error, but that doesn't
     * seem to exist ?
     */
    prevSize = (int32_t) GetFileSize((HANDLE)handle, NULL);

    switch (whence)
    {
    case FILE_SEEK_BEGIN:
        winWhence = FILE_BEGIN;
        if (value < 0)
        {
            DVMTraceDbg("file_seekex(FILE_SEEK_BEGIN): handle=0x%x negative value (%d)", handle, value);
            //return FILE_RES_FAILURE;
			return skip;
        }
        if (value - prevSize > 0)
        {
			skip = prevSize;
            setEnd = TRUE;
        }
        break;

    case FILE_SEEK_CURRENT:
        winWhence = FILE_CURRENT;
        delta = SetFilePointer((HANDLE)handle, 0, NULL, winWhence);
        if (delta + value < 0)
        {
            DVMTraceDbg("file_seekex(FILE_SEEK_CURRENT): handle=0x%x negative value (%d)", handle, value);
            //return FILE_RES_FAILURE;
			return skip;
        }
        if (delta + value > prevSize)
        {
			skip = prevSize - delta;
            setEnd = TRUE;
        }
        break;

    case FILE_SEEK_END:
        winWhence = FILE_END;
        if (prevSize + value < 0)
        {
            DVMTraceDbg("file_seekex(FILE_SEEK_END): handle=0x%x negative value (%d)", handle, value);
            //return FILE_RES_FAILURE;
			return skip;
        }
        if (value >= 0)
        {
            setEnd = TRUE;
        }
        break;
        
    default:
    	DVMTraceErr("unsupport seek type!\n");
    	break;
    }

    if (SetFilePointer((HANDLE)handle, value, NULL, winWhence) < 0)
    {
        DVMTraceErr("file_seekex: handle=0x%x failed - error=%d", handle, GetLastError());
        //return FILE_RES_FAILURE;
		return 0;
    }
    else if (setEnd)
    {
        /* Increase file length */
        SetEndOfFile((HANDLE)handle);
    }
    return skip;//FILE_RES_SUCCESS;

#elif defined(ARCH_ARM_SPD)
    int32_t prevSize=0,delta=0;
    bool_t  setEnd = FALSE;
    int64_t winWhence = 0;

    if (SFS_GetFileSize((SFS_HANDLE)handle, (uint32*)&prevSize) != SFS_NO_ERROR)
    {
        DVMTraceErr("CPL_file_setPosition - ERROR: get file size error!!");
        return FILE_RES_FAILURE;
    }
    
    switch (whence)
    {
    case SFS_SEEK_BEGIN:
        winWhence = SFS_SEEK_BEGIN;
        if (value < 0)
        {
            DVMTraceDbg("file_seekex(FILE_SEEK_BEGIN): handle=0x%x negative value (%d)", handle, value);
            return FILE_RES_FAILURE;
        }
        if (value - prevSize > 0)
        {
            setEnd = TRUE;
        }
        break;

    case SFS_SEEK_CUR:
        winWhence = SFS_SEEK_CUR;
        delta = SFS_SetFilePointer((SFS_HANDLE)handle, 0, winWhence);
        if (delta + value < 0)
        {
            DVMTraceDbg("file_seekex(FILE_SEEK_CURRENT): handle=0x%x negative value (%d)", handle, value);
            return FILE_RES_FAILURE;
        }
        if (delta + value > prevSize)
        {
            setEnd = TRUE;
        }
        break;

    case SFS_SEEK_END:
        winWhence = SFS_SEEK_END;
        if (prevSize + value < 0)
        {
            DVMTraceDbg("file_seekex(FILE_SEEK_END): handle=0x%x negative value (%d)", handle, value);
            return FILE_RES_FAILURE;
        }
        if (value >= 0)
        {
            setEnd = TRUE;
        }
        break;
        
    default:
    	DVMTraceErr("unsupport seek type!\n");
    	break;
    }
    
    if (SFS_SetFilePointer((SFS_HANDLE)handle,value, winWhence) != SFS_NO_ERROR)
    {
        DVMTraceErr("CPL_file_setPosition - ERROR: set file pointer to end error!!");
        return FILE_RES_FAILURE;
    }
    else if (setEnd)
    {
        /* Increase file length */
        SFS_SetFilePointer((SFS_HANDLE)handle, (int64_t)0, SFS_SEEK_END) ;
    }
    
    DVMTraceInf("Seek over,success!\n");
    return FILE_RES_SUCCESS;
#endif
}

int32_t file_seek(int32_t handle, int32_t value)
{
    file_seekex(handle, value, FILE_SEEK_BEGIN);
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
    DVMTraceInf("file_getLengthByFd - Length = %d\n", len);
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
#if defined(ARCH_X86)

#elif defined(ARCH_ARM_SPD)
	uint16_t fname[MAX_FILENAME_LEN] = {0};
	uint8_t  asciiname[MAX_FILENAME_LEN] = {0};
	 if(NULL == name)
    {
        DVMTraceInf("CPL_file_mkdir - ERROR: get host name error!!!");
        return FILE_RES_FAILURE;
    }

	CRTL_memcpy(fname,name,nameLen*2);
    if (fname[nameLen-1] == '/' || fname[nameLen-1] == '\\') 
    	fname[nameLen-1] = '\0';
    	
    CRTL_wstrtoutf8(asciiname,nameLen,fname,nameLen);
    DVMTraceInf("To make dir:%s \n",asciiname);
    if (SFS_CreateDirectory(fname) != SFS_NO_ERROR)
    {
        DVMTraceErr("CPL_file_mkdir - ERROR: create directory file!!");
         return FILE_RES_FAILURE;
    }

    return FILE_RES_SUCCESS;
#endif    
    return 0;
}

#if defined(ARCH_ARM_SPD)

#define DEV_NAME			L"D"
#define DTHING_PATH 		L"D:\\dthing"        
#define DTHING_PATH_FIX 	L"D:\\dthing\\"

#define DTHING_PATH_FIX_C 	"D:\\dthing\\"

bool_t file_registerDeviceIfNeed()
{
	uint16_t    dev_name[] = {'D', 0x00};
    SFS_DEVICE_FORMAT_E  format = SFS_FAT32_FORMAT;
    SFS_ERROR_E sfsError = SFS_ERROR_NONE;
	bool_t ret = TRUE;
	
	DVMTraceDbg("====file device register\n");
	sfsError = SFS_GetDeviceStatus(dev_name);
	
	switch(sfsError)
	{
		case SFS_ERROR_NONE:
		  isRegistered = TRUE;
			DVMTraceInf("====device is ok!\n");
			break;
			
		case SFS_ERROR_DEVICE:
		case SFS_ERROR_SYSTEM:	
		case SFS_ERROR_NOT_EXIST:
			DVMTraceDbg("====device is error,register it\n");
			sfsError = SFS_RegisterDevice(dev_name, &format);
			if(SFS_ERROR_NONE == sfsError)
		    {
		        isRegistered = TRUE;
		        DVMTraceDbg("==== SFS_RegisterDevice SUCCESS");
		    }
		    else
		    {		
		        sfsError = SFS_Format(L"D", SFS_AUTO_FORMAT, NULL);
		        if(SFS_ERROR_NONE == sfsError) {
		            isRegistered = TRUE;
		            DVMTraceDbg("==== SFS_Format SUCCESS");
		            DVMTraceDbg("==== SFS_RegisterDevice SUCCESS");
		        }
		        else {
		            DVMTraceErr("====== SFS_RegisterDevice error: %d", sfsError);
		            DVMTraceErr("====== SFS_Format error: %d", sfsError);
		            ret =FALSE;
		        }
		    }
			break;
			
		default:
			DVMTraceErr("====unhandle device error:%d\n",sfsError);
			ret =FALSE;
			break;
	}
	return ret;
}

#endif


uint16_t * file_getDthingWDir()
{
#if defined (ARCH_X86)	
	return L"D:\\nix.long\\ReDvmAll\\dvm\\appdb\\";
#elif defined(ARCH_ARM_SPD)	
	return DTHING_PATH_FIX;
#endif
	return NULL;	
}

uint8_t * file_getDthingDir()
{
#if defined (ARCH_X86)	
	return "D:\\nix.long\\ReDvmAll\\dvm\\appdb\\";
#elif defined(ARCH_ARM_SPD)	
	return DTHING_PATH_FIX_C;
#endif
	return NULL;	
}

void file_startup()
{	
#if defined (ARCH_X86)

#elif defined(ARCH_ARM_SPD)
	bool_t ret;
	int32_t dir_len = CRTL_wcslen(DTHING_PATH);
	if(file_registerDeviceIfNeed())
	{
		if(file_exists(DTHING_PATH,dir_len) != FILE_RES_ISDIR)
		{
			DVMTraceInf("===file device register:to make dir\n");
			file_mkdir(DTHING_PATH,dir_len);
		}
	}
	else
	{
		DVMTraceInf("===file device register fail!\n");
	}
#endif	
}

void file_shutdown() 
{
}

bool_t file_isFSRegistered(void)
{
#ifdef ARCH_X86
    return TRUE;
#elif defined(ARCH_ARM_SPD)
    SCI_TRACE_LOW("file_isFSRegistered:%d", isRegistered);
    return isRegistered;
#endif
}