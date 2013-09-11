/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/07/25 $
 * Last modified:	$Date: 2013/07/28 $
 * Version:         $ID: encoding.h#1
 */


#ifndef __ENCODING_H__
#define __ENCODING_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Convert an array of Unicode characters to ASCII bytes.
 * The conversion replaces \\u0000 characters and those over 255 with a '?'.
 * The resulting data is always terminated with a zero byte, even
 * if that means truncating the string.
 *
 * Pass dstBytes=0 and dst=NULL to get the number of bytes necessary for
 * encoding including the zero terminating byte.
 *
 * @param src      Source array of Unicode characters
 * @param srcChars Number of source characters to encode (characters not bytes)
 * @param dst      Destination for encoded data
 * @param dstBytes Maximum number of bytes to write to dst (inc terminator)
 * @return Number of bytes required for the encoding or -1 if the string had
 *         to be truncated.
 */
int32_t convertUcs2ToAscii(const uint16_t* src, int32_t srcChars, char* dst, int32_t dstBytes);


/**
 * Convert ASCII bytes into an array of Unicode characters.
 *
 * Truncation occurs when the destination array is not big enough,
 * or if a nul byte is found before srcBytes have been consumed.
 *
 * Unlike some of the other conversion routines, the destination array is
 * not terminated with a \\u0000 character.
 *
 * Pass dst=NULL to get the number of Unicode characters
 * that the ASCII data decodes to.
 *
 * @param src       Source array of ASCII encoded bytes
 * @param srcBytes  Number of bytes to decode (-1 means up to first nul byte)
 * @param dst       Destination for decoded characters
 * @param dstBytes  Size of destination array in bytes (not characters)
 *
 * @return Number of characters that the source data decodes to, or
 *  -1 if the destination array is not big enough.
 */
int32_t convertAsciiToUcs2(const char* src, int32_t srcBytes, uint16_t* dst, int32_t dstBytes);

/**
 * Convert an array of Unicode characters to UTF8 encoded bytes.
 *
 * The exact form of UTF8 encoding matches the JVM Spec in that \\u0000
 * characters are encoded using the two byte UTF8 form, and the result
 * is terminated with a zero byte.
 *
 * The resulting data is always terminated with a zero byte, even
 * if that means truncating the string.  When truncation occurs, it will
 * always be at a clean UTF8 boundary.
 *
 * Pass dstBytes=0 and dst=NULL to get the number of bytes necessary for
 * encoding including the zero terminating byte.
 *
 * @param src      Source array of Unicode characters
 * @param srcChars Number of source characters to encode (characters not bytes)
 * @param dst      Destination for encoded data
 * @param dstBytes Maximum number of bytes to write to dst (inc terminator)
 * @return Number of bytes required for the encoding or -1 if the string had
 *         to be truncated.
 */
int32_t convertUcs2ToUtf8(const uint16_t* src, int32_t srcChars, char* dst, int32_t dstBytes);

/**
 * Convert UTF8 encoded bytes into an array of Unicode characters.
 *
 * Truncation occurs when the destination array is not big enough,
 * if the source UTF8 data is badly formatted, or if a nul byte is
 * found before srcBytes have been consumed.
 *
 * Unlike some of the other conversion routines, the destination array is
 * not terminated with a \\u0000 character.
 *
 * Pass dst=NULL to get the number of Unicode characters
 * that the UTF8 data decodes to.
 *
 * @param src       Source array of UTF8 encoded bytes
 * @param srcBytes  Number of bytes to decode (-1 means up to first nul byte)
 * @param dst       Destination for decoded characters
 * @param dstBytes  Size of destination array in bytes (not characters)
 *
 * @return Number of characters that the source data decodes to, or
 *  -1 if the destination array is not big enough.
 */
int32_t convertUtf8ToUcs2(const char* src, int32_t srcBytes, uint16_t* dst, int32_t dstBytes);



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
uint32_t dvmComputeUtf8Hash(const char* utf8Str);

#ifdef __cplusplus
}
#endif

#endif //__ENCODING_H__