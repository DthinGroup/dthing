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
 * UTF-8 and Unicode string manipulation, plus java/lang/String convenience
 * functions.
 *
 * In most cases we populate the fields in the String object directly,
 * rather than going through an instance field lookup.
 */
#include <dthing.h>
#include <utfstring.h>
#include <gc.h>
#include <array.h>

/*
 * Allocate a new instance of the class String, performing first-use
 * initialization of the class if necessary. Upon success, the
 * returned value will have all its fields except hashCode already
 * filled in, including a reference to a newly-allocated char[] for
 * the contents, sized as given. Additionally, a reference to the
 * chars array is stored to the pChars pointer. Callers must
 * subsequently call dvmReleaseTrackedAlloc() on the result pointer.
 * This function returns NULL on failure.
 */
static StringObject* makeStringObject(u4 charsLength, ArrayObject** pChars)
{
    Object* result = NULL;
    ArrayObject* chars = NULL;

    /*
     * The String class should have already gotten found (but not
     * necessarily initialized) before making it here. We assert it
     * explicitly, since historically speaking, we have had bugs with
     * regard to when the class String gets set up. The assert helps
     * make any regressions easier to diagnose.
     */
    //assert(gDvm.classJavaLangString != NULL);

    if (!dvmIsClassInitialized(gDvm.classJavaLangString)) {
        /* Perform first-time use initialization of the class. */
        if (!dvmInitClass(gDvm.classJavaLangString)) {
            DVMTraceErr("FATAL: Could not initialize class String");
            //dvmAbort();
        }
    }

    result = dvmAllocObject(gDvm.classJavaLangString, 0);
    if (result == NULL) {
        return NULL;
    }

    chars = dvmAllocPrimitiveArray('C', charsLength, 0);
    if (chars == NULL) {
        //dvmReleaseTrackedAlloc(result, NULL);
        return NULL;
    }

    dvmSetFieldInt(result, STRING_FIELDOFF_COUNT, charsLength);
    dvmSetFieldObject(result, STRING_FIELDOFF_VALUE, (Object*) chars);
    //dvmReleaseTrackedAlloc((Object*) chars, NULL);
    /* Leave offset and hashCode set to zero. */

    *pChars = chars;
    return (StringObject*) result;
}


/*
 * Compute a hash code on a UTF-8 string, for use with internal hash tables.
 *
 * This may or may not yield the same results as the java/lang/String
 * computeHashCode() function.  (To make sure this doesn't get abused,
 * I'm initializing the hash code to 1 so they *don't* match up.)
 *
 * It would be more correct to invoke dexGetUtf16FromUtf8() here and compute
 * the hash with the result.  That way, if something encoded the same
 * character in two different ways, the hash value would be the same.  For
 * our purposes that isn't necessary.
 */
uint32_t dvmComputeUtf8Hash(const char* utf8Str)
{
    uint32_t hash = 1;

    while (*utf8Str != '\0')
        hash = hash * 31 + *utf8Str++;

    return hash;
}

/*
 * Like "strlen", but for strings encoded with "modified" UTF-8.
 *
 * The value returned is the number of characters, which may or may not
 * be the same as the number of bytes.
 *
 * (If this needs optimizing, try: mask against 0xa0, shift right 5,
 * get increment {1-3} from table of 8 values.)
 */
u4 dvmUtf8Len(const char* utf8Str)
{
    u4  len = 0;
    int ic;

    while ((ic = *utf8Str++) != '\0') {
        len++;
        if ((ic & 0x80) != 0) {
            /* two- or three-byte encoding */
            utf8Str++;
            if ((ic & 0x20) != 0) {
                /* three-byte encoding */
                utf8Str++;
            }
        }
    }

    return len;
}

/*
 * Convert a "modified" UTF-8 string to UTF-16.
 */
void dvmConvertUtf8ToUtf16(u2* utf16Str, const char* utf8Str)
{
    while (*utf8Str != '\0')
        *utf16Str++ = dexGetUtf16FromUtf8(&utf8Str);
}

/*
 * Given a UTF-16 string, compute the length of the corresponding UTF-8
 * string in bytes.
 */
static int utf16_utf8ByteLen(const u2* utf16Str, int len)
{
    int utf8Len = 0;

    while (len--) {
        unsigned int uic = *utf16Str++;

        /*
         * The most common case is (uic > 0 && uic <= 0x7f).
         */
        if (uic == 0 || uic > 0x7f) {
            if (uic > 0x07ff)
                utf8Len += 3;
            else /*(uic > 0x7f || uic == 0) */
                utf8Len += 2;
        } else
            utf8Len++;
    }
    return utf8Len;
}

/*
 * Convert a UTF-16 string to UTF-8.
 *
 * Make sure you allocate "utf8Str" with the result of utf16_utf8ByteLen(),
 * not just "len".
 */
static void convertUtf16ToUtf8(char* utf8Str, const u2* utf16Str, int len)
{

    while (len--) {
        unsigned int uic = *utf16Str++;

        /*
         * The most common case is (uic > 0 && uic <= 0x7f).
         */
        if (uic == 0 || uic > 0x7f) {
            if (uic > 0x07ff) {
                *utf8Str++ = (uic >> 12) | 0xe0;
                *utf8Str++ = ((uic >> 6) & 0x3f) | 0x80;
                *utf8Str++ = (uic & 0x3f) | 0x80;
            } else /*(uic > 0x7f || uic == 0)*/ {
                *utf8Str++ = (uic >> 6) | 0xc0;
                *utf8Str++ = (uic & 0x3f) | 0x80;
            }
        } else {
            *utf8Str++ = uic;
        }
    }

    *utf8Str = '\0';
}

/*
 * Use the java/lang/String.computeHashCode() algorithm.
 */
static u4 computeUtf16Hash(const u2* utf16Str, size_t len)
{
    u4 hash = 0;

    while (len--)
        hash = hash * 31 + *utf16Str++;

    return hash;
}


StringObject* dvmCreateStringFromCstr(const char* utf8Str) {
    //assert(utf8Str != NULL);
    return dvmCreateStringFromCstrAndLength(utf8Str, dvmUtf8Len(utf8Str));
}


/*
 * Create a java/lang/String from a C string, given its UTF-16 length
 * (number of UTF-16 code points).
 *
 * The caller must call dvmReleaseTrackedAlloc() on the return value.
 *
 * Returns NULL and throws an exception on failure.
 */
StringObject* dvmCreateStringFromCstrAndLength(const char* utf8Str, u4 utf16Length)
{
    //assert(utf8Str != NULL);

    ArrayObject* chars;
    u4 hashCode;
    StringObject* newObj = makeStringObject(utf16Length, &chars);
    if (newObj == NULL) {
        return NULL;
    }

    dvmConvertUtf8ToUtf16((u2*)(void*)chars->contents, utf8Str);

    hashCode = computeUtf16Hash((u2*)(void*)chars->contents, utf16Length);
    dvmSetFieldInt((Object*) newObj, STRING_FIELDOFF_HASHCODE, hashCode);

    return newObj;
}

/*
 * Create a new java/lang/String object, using the given Unicode data.
 */
StringObject* dvmCreateStringFromUnicode(const u2* unichars, int len)
{
    /* We allow a NULL pointer if the length is zero. */
    //assert(len == 0 || unichars != NULL);

    ArrayObject* chars;
    u4 hashCode;
    StringObject* newObj = makeStringObject(len, &chars);
    if (newObj == NULL) {
        return NULL;
    }

    if (len > 0) CRTL_memcpy(chars->contents, unichars, len * sizeof(u2));

    hashCode = computeUtf16Hash((u2*)(void*)chars->contents, len);
    dvmSetFieldInt((Object*)newObj, STRING_FIELDOFF_HASHCODE, hashCode);

    return newObj;
}

/*
 * Create a new C string from a java/lang/String object.
 *
 * Returns NULL if the object is NULL.
 */
char* dvmCreateCstrFromString(const StringObject* jstr)
{
    char* newStr;
	const u2* data = dvmGetStringData(jstr);
	int len = dvmGetStringLength(jstr);
	int byteLen = 0;

	if (data == NULL || len == 0) {
        return NULL;
	}

    byteLen = utf16_utf8ByteLen(data, len);
    newStr = (char*) CRTL_malloc(byteLen + 1);
	if (newStr == NULL) {
        return NULL;
	}
    convertUtf16ToUtf8(newStr, data, len);

    return newStr;
}

/*
 * Get UTF-16 characters address of a java/lang/String.
 *
 * @return NULL if "jstr" is NULL.
 */
const u2* dvmGetStringData(const StringObject* jstr) {
	ArrayObject* chars;
	int offset;

	if (jstr == NULL) {
		return NULL;
	}

	offset = dvmGetFieldInt((Object*) jstr, STRING_FIELDOFF_OFFSET);
	chars = (ArrayObject*) dvmGetFieldObject((Object*) jstr, STRING_FIELDOFF_VALUE);
	return (const u2*) chars->contents + offset;
}

/*
 * Get UTF-16 characters count of a java/lang/String.
 *
 * @return 0 if "jstr" is NULL.
 */
int dvmGetStringLength(const StringObject* jstr) {
	if (jstr == NULL) {
		return 0;
	}
	return dvmGetFieldInt((Object*) jstr, STRING_FIELDOFF_COUNT);
}

/*
 * MUST free the return ptr after using!!!
 */
const u1 * dvmGetStringDataInUtf8(const StringObject* jstr){
	u2 * data;
	u1 * rdata = NULL;
	int length = 0;
	if(jstr == NULL || (data = dvmGetStringData(jstr)) == NULL)
		return NULL;
	length = dvmGetStringLength(jstr);
	length = utf16_utf8ByteLen(data, length);

	rdata = CRTL_malloc(length + 2)	;
	if(rdata == NULL)
		return NULL;

	CRTL_memset(rdata, 0, length +2);
	convertUtf16ToUtf8(rdata, data, length);
	return rdata;
}