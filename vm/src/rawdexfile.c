/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Open an unoptimized DEX file.
 */

#include <dthing.h>
#include <rawdexfile.h>
#include <opl_file.h>
#include <encoding.h>
#include <heap.h>


/* See documentation comment in header. */
int dvmRawDexFileOpenArray(const u1* pBytes, u4 length, DvmDex** ppDDex)
{
    DvmDex* pDvmDex = NULL;

    if (dvmDexFileOpenFromRawData(pBytes, length, &pDvmDex) < 0)
    {
        DVMTraceErr("dvmRawDexFileOpenArray failure");
        return -1;
    }
    *ppDDex = pDvmDex;
    return 0;
}


/* See documentation comment in header. */
int dvmRawDexFileOpen(const char* fileName, DvmDex** ppDDex)
{
    /*
     * TODO: This duplicates a lot of code from dvmJarFileOpen() in
     * JarFile.c. This should be refactored.
     */

    DvmDex* pDvmDex = NULL;
    int32_t fhandle;

    uint16_t fUcs2Name[MAX_FILE_NAME_LEN] = {0x0,};
    int32_t  srcBytes = (int32_t)CRTL_strlen(fileName);
    int32_t  dstChars = 0;
    int32_t  result = -1;

    dstChars = convertAsciiToUcs2(fileName, srcBytes, fUcs2Name, MAX_FILE_NAME_LEN);
    if (dstChars <= 0)
    {
        DVMTraceErr("dvmRawDexFileOpen - Error: corrupt file name\n");
        return -1;
    }

    if (file_open(fUcs2Name, dstChars, FILE_MODE_RD, &fhandle) != FILE_RES_SUCCESS)
    {
        DVMTraceErr("dvmRawDexFileOpen - Error: open file failure\n");
        return -1;
    }

    /*
     * Map the cached version.  This immediately rewinds the fd, so it
     * doesn't have to be seeked anywhere in particular.
     */
    if (dvmDexFileOpenFromFd(fhandle, &pDvmDex) != 0) {
        DVMTraceErr("Unable to open dex file: %s\n", fileName);
        goto bail;
    }

    DVMTraceInf("Successfully opened '%s'", fileName);

    *ppDDex = pDvmDex;

    /* success */
    result = 0;

bail:
    return result;
}

