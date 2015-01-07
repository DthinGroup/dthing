/*
 *  Licensed to the Apache Software Foundation (ASF) under one or more
 *  contributor license agreements.  See the NOTICE file distributed with
 *  this work for additional information regarding copyright ownership.
 *  The ASF licenses this file to You under the Apache License, Version 2.0
 *  (the "License"); you may not use this file except in compliance with
 *  the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

package java.lang;

import java.io.UnsupportedEncodingException;

import com.yarlungsoft.charset.Helper;
import com.yarlungsoft.util.*;


/**
 * An immutable sequence of characters/code units ({@code char}s). A
 * {@code String} is represented by array of UTF-16 values, such that
 * Unicode supplementary characters (code points) are stored/encoded as
 * surrogate pairs via Unicode code units ({@code char}).
 *
 * <a name="backing_array"><h3>Backing Arrays</h3></a>
 * This class is implemented using a char[]. The length of the array may exceed
 * the length of the string. For example, the string "Hello" may be backed by
 * the array {@code ['H', 'e', 'l', 'l', 'o', 'W'. 'o', 'r', 'l', 'd']} with
 * offset 0 and length 5.
 *
 * <p>Multiple strings can share the same char[] because strings are immutable.
 * The {@link #substring} method <strong>always</strong> returns a string that
 * shares the backing array of its source string. Generally this is an
 * optimization: fewer character arrays need to be allocated, and less copying
 * is necessary. But this can also lead to unwanted heap retention. Taking a
 * short substring of long string means that the long shared char[] won't be
 * garbage until both strings are garbage. This typically happens when parsing
 * small substrings out of a large input. To avoid this where necessary, call
 * {@code new String(longString.subString(...))}. The string copy constructor
 * always ensures that the backing array is no larger than necessary.
 *
 * @see StringBuffer
 * @see StringBuilder
 * @see Charset
 * @since 1.0
 */
public final class String implements CharSequence {

    private static final long serialVersionUID = -6849794470754667710L;

    private static final char[] ASCII;
    static {
        ASCII = new char[128];
        for (int i = 0; i < ASCII.length; ++i) {
            ASCII[i] = (char) i;
        }
    }

    char[] value;

    int offset;

    int count;

    private int hashCode;

    /**
     * Creates an empty string.
     */
    public String() {
        value = EmptyArray.CHAR;
        offset = 0;
        count = 0;
    }


    /**
     * Allocates a new string without a values array!
     * Only ever called from StringBuffer.toString() which will
     * assign the array explicitly.
     */
    String(int count) {
        this.count = count;
        this.offset = 0;
        this.value = null;
    }

    /**
     * Converts the byte array to a string using the system's
     * {@link java.nio.charset.Charset#defaultCharset default charset}.
     */
    public String(byte[] data) {
        this(data, 0, data.length);
    }

    /**
     * Converts a subsequence of the byte array to a string using the system's
     * {@link java.nio.charset.Charset#defaultCharset default charset}.
     *
     * @throws NullPointerException
     *             if {@code data == null}.
     * @throws IndexOutOfBoundsException
     *             if {@code byteCount < 0 || offset < 0 || offset + byteCount > data.length}.
     */
    public String(byte[] data, int offset, int byteCount) {
        if ((byteCount | byteCount) < 0 || byteCount > data.length - offset) {
            throw failedBoundsCheck(data.length, offset, byteCount);
        }
        char[] carray = Helper.byteToCharArray(data, offset, byteCount);
        this.value = carray;
        this.count = carray.length;
    }

    /**
     * Converts the byte array to a string using the named charset.
     *
     * <p>The behavior when the bytes cannot be decoded by the named charset
     * is unspecified. Use {@link java.nio.charset.CharsetDecoder} for more control.
     *
     * @throws NullPointerException
     *             if {@code data == null}.
     * @throws IndexOutOfBoundsException
     *             if {@code byteCount < 0 || offset < 0 || offset + byteCount > data.length}.
     * @throws UnsupportedEncodingException
     *             if the named charset is not supported.
     */
    public String(byte[] data, int offset, int byteCount, String charsetName) throws UnsupportedEncodingException {
        if ((byteCount | byteCount) < 0 || byteCount > data.length - offset) {
            throw failedBoundsCheck(data.length, offset, byteCount);
        }
    	char[] carray = Helper.byteToCharArray(data, offset, byteCount, charsetName);
        this.value = carray;
        this.count = carray.length;
    }

    /**
     * Converts the byte array to a string using the named charset.
     *
     * <p>The behavior when the bytes cannot be decoded by the named charset
     * is unspecified. Use {@link java.nio.charset.CharsetDecoder} for more control.
     *
     * @throws NullPointerException
     *             if {@code data == null}.
     * @throws UnsupportedEncodingException
     *             if {@code charsetName} is not supported.
     */
    public String(byte[] data, String charsetName) throws UnsupportedEncodingException {
    	this(data, 0, data.length, charsetName);
    }


    /**
     * Initializes this string to contain the characters in the specified
     * character array. Modifying the character array after creating the string
     * has no effect on the string.
     *
     * @throws NullPointerException if {@code data == null}
     */
    public String(char[] data) {
        this(data, 0, data.length);
    }

    /**
     * Initializes this string to contain the specified characters in the
     * character array. Modifying the character array after creating the string
     * has no effect on the string.
     *
     * @throws NullPointerException
     *             if {@code data == null}.
     * @throws IndexOutOfBoundsException
     *             if {@code charCount < 0 || offset < 0 || offset + charCount > data.length}
     */
    public String(char[] data, int offset, int charCount) {
        if ((offset | charCount) < 0 || charCount > data.length - offset) {
            throw failedBoundsCheck(data.length, offset, charCount);
        }
        this.value = new char[charCount];
        this.count = charCount;
        System.arraycopy(data, offset, value, 0, count);
    }

    /*
     * Internal version of the String(char[], int, int) constructor.
     * Does not range check, null check, or copy the character array.
     */
    String(int offset, int charCount, char[] chars) {
        this.value = chars;
        this.offset = offset;
        this.count = charCount;
    }

    /**
     * Constructs a new string with the same sequence of characters as {@code
     * toCopy}. The returned string's <a href="#backing_array">backing array</a>
     * is no larger than necessary.
     */
    public String(String toCopy) {
        this.count = toCopy.count;
        this.offset = 0;
        if (toCopy.value.length > this.count) {
            // Assume this constructor was called to trim the excess
            this.value = new char[this.count];
            System.arraycopy(toCopy.value, toCopy.offset, this.value, 0, this.count);
        } else {
            this.value = toCopy.value;
        }
    }

    /**
     * Creates a {@code String} from the contents of the specified
     * {@code StringBuffer}.
     */
    public String(StringBuffer stringBuffer) {
    	stringBuffer.shareToString(this);
    }

    /**
     * Returns the character at the specified offset in this string.
     *
     * @param index
     *            the zero-based index in this string.
     * @return the character at the index.
     * @throws IndexOutOfBoundsException
     *             if {@code index < 0} or {@code index >= length()}.
     */
    public char charAt(int index) {
    	if (index >= 0 && index < count) {
    		return value[index + offset];
    	}
    	throw new StringIndexOutOfBoundsException();
    }

    private StringIndexOutOfBoundsException indexAndLength(int index) {
        throw new StringIndexOutOfBoundsException(this, index);
    }

    private StringIndexOutOfBoundsException startEndAndLength(int start, int end) {
        throw new StringIndexOutOfBoundsException(this, start, end - start);
    }

    private StringIndexOutOfBoundsException failedBoundsCheck(int arrayLength, int offset, int count) {
        throw new StringIndexOutOfBoundsException(arrayLength, offset, count);
    }

    /**
     * This isn't equivalent to either of ICU's u_foldCase case folds, and thus any of the Unicode
     * case folds, but it's what the RI uses.
     */
    private char foldCase(char ch) {
        if (ch < 128) {
            if ('A' <= ch && ch <= 'Z') {
                return (char) (ch + ('a' - 'A'));
            }
            return ch;
        }
        return Character.toLowerCase(Character.toUpperCase(ch));
    }

    /**
     * Compares the specified string to this string using the Unicode values of
     * the characters. Returns 0 if the strings contain the same characters in
     * the same order. Returns a negative integer if the first non-equal
     * character in this string has a Unicode value which is less than the
     * Unicode value of the character at the same position in the specified
     * string, or if this string is a prefix of the specified string. Returns a
     * positive integer if the first non-equal character in this string has a
     * Unicode value which is greater than the Unicode value of the character at
     * the same position in the specified string, or if the specified string is
     * a prefix of this string.
     *
     * @param string
     *            the string to compare.
     * @return 0 if the strings are equal, a negative integer if this string is
     *         before the specified string, or a positive integer if this string
     *         is after the specified string.
     * @throws NullPointerException
     *             if {@code string} is {@code null}.
     */
    public int compareTo(String string) {
        if (string == null) {
            throw new NullPointerException();
        }

        if (this != string) {
            int n = Math.min(this.count, string.count);
            char v1[] = this.value;
            char v2[] = string.value;
            int i = this.offset;
            int j = string.offset;

            while (--n >= 0) {
                int diff = v1[i] - v2[j];
                if (diff == 0) {
                    i++; j++;
                } else {
                    return diff;
                }
            }
            return this.count - string.count;
        }
        return 0;
    }

    /**
     * Compares the specified string to this string using the Unicode values of
     * the characters, ignoring case differences. Returns 0 if the strings
     * contain the same characters in the same order. Returns a negative integer
     * if the first non-equal character in this string has a Unicode value which
     * is less than the Unicode value of the character at the same position in
     * the specified string, or if this string is a prefix of the specified
     * string. Returns a positive integer if the first non-equal character in
     * this string has a Unicode value which is greater than the Unicode value
     * of the character at the same position in the specified string, or if the
     * specified string is a prefix of this string.
     *
     * @param string
     *            the string to compare.
     * @return 0 if the strings are equal, a negative integer if this string is
     *         before the specified string, or a positive integer if this string
     *         is after the specified string.
     * @throws NullPointerException
     *             if {@code string} is {@code null}.
     */
    public int compareToIgnoreCase(String string) {
        int o1 = offset, o2 = string.offset, result;
        int end = offset + (count < string.count ? count : string.count);
        char c1, c2;
        char[] target = string.value;
        while (o1 < end) {
            if ((c1 = value[o1++]) == (c2 = target[o2++])) {
                continue;
            }
            c1 = foldCase(c1);
            c2 = foldCase(c2);
            if ((result = c1 - c2) != 0) {
                return result;
            }
        }
        return count - string.count;
    }

    /**
     * Concatenates this string and the specified string.
     *
     * @param string
     *            the string to concatenate
     * @return a new string which is the concatenation of this string and the
     *         specified string.
     */
    public String concat(String string) {
        if (string.count > 0 && count > 0) {
            char[] buffer = new char[count + string.count];
            System.arraycopy(value, offset, buffer, 0, count);
            System.arraycopy(string.value, string.offset, buffer, count, string.count);
            return new String(0, buffer.length, buffer);
        }
        return count == 0 ? string : this;
    }

    /**
     * Creates a new string containing the characters in the specified character
     * array. Modifying the character array after creating the string has no
     * effect on the string.
     *
     * @param data
     *            the array of characters.
     * @return the new string.
     * @throws NullPointerException
     *             if {@code data} is {@code null}.
     */
    public static String copyValueOf(char[] data) {
        return new String(data, 0, data.length);
    }

    /**
     * Creates a new string containing the specified characters in the character
     * array. Modifying the character array after creating the string has no
     * effect on the string.
     *
     * @param data
     *            the array of characters.
     * @param start
     *            the starting offset in the character array.
     * @param length
     *            the number of characters to use.
     * @return the new string.
     * @throws NullPointerException
     *             if {@code data} is {@code null}.
     * @throws IndexOutOfBoundsException
     *             if {@code length < 0, start < 0} or {@code start + length >
     *             data.length}.
     */
    public static String copyValueOf(char[] data, int start, int length) {
        return new String(data, start, length);
    }

    /**
     * Compares the specified string to this string to determine if the
     * specified string is a suffix.
     *
     * @param suffix
     *            the suffix to look for.
     * @return {@code true} if the specified string is a suffix of this string,
     *         {@code false} otherwise.
     * @throws NullPointerException
     *             if {@code suffix} is {@code null}.
     */
    public boolean endsWith(String suffix) {
        return regionMatches(count - suffix.count, suffix, 0, suffix.count);
    }

    /**
     * Compares the specified object to this string and returns true if they are
     * equal. The object must be an instance of string with the same characters
     * in the same order.
     *
     * @param object
     *            the object to compare.
     * @return {@code true} if the specified object is equal to this string,
     *         {@code false} otherwise.
     * @see #hashCode
     */
    //@Override
    public boolean equals(Object object) {
    	if (this == object) {
    	    return true;
    	}

    	if (object instanceof String) {
    	    String anotherString = (String)object;
    	    int n = count;
    	    if (n == anotherString.count) {
    	        char v1[] = value;
    	        char v2[] = anotherString.value;
    	        int i = offset;
    	        int j = anotherString.offset;
    	        while (n-- != 0) {
    	            if (v1[i] != v2[j]) {
    	                return false;
    	            }
    	            i++;
    	            j++;
    	        }
    	        return true;
    	    }
    	}
    	return false;
    }

    /**
     * Compares the specified string to this string ignoring the case of the
     * characters and returns true if they are equal.
     *
     * @param string
     *            the string to compare.
     * @return {@code true} if the specified string is equal to this string,
     *         {@code false} otherwise.
     */
    public boolean equalsIgnoreCase(String string) {
        if (string == this) {
            return true;
        }
        if (string == null || count != string.count) {
            return false;
        }
        int o1 = offset, o2 = string.offset;
        int end = offset + count;
        char[] target = string.value;
        while (o1 < end) {
            char c1 = value[o1++];
            char c2 = target[o2++];
            if (c1 != c2 && foldCase(c1) != foldCase(c2)) {
                return false;
            }
        }
        return true;
    }

    /**
     * Returns a new byte array containing the characters of this string encoded using the
     * system's {@link java.nio.charset.Charset#defaultCharset default charset}.
     *
     * <p>The behavior when this string cannot be represented in the system's default charset
     * is unspecified. In practice, when the default charset is UTF-8 (as it is on Android),
     * all strings can be encoded.
     */
    public byte[] getBytes() {
    	return Helper.charToByteArray(value, offset, count);
    }

    /**
     * Returns a new byte array containing the characters of this string encoded using the
     * named charset.
     *
     * <p>The behavior when this string cannot be represented in the named charset
     * is unspecified. Use {@link java.nio.charset.CharsetEncoder} for more control.
     *
     * @throws UnsupportedEncodingException if the charset is not supported
     */
    public byte[] getBytes(String charsetName) throws UnsupportedEncodingException {
    	return Helper.charToByteArray(value, offset, count, charsetName);
    }

    /**
     * Copies the specified characters in this string to the character array
     * starting at the specified offset in the character array.
     *
     * @param start
     *            the starting offset of characters to copy.
     * @param end
     *            the ending offset of characters to copy.
     * @param buffer
     *            the destination character array.
     * @param index
     *            the starting offset in the character array.
     * @throws NullPointerException
     *             if {@code buffer} is {@code null}.
     * @throws IndexOutOfBoundsException
     *             if {@code start < 0}, {@code end > length()}, {@code start >
     *             end}, {@code index < 0}, {@code end - start > buffer.length -
     *             index}
     */
    public void getChars(int start, int end, char[] buffer, int index) {
        // Note: last character not copied!
        if (start >= 0 && start <= end && end <= count) {
            System.arraycopy(value, start + offset, buffer, index, end - start);
        } else {
            // We throw StringIndexOutOfBoundsException rather than System.arraycopy's AIOOBE.
            throw startEndAndLength(start, end);
        }
    }

    /**
     * Returns a hashcode for this string. The hashcode for a
     * <code>String</code> object is computed as
     * <blockquote><pre>
     * s[0]*31^(n-1) + s[1]*31^(n-2) + ... + s[n-1]
     * </pre></blockquote>
     * using <code>int</code> arithmetic, where <code>s[i]</code> is the
     * <i>i</i>th character of the string, <code>n</code> is the length of
     * the string, and <code>^</code> indicates exponentiation.
     * (The hash value of the empty string is zero.)
     *
     * @return  a hash code value for this object.
     */
    //@Override
    public int hashCode() {
        int hash = hashCode;
        if (hash == 0) {
            if (count == 0) {
                return 0;
            }
            final int end = count + offset;
            final char[] chars = value;
            for (int i = offset; i < end; ++i) {
                hash = 31*hash + chars[i];
            }
            hashCode = hash;
        }
        return hash;
    }

    /**
     * Searches in this string for the first index of the specified character.
     * The search for the character starts at the beginning and moves towards
     * the end of this string.
     *
     * @param c
     *            the character to find.
     * @return the index in this string of the specified character, -1 if the
     *         character isn't found.
     */
    public int indexOf(int c) {
    	return indexOf(c, 0);
    }

    /**
     * Searches in this string for the index of the specified character. The
     * search for the character starts at the specified offset and moves towards
     * the end of this string.
     *
     * @param c
     *            the character to find.
     * @param start
     *            the starting offset.
     * @return the index in this string of the specified character, -1 if the
     *         character isn't found.
     */
    public int indexOf(int c, int start) {
        int max = offset + count;
        char v[] = value;

        if (start < 0) {
        	start = 0;
        } else if (start >= count) {
            // Note: fromIndex might be near -1>>>1.
            return -1;
        }
        for (int i = offset + start ; i < max ; i++) {
            if (v[i] == c) {
                return i - offset;
            }
        }
        return -1;
    }

    /**
     * Searches in this string for the first index of the specified string. The
     * search for the string starts at the beginning and moves towards the end
     * of this string.
     *
     * @param string
     *            the string to find.
     * @return the index of the first character of the specified string in this
     *         string, -1 if the specified string is not a substring.
     * @throws NullPointerException
     *             if {@code string} is {@code null}.
     */
    public int indexOf(String string) {
        int start = 0;
        int subCount = string.count;
        int _count = count;
        if (subCount > 0) {
            if (subCount > _count) {
                return -1;
            }
            char[] target = string.value;
            int subOffset = string.offset;
            char firstChar = target[subOffset];
            int end = subOffset + subCount;
            while (true) {
                int i = indexOf(firstChar, start);
                if (i == -1 || subCount + i > _count) {
                    return -1; // handles subCount > count || start >= count
                }
                int o1 = offset + i, o2 = subOffset;
                char[] _value = value;
                while (++o2 < end && _value[++o1] == target[o2]) {
                    // Intentionally empty
                }
                if (o2 == end) {
                    return i;
                }
                start = i + 1;
            }
        }
        return start < _count ? start : _count;
    }

    /**
     * Searches in this string for the index of the specified string. The search
     * for the string starts at the specified offset and moves towards the end
     * of this string.
     *
     * @param subString
     *            the string to find.
     * @param start
     *            the starting offset.
     * @return the index of the first character of the specified string in this
     *         string, -1 if the specified string is not a substring.
     * @throws NullPointerException
     *             if {@code subString} is {@code null}.
     */
    public int indexOf(String subString, int start) {
        if (start < 0) {
            start = 0;
        }
        int subCount = subString.count;
        int _count = count;
        if (subCount > 0) {
            if (subCount + start > _count) {
                return -1;
            }
            char[] target = subString.value;
            int subOffset = subString.offset;
            char firstChar = target[subOffset];
            int end = subOffset + subCount;
            while (true) {
                int i = indexOf(firstChar, start);
                if (i == -1 || subCount + i > _count) {
                    return -1; // handles subCount > count || start >= count
                }
                int o1 = offset + i, o2 = subOffset;
                char[] _value = value;
                while (++o2 < end && _value[++o1] == target[o2]) {
                    // Intentionally empty
                }
                if (o2 == end) {
                    return i;
                }
                start = i + 1;
            }
        }
        return start < _count ? start : _count;
    }

    /**
     * Code to do searches shared between String and StringBuffer. The
     * source is the character array being searched, and the target
     * is the string being searched for. Both character arrays are known
     * to be non-null.
     *
     * @param   source       the characters being searched.
     * @param   sourceOffset offset of the source string.
     * @param   sourceCount  count of the source string.
     * @param   target       the characters being searched for.
     * @param   targetOffset offset of the target string.
     * @param   targetCount  count of the target string.
     * @param   fromIndex    the index to begin searching from.
     */
    static int indexOf(char[] source, int sourceOffset, int sourceCount,
                       char[] target, int targetOffset, int targetCount,
                       int fromIndex) {

        if (fromIndex < sourceCount) {
            if (fromIndex < 0) {
                fromIndex = 0;
            }
            if (targetCount == 0) {
                // An empty string matches immediately
                return fromIndex;
            }

            char first  = target[targetOffset];
            int max = sourceOffset + (sourceCount - targetCount);

            // subtract 1 and drop into loop which will add one
            fromIndex += sourceOffset - 1;

        startSearchForFirstChar:
            while (true) {
                do {
                    fromIndex++;
                    if (fromIndex > max) {
                        break startSearchForFirstChar;
                    }
                } while (source[fromIndex] != first);

                // Found first character, now look at the rest of target
                for(int cnt = targetCount; --cnt > 0; ) {
                    if (source[fromIndex + cnt] !=
                        target[targetOffset + cnt]) {
                        continue startSearchForFirstChar;
                    }
                }

                return fromIndex - sourceOffset;  // Found whole string.
            }
        }
        else if (targetCount == 0) {
            // There is an empty string at the end of any string.
            // This is specifically checked for by the CDC 1.1 TCK
            // String.IndexOf1Tests (subtest String1101).
            return sourceCount;
        }

        return -1;
    }

    /**
     * Returns an interned string equal to this string. The VM maintains an internal set of
     * unique strings. All string literals found in loaded classes'
     * constant pools are automatically interned. Manually-interned strings are only weakly
     * referenced, so calling {@code intern} won't lead to unwanted retention.
     *
     * <p>Interning is typically used because it guarantees that for interned strings
     * {@code a} and {@code b}, {@code a.equals(b)} can be simplified to
     * {@code a == b}. (This is not true of non-interned strings.)
     *
     * <p>Many applications find it simpler and more convenient to use an explicit
     * {@link java.util.HashMap} to implement their own pools.
     */
    public native String intern();

    /**
     * Returns the last index of the code point {@code c}, or -1.
     * The search for the character starts at the end and moves towards the
     * beginning of this string.
     */
    public int lastIndexOf(int c) {

        int _count = count;
        int _offset = offset;
        char[] _value = value;
        for (int i = _offset + _count - 1; i >= _offset; --i) {
            if (_value[i] == c) {
                return i - _offset;
            }
        }
        return -1;
    }

    /**
     * Returns the last index of the code point {@code c}, or -1.
     * The search for the character starts at offset {@code start} and moves towards
     * the beginning of this string.
     */
    public int lastIndexOf(int c, int start) {

        int _count = count;
        int _offset = offset;
        char[] _value = value;
        if (start >= 0) {
            if (start >= _count) {
                start = _count - 1;
            }
            for (int i = _offset + start; i >= _offset; --i) {
                if (_value[i] == c) {
                    return i - _offset;
                }
            }
        }
        return -1;
    }


    /**
     * Searches in this string for the last index of the specified string. The
     * search for the string starts at the end and moves towards the beginning
     * of this string.
     *
     * @param string
     *            the string to find.
     * @return the index of the first character of the specified string in this
     *         string, -1 if the specified string is not a substring.
     * @throws NullPointerException
     *             if {@code string} is {@code null}.
     */
    public int lastIndexOf(String string) {
        // Use count instead of count - 1 so lastIndexOf("") returns count
        return lastIndexOf(string, count);
    }

    /**
     * Searches in this string for the index of the specified string. The search
     * for the string starts at the specified offset and moves towards the
     * beginning of this string.
     *
     * @param subString
     *            the string to find.
     * @param start
     *            the starting offset.
     * @return the index of the first character of the specified string in this
     *         string , -1 if the specified string is not a substring.
     * @throws NullPointerException
     *             if {@code subString} is {@code null}.
     */
    public int lastIndexOf(String subString, int start) {
        int subCount = subString.count;
        if (subCount <= count && start >= 0) {
            if (subCount > 0) {
                if (start > count - subCount) {
                    start = count - subCount;
                }
                // count and subCount are both >= 1
                char[] target = subString.value;
                int subOffset = subString.offset;
                char firstChar = target[subOffset];
                int end = subOffset + subCount;
                while (true) {
                    int i = lastIndexOf(firstChar, start);
                    if (i == -1) {
                        return -1;
                    }
                    int o1 = offset + i, o2 = subOffset;
                    while (++o2 < end && value[++o1] == target[o2]) {
                        // Intentionally empty
                    }
                    if (o2 == end) {
                        return i;
                    }
                    start = i - 1;
                }
            }
            return start < count ? start : count;
        }
        return -1;
    }

    /**
     * Code to do searches shared between String and StringBuffer. The source
     * is the character array being searched, and the target is the string
     * being searched for. Both character arrays are known to be non-null.
     *
     * @param   source       the characters being searched.
     * @param   sourceOffset offset of the source string.
     * @param   sourceCount  count of the source string.
     * @param   target       the characters being searched for.
     * @param   targetOffset offset of the target string.
     * @param   targetCount  count of the target string.
     * @param   fromIndex    the index to begin searching from.
     */
    static int lastIndexOf(
        char[] source, int sourceOffset, int sourceCount,
        char[] target, int targetOffset, int targetCount,
        int fromIndex) {

	        if (fromIndex < 0)
	            return -1;

	        if (fromIndex > (sourceCount - targetCount))
	            fromIndex = sourceCount - targetCount;

	        if (targetCount == 0)
	            return fromIndex;   // empty string matches immediately

	        outerloop:
	        for (; fromIndex >= 0; fromIndex--) {
	            for (int i = 0, so = sourceOffset + fromIndex, to = targetOffset;
	                    i < targetCount;
	                    i++, so++, to++) {
	                if (source[so] != target[to]) {
	                    continue outerloop;
	                }
	            }
	            return fromIndex;
	        }
	        return -1;

    }

    /**
     * Returns the size of this string.
     *
     * @return the number of characters in this string.
     */
    public int length() {
    	return count;
    }

    /**
     * Compares the specified string to this string and compares the specified
     * range of characters to determine if they are the same.
     *
     * @param thisStart
     *            the starting offset in this string.
     * @param string
     *            the string to compare.
     * @param start
     *            the starting offset in the specified string.
     * @param length
     *            the number of characters to compare.
     * @return {@code true} if the ranges of characters are equal, {@code false}
     *         otherwise
     * @throws NullPointerException
     *             if {@code string} is {@code null}.
     */
    public boolean regionMatches(int thisStart, String string, int start, int length) {
        if (string == null) {
            throw new NullPointerException();
        }
        if (start < 0 || string.count - start < length) {
            return false;
        }
        if (thisStart < 0 || count - thisStart < length) {
            return false;
        }
        if (length <= 0) {
            return true;
        }
        int o1 = offset + thisStart, o2 = string.offset + start;
        char[] value1 = value;
        char[] value2 = string.value;
        for (int i = 0; i < length; ++i) {
            if (value1[o1 + i] != value2[o2 + i]) {
                return false;
            }
        }
        return true;
    }

    /**
     * Compares the specified string to this string and compares the specified
     * range of characters to determine if they are the same. When ignoreCase is
     * true, the case of the characters is ignored during the comparison.
     *
     * @param ignoreCase
     *            specifies if case should be ignored.
     * @param thisStart
     *            the starting offset in this string.
     * @param string
     *            the string to compare.
     * @param start
     *            the starting offset in the specified string.
     * @param length
     *            the number of characters to compare.
     * @return {@code true} if the ranges of characters are equal, {@code false}
     *         otherwise.
     * @throws NullPointerException
     *             if {@code string} is {@code null}.
     */
    public boolean regionMatches(boolean ignoreCase, int thisStart, String string, int start, int length) {
        if (!ignoreCase) {
            return regionMatches(thisStart, string, start, length);
        }
        if (string == null) {
            throw new NullPointerException("string == null");
        }
        if (thisStart < 0 || length > count - thisStart) {
            return false;
        }
        if (start < 0 || length > string.count - start) {
            return false;
        }
        thisStart += offset;
        start += string.offset;
        int end = thisStart + length;
        char[] target = string.value;
        while (thisStart < end) {
            char c1 = value[thisStart++];
            char c2 = target[start++];
            if (c1 != c2 && foldCase(c1) != foldCase(c2)) {
                return false;
            }
        }
        return true;
    }

    /**
     * Copies this string replacing occurrences of the specified character with
     * another character.
     *
     * @param oldChar
     *            the character to replace.
     * @param newChar
     *            the replacement character.
     * @return a new string with occurrences of oldChar replaced by newChar.
     */
    public String replace(char oldChar, char newChar) {
        char[] buffer = value;
        int _offset = offset;
        int _count = count;

        int idx = _offset;
        int last = _offset + _count;
        boolean copied = false;
        while (idx < last) {
            if (buffer[idx] == oldChar) {
                if (!copied) {
                    char[] newBuffer = new char[_count];
                    System.arraycopy(buffer, _offset, newBuffer, 0, _count);
                    buffer = newBuffer;
                    idx -= _offset;
                    last -= _offset;
                    copied = true;
                }
                buffer[idx] = newChar;
            }
            idx++;
        }

        return copied ? new String(0, count, buffer) : this;
    }

    /**
     * Copies this string replacing occurrences of the specified target sequence
     * with another sequence. The string is processed from the beginning to the
     * end.
     *
     * @param target
     *            the sequence to replace.
     * @param replacement
     *            the replacement sequence.
     * @return the resulting string.
     * @throws NullPointerException
     *             if {@code target} or {@code replacement} is {@code null}.
     */
    public String replace(CharSequence target, CharSequence replacement) {
        if (target == null) {
            throw new NullPointerException("target == null");
        }
        if (replacement == null) {
            throw new NullPointerException("replacement == null");
        }

        String targetString = target.toString();
        int matchStart = indexOf(targetString, 0);
        if (matchStart == -1) {
            // If there's nothing to replace, return the original string untouched.
            return this;
        }

        String replacementString = replacement.toString();

        // The empty target matches at the start and end and between each character.
        int targetLength = targetString.length();
        if (targetLength == 0) {
            int resultLength = (count + 2) * replacementString.length();
            StringBuilder result = new StringBuilder(resultLength);
            result.append(replacementString);
            for (int i = offset; i < count; ++i) {
                result.append(value[i]);
                result.append(replacementString);
            }
            return result.toString();
        }

        StringBuilder result = new StringBuilder(count);
        int searchStart = 0;
        do {
            // Copy characters before the match...
            result.append(value, offset + searchStart, matchStart - searchStart);
            // Insert the replacement...
            result.append(replacementString);
            // And skip over the match...
            searchStart = matchStart + targetLength;
        } while ((matchStart = indexOf(targetString, searchStart)) != -1);
        // Copy any trailing chars...
        result.append(value, offset + searchStart, count - searchStart);
        return result.toString();
    }

    /**
     * Compares the specified string to this string to determine if the
     * specified string is a prefix.
     *
     * @param prefix
     *            the string to look for.
     * @return {@code true} if the specified string is a prefix of this string,
     *         {@code false} otherwise
     * @throws NullPointerException
     *             if {@code prefix} is {@code null}.
     */
    public boolean startsWith(String prefix) {
        return startsWith(prefix, 0);
    }

    /**
     * Compares the specified string to this string, starting at the specified
     * offset, to determine if the specified string is a prefix.
     *
     * @param prefix
     *            the string to look for.
     * @param start
     *            the starting offset.
     * @return {@code true} if the specified string occurs in this string at the
     *         specified offset, {@code false} otherwise.
     * @throws NullPointerException
     *             if {@code prefix} is {@code null}.
     */
    public boolean startsWith(String prefix, int start) {
        return regionMatches(start, prefix, 0, prefix.count);
    }

    /**
     * Returns a string containing a suffix of this string. The returned string
     * shares this string's <a href="#backing_array">backing array</a>.
     *
     * @param start
     *            the offset of the first character.
     * @return a new string containing the characters from start to the end of
     *         the string.
     * @throws IndexOutOfBoundsException
     *             if {@code start < 0} or {@code start > length()}.
     */
    public String substring(int start) {
        if (start == 0) {
            return this;
        }
        if (start >= 0 && start <= count) {
            return new String(offset + start, count - start, value);
        }
        throw indexAndLength(start);
    }

    /**
     * Returns a string containing a subsequence of characters from this string.
     * The returned string shares this string's <a href="#backing_array">backing
     * array</a>.
     *
     * @param start
     *            the offset of the first character.
     * @param end
     *            the offset one past the last character.
     * @return a new string containing the characters from start to end - 1
     * @throws IndexOutOfBoundsException
     *             if {@code start < 0}, {@code start > end} or {@code end >
     *             length()}.
     */
    public String substring(int start, int end) {
        if (start == 0 && end == count) {
            return this;
        }
        // NOTE last character not copied!
        // Fast range check.
        if (start >= 0 && start <= end && end <= count) {
            return new String(offset + start, end - start, value);
        }
        throw startEndAndLength(start, end);
    }

    /**
     * Copies the characters in this string to a character array.
     *
     * @return a character array containing the characters of this string.
     */
    public char[] toCharArray() {
        char[] buffer = new char[count];
        System.arraycopy(value, offset, buffer, 0, count);
        return buffer;
    }

    /**
     * Converts this string to lower case, using the rules of the user's default locale.
     * See "<a href="../util/Locale.html#default_locale">Be wary of the default locale</a>".
     *
     * @return a new lower case string, or {@code this} if it's already all lower case.
     */
    public String toLowerCase() {
        // Check for original string already in lower case.
        // One test suite checks for this (although not part of CLDC spec)

        int idx = this.count;
        do {
            if (--idx < 0) {
                return this;
            }
        } while (Character.isLowerCase(value[offset+idx]));

        // Copy the section of the original string that is already lower case

        int end = this.count;
        char[] buf = new char[end];
        while (--end > idx) {
            buf[end] = value[offset+end];
        }

        // Convert the rest (we know that (idx >= 0) since there is at least
        // one character to convert.

        do {
            buf[idx] = Character.toLowerCase(value[offset+idx]);
        } while (--idx >= 0);

        return new String(0, this.count, buf);
    }



    /**
     * Returns this string.
     */
    //@Override
    public String toString() {
        return this;
    }

    /**
     * Converts this this string to upper case, using the rules of the user's default locale.
     * See "<a href="../util/Locale.html#default_locale">Be wary of the default locale</a>".
     *
     * @return a new upper case string, or {@code this} if it's already all upper case.
     */
    public String toUpperCase() {
        // Check for original string already in lower case.
        // One test suite checks for this (although not part of CLDC spec)

        int idx = this.count;
        do {
            if (--idx < 0) {
                return this;
            }
        } while (Character.isLowerCase(value[offset+idx]));

        // Copy the section of the original string that is already lower case

        int end = this.count;
        char[] buf = new char[end];
        while (--end > idx) {
            buf[end] = value[offset+end];
        }

        // Convert the rest (we know that (idx >= 0) since there is at least
        // one character to convert.

        do {
            buf[idx] = Character.toLowerCase(value[offset+idx]);
        } while (--idx >= 0);
        return new String(0, this.count, buf);
    }



    /**
     * Copies this string removing white space characters from the beginning and
     * end of the string.
     *
     * @return a new string with characters <code><= \\u0020</code> removed from
     *         the beginning and the end.
     */
    public String trim() {
        int start = offset, last = offset + count - 1;
        int end = last;
        while ((start <= end) && (value[start] <= ' ')) {
            start++;
        }
        while ((end >= start) && (value[end] <= ' ')) {
            end--;
        }
        if (start == offset && end == last) {
            return this;
        }
        return new String(start, end - start + 1, value);
    }

    /**
     * Creates a new string containing the characters in the specified character
     * array. Modifying the character array after creating the string has no
     * effect on the string.
     *
     * @param data
     *            the array of characters.
     * @return the new string.
     * @throws NullPointerException
     *             if {@code data} is {@code null}.
     */
    public static String valueOf(char[] data) {
        return new String(data, 0, data.length);
    }

    /**
     * Creates a new string containing the specified characters in the character
     * array. Modifying the character array after creating the string has no
     * effect on the string.
     *
     * @param data
     *            the array of characters.
     * @param start
     *            the starting offset in the character array.
     * @param length
     *            the number of characters to use.
     * @return the new string.
     * @throws IndexOutOfBoundsException
     *             if {@code length < 0}, {@code start < 0} or {@code start +
     *             length > data.length}
     * @throws NullPointerException
     *             if {@code data} is {@code null}.
     */
    public static String valueOf(char[] data, int start, int length) {
        return new String(data, start, length);
    }

    /**
     * Converts the specified character to its string representation.
     *
     * @param value
     *            the character.
     * @return the character converted to a string.
     */
    public static String valueOf(char value) {
        String s;
        if (value < 128) {
            s = new String(value, 1, ASCII);
        } else {
            s = new String(0, 1, new char[] { value });
        }
        s.hashCode = value;
        return s;
    }

    /**
     * Converts the specified double to its string representation.
     *
     * @param value
     *            the double.
     * @return the double converted to a string.
     */
    public static String valueOf(double value) {
        return Double.toString(value);
    }

    /**
     * Converts the specified float to its string representation.
     *
     * @param value
     *            the float.
     * @return the float converted to a string.
     */
    public static String valueOf(float value) {
        return Float.toString(value);
    }

    /**
     * Converts the specified integer to its string representation.
     *
     * @param value
     *            the integer.
     * @return the integer converted to a string.
     */
    public static String valueOf(int value) {
        return Integer.toString(value);
    }

    /**
     * Converts the specified long to its string representation.
     *
     * @param value
     *            the long.
     * @return the long converted to a string.
     */
    public static String valueOf(long value) {
        return Long.toString(value);
    }

    /**
     * Converts the specified object to its string representation. If the object
     * is null return the string {@code "null"}, otherwise use {@code
     * toString()} to get the string representation.
     *
     * @param value
     *            the object.
     * @return the object converted to a string, or the string {@code "null"}.
     */
    public static String valueOf(Object value) {
        return value != null ? value.toString() : "null";
    }

    /**
     * Converts the specified boolean to its string representation. When the
     * boolean is {@code true} return {@code "true"}, otherwise return {@code
     * "false"}.
     *
     * @param value
     *            the boolean.
     * @return the boolean converted to a string.
     */
    public static String valueOf(boolean value) {
        return value ? "true" : "false";
    }

    /**
     * Returns whether the characters in the StringBuffer {@code strbuf} are the
     * same as those in this string.
     *
     * @param strbuf
     *            the StringBuffer to compare this string to.
     * @return {@code true} if the characters in {@code strbuf} are identical to
     *         those in this string. If they are not, {@code false} will be
     *         returned.
     * @throws NullPointerException
     *             if {@code strbuf} is {@code null}.
     * @since 1.4
     */
    public boolean contentEquals(StringBuffer strbuf) {
        synchronized (strbuf) {
            int size = strbuf.length();
            if (count != size) {
                return false;
            }
            return regionMatches(0, new String(0, size, strbuf.getValue()), 0,
                    size);
        }
    }

    /**
     * Compares a {@code CharSequence} to this {@code String} to determine if
     * their contents are equal.
     *
     * @param cs
     *            the character sequence to compare to.
     * @return {@code true} if equal, otherwise {@code false}
     * @since 1.5
     */
    public boolean contentEquals(CharSequence cs) {
        if (cs == null) {
            throw new NullPointerException();
        }

        int len = cs.length();

        if (len != count) {
            return false;
        }

        if (len == 0 && count == 0) {
            return true; // since both are empty strings
        }

        return regionMatches(0, cs.toString(), 0, len);
    }

	//@Override
	public CharSequence subSequence(int start, int end) {
		return this.substring(start, end);
	}
}
