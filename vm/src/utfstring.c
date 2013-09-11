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
size_t dvmUtf8Len(const char* utf8Str)
{
    size_t len = 0;
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
    //TODO: to implement
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

