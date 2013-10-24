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
 * Internal-native initialization and some common utility functions.
 */

#include <classmisc.h>
#include <exception.h>
#include <dthread.h>
#include <utfstring.h>
#include <exception.h>

#define PRIM_TYPE_TO_LETTER "ZCFDBSIJV"     /* must match order in enum */

/*
 * Validate a "binary" class name, e.g. "java.lang.String" or "[I".
 */
static bool_t validateClassName(const char* name)
{
    int len = CRTL_strlen(name);
    int i = 0;

    /* check for reasonable array types */
    if (name[0] == '[') {
        while (name[i] == '[')
            i++;

        if (name[i] == 'L') {
            /* array of objects, make sure it ends well */
            if (name[len-1] != ';')
                return FALSE;
        } else if (CRTL_strchr(PRIM_TYPE_TO_LETTER, name[i]) != NULL) {
            if (i != len-1)
                return FALSE;
        } else {
            return FALSE;
        }
    }

    /* quick check for illegal chars */
    for ( ; i < len; i++) {
        if (name[i] == '/')
            return FALSE;
    }

    return TRUE;
}

/*
 * Return a newly-allocated string for the "dot version" of the class
 * name for the given type descriptor. That is, The initial "L" and
 * final ";" (if any) have been removed and all occurrences of '/'
 * have been changed to '.'.
 *
 * "Dot version" names are used in the class loading machinery.
 * See also dvmHumanReadableDescriptor.
 */
char* dvmDescriptorToDot(const char* str)
{
    size_t at = CRTL_strlen(str);
    char* newStr;

    if ((at >= 2) && (str[0] == 'L') && (str[at - 1] == ';')) {
        at -= 2; /* Two fewer chars to copy. */
        str++; /* Skip the 'L'. */
    }

    newStr = (char*)CRTL_malloc(at + 1); /* Add one for the '\0'. */
    if (newStr == NULL)
        return NULL;

    newStr[at] = '\0';

    while (at > 0) {
        at--;
        newStr[at] = (str[at] == '/') ? '.' : str[at];
    }

    return newStr;
}

/*
 * Return a newly-allocated string for the type descriptor
 * corresponding to the "dot version" of the given class name. That
 * is, non-array names are surrounded by "L" and ";", and all
 * occurrences of '.' are changed to '/'.
 */
char* dvmDotToDescriptor(const char* str)
{
    size_t length = CRTL_strlen(str);
    int wrapElSemi = 0;
    char* newStr;
    char* at;

    if (str[0] != '[') {
        length += 2; /* for "L" and ";" */
        wrapElSemi = 1;
    }

    newStr = at = CRTL_malloc(length + 1); /* + 1 for the '\0' */

    if (newStr == NULL) {
        return NULL;
    }

    if (wrapElSemi) {
        *(at++) = 'L';
    }

    while (*str) {
        char c = *(str++);
        if (c == '.') {
            c = '/';
        }
        *(at++) = c;
    }

    if (wrapElSemi) {
        *(at++) = ';';
    }

    *at = '\0';
    return newStr;
}

/*
 * Find a class by name, initializing it if requested.
 */
ClassObject* dvmFindClassByName(StringObject* nameObj)
{
    ClassObject* clazz = NULL;
    char* name = NULL;
    char* descriptor = NULL;

    if (nameObj == NULL) {
        dvmThrowException("Ljava/lang/NullPointerException;", NULL);
        goto bail;
    }
    name = dvmCreateCstrFromString(nameObj);

    /*
     * We need to validate and convert the name (from x.y.z to x/y/z).  This
     * is especially handy for array types, since we want to avoid
     * auto-generating bogus array classes.
     */
    if (!validateClassName(name)) {
        DVMTraceWar("dvmFindClassByName rejecting '%s'\n", name);
        dvmThrowException("Ljava/lang/ClassNotFoundException;", name);
        goto bail;
    }

    descriptor = dvmDotToDescriptor(name);
    if (descriptor == NULL) {
        goto bail;
    }

    clazz = dvmFindClassNoInit(descriptor);

    if (clazz == NULL) {
        //Thread* self = NULL;
        //Object* oldExcep = NULL;

        DVMTraceInf("FAIL: load %s\n", descriptor);
        //self = dthread_currentThread();
        //oldExcep = dvmGetException(self);
        //dvmClearException(self);
        //dvmThrowChainedException("Ljava/lang/ClassNotFoundException;", name, oldExcep);
        dvmThrowException("Ljava/lang/ClassNotFoundException;", name);
    } else {
        DVMTraceInf("GOOD: load %s --> %p ldr\n", descriptor, clazz);
    }

bail:
    CRTL_free(name);
    CRTL_free(descriptor);
    return clazz;
}
