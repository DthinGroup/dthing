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

/**
 * The wrapper for the primitive type {@code long}.
 * <p>
 * Implementation note: The "bit twiddling" methods in this class use techniques
 * described in <a href="http://www.hackersdelight.org/">Henry S. Warren,
 * Jr.'s Hacker's Delight, (Addison Wesley, 2002)</a> and <a href=
 * "http://graphics.stanford.edu/~seander/bithacks.html">Sean Anderson's
 * Bit Twiddling Hacks.</a>
 *
 * @see java.lang.Integer
 * @since 1.0
 */
public final class Long {

    private static final long serialVersionUID = 4290774380558885855L;

    /**
     * The value which the receiver represents.
     */
    private final long value;

    /**
     * Constant for the maximum {@code long} value, 2<sup>63</sup>-1.
     */
    public static final long MAX_VALUE = 0x7FFFFFFFFFFFFFFFL;

    /**
     * Constant for the minimum {@code long} value, -2<sup>63</sup>.
     */
    public static final long MIN_VALUE = 0x8000000000000000L;

    /**
     * Constant for the number of bits needed to represent a {@code long} in
     * two's complement form.
     *
     * @since 1.5
     */
    public static final int SIZE = 64;

    /**
     * Constructs a new {@code Long} with the specified primitive long value.
     *
     * @param value
     *            the primitive long value to store in the new instance.
     */
    public Long(long value) {
        this.value = value;
    }

    /**
     * Returns the value of this Long as a byte.
     *
     * @since	JDK1.1
     */
    public byte byteValue() {
        return (byte) value;
    }

    private static NumberFormatException invalidLong(String s) {
        throw new NumberFormatException("Invalid long: \"" + s + "\"");
    }

    /**
     * Parses the specified string and returns a {@code Long} instance if the
     * string can be decoded into a long value. The string may be an optional
     * minus sign "-" followed by a hexadecimal ("0x..." or "#..."), octal
     * ("0..."), or decimal ("...") representation of a long.
     *
     * @param string
     *            a string representation of a long value.
     * @return a {@code Long} containing the value represented by {@code string}.
     * @throws NumberFormatException
     *             if {@code string} cannot be parsed as a long value.
     */
    public static Long decode(String string) throws NumberFormatException {
        int length = string.length(), i = 0;
        if (length == 0) {
            throw invalidLong(string);
        }
        char firstDigit = string.charAt(i);
        boolean negative = firstDigit == '-';
        if (negative) {
            if (length == 1) {
                throw invalidLong(string);
            }
            firstDigit = string.charAt(++i);
        }

        int base = 10;
        if (firstDigit == '0') {
            if (++i == length) {
                return valueOf(0L);
            }
            if ((firstDigit = string.charAt(i)) == 'x' || firstDigit == 'X') {
                if (i == length) {
                    throw invalidLong(string);
                }
                i++;
                base = 16;
            } else {
                base = 8;
            }
        } else if (firstDigit == '#') {
            if (i == length) {
                throw invalidLong(string);
            }
            i++;
            base = 16;
        }

        long result = parse(string, i, base, negative);
        return valueOf(result);
    }

    /**
     * Returns the value of this Long as a double value.
     *
     * @return  the <code>double</code> value represented by this object.
     */
    public double doubleValue() {
        return value;
    }

    /**
     * Compares this instance with the specified object and indicates if they
     * are equal. In order to be equal, {@code o} must be an instance of
     * {@code Long} and have the same long value as this object.
     *
     * @param o
     *            the object to compare this long with.
     * @return {@code true} if the specified object is equal to this
     *         {@code Long}; {@code false} otherwise.
     */
    @Override
    public boolean equals(Object o) {
        return (o instanceof Long) && (((Long) o).value == value);
    }

    /**
     * Returns the value of this Long as a float value.
     *
     * @return  the <code>float</code> value represented by this object.
     */
    public float floatValue() {
        return value;
    }

    /**
     * Computes a hashcode for this Long. The result is the exclusive
     * OR of the two halves of the primitive <code>long</code> value
     * represented by this <code>Long</code> object. That is, the hashcode
     * is the value of the expression:
     * <blockquote><pre>
     * (int)(this.longValue()^(this.longValue()>>>32))
     * </pre></blockquote>
     *
     * @return  a hash code value for this object.
     */
    @Override
    public int hashCode() {
        return (int) (value ^ (value >>> 32));
    }

    /**
     * Returns the value of this Long as a int value.
     *
     * @return  the <code>int</code> value represented by this object.
     */
    public int intValue() {
        return (int) value;
    }

    /**
     * Returns the value of this Long as a long value.
     *
     * @return  the <code>long</code> value represented by this object.
     */
    public long longValue() {
        return value;
    }

    /**
     * Parses the specified string as a signed decimal long value. The ASCII
     * character \u002d ('-') is recognized as the minus sign.
     *
     * @param string
     *            the string representation of a long value.
     * @return the primitive long value represented by {@code string}.
     * @throws NumberFormatException
     *             if {@code string} cannot be parsed as a long value.
     */
    public static long parseLong(String string) throws NumberFormatException {
        return parseLong(string, 10);
    }

    /**
     * Parses the specified string as a signed long value using the specified
     * radix. The ASCII character \u002d ('-') is recognized as the minus sign.
     *
     * @param string
     *            the string representation of a long value.
     * @param radix
     *            the radix to use when parsing.
     * @return the primitive long value represented by {@code string} using
     *         {@code radix}.
     * @throws NumberFormatException
     *             if {@code string} cannot be parsed as a long value, or
     *             {@code radix < Character.MIN_RADIX ||
     *             radix > Character.MAX_RADIX}.
     */
    public static long parseLong(String string, int radix) throws NumberFormatException {
        if (radix < Character.MIN_RADIX || radix > Character.MAX_RADIX) {
            throw new NumberFormatException("Invalid radix: " + radix);
        }
        if (string == null) {
            throw invalidLong(string);
        }
        int length = string.length(), i = 0;
        if (length == 0) {
            throw invalidLong(string);
        }
        boolean negative = string.charAt(i) == '-';
        if (negative && ++i == length) {
            throw invalidLong(string);
        }

        return parse(string, i, radix, negative);
    }

    private static long parse(String string, int offset, int radix, boolean negative) {
        long max = Long.MIN_VALUE / radix;
        long result = 0, length = string.length();
        while (offset < length) {
            int digit = Character.digit(string.charAt(offset++), radix);
            if (digit == -1) {
                throw invalidLong(string);
            }
            if (max > result) {
                throw invalidLong(string);
            }
            long next = result * radix - digit;
            if (next > result) {
                throw invalidLong(string);
            }
            result = next;
        }
        if (!negative) {
            result = -result;
            if (result < 0) {
                throw invalidLong(string);
            }
        }
        return result;
    }

    /**
     * Returns the value of this Long as a short.
     *
     * @since	JDK1.1
     */
    public short shortValue() {
        return (short) value;
    }

    /**
     * Digit representations (ASCII) for number formatting. This saves calling
     * the Character method to get a digit representation for any radix.
     */
    static final char[] digitsArray =
                    "0123456789abcdefghijklmnopqrstuvwxyz".toCharArray();
    
    /**
     * This does the same as the toString method but for the bit-based
     * representations, where sign is ignored and each digit can be extracted
     * with shifting rather than division.
     */
    private static String toUnsignedString(long lval, int shift)
    {
        char[]	resultArray = new char[64];
        long mask = (1 << shift) - 1;
        int	resultArrayPos = 63;			// Array position

        do
        {
            resultArray[resultArrayPos--] = digitsArray[(int) (lval & mask)];
            lval >>>= shift;
        } while (lval != 0);

        return new String(resultArray, resultArrayPos+1, 63 - resultArrayPos);
    }
    
    /**
     * Converts the specified long value into its binary string representation.
     * The returned string is a concatenation of '0' and '1' characters.
     *
     * @param v
     *            the long value to convert.
     * @return the binary string representation of {@code v}.
     */
    public static String toBinaryString(long v) {
    	return toUnsignedString(v, 1);
    }

    /**
     * Converts the specified long value into its hexadecimal string
     * representation. The returned string is a concatenation of characters from
     * '0' to '9' and 'a' to 'f'.
     *
     * @param v
     *            the long value to convert.
     * @return the hexadecimal string representation of {@code l}.
     */
    public static String toHexString(long v) {
    	return toUnsignedString(v, 4);
    }

    /**
     * Converts the specified long value into its octal string representation.
     * The returned string is a concatenation of characters from '0' to '7'.
     *
     * @param v
     *            the long value to convert.
     * @return the octal string representation of {@code l}.
     */
    public static String toOctalString(long v) {
    	return toUnsignedString(v, 3);
    }

    @Override
    public String toString() {
        return Long.toString(value);
    }

    /**
     * Converts the specified long value into its decimal string representation.
     * The returned string is a concatenation of a minus sign if the number is
     * negative and characters from '0' to '9'.
     *
     * @param n
     *            the long to convert.
     * @return the decimal string representation of {@code l}.
     */
    public static String toString(long v) {
    	return toString(v, 10);
    }

    /**
     * Converts the specified signed long value into a string representation based on
     * the specified radix. The returned string is a concatenation of a minus
     * sign if the number is negative and characters from '0' to '9' and 'a' to
     * 'z', depending on the radix. If {@code radix} is not in the interval
     * defined by {@code Character.MIN_RADIX} and {@code Character.MAX_RADIX}
     * then 10 is used as the base for the conversion.
     *
     * <p>This method treats its argument as signed. If you want to convert an
     * unsigned value to one of the common non-decimal bases, you may find
     * {@link #toBinaryString}, {@code #toHexString}, or {@link #toOctalString}
     * more convenient.
     *
     * @param v
     *            the signed long to convert.
     * @param radix
     *            the base to use for the conversion.
     * @return the string representation of {@code v}.
     */
    public static String toString(long v, int radix) {
    	if (radix < Character.MIN_RADIX || radix > Character.MAX_RADIX)
    	    radix = 10;

    	char[] buf = new char[65];
    	boolean negative = (v < 0);
    	int charPos = 65;

    	if (!negative) {
    	    v = -v;
    	}

    	do {
    	    int x = '0' - (int)(v % radix);
    	    if (x > '9') {
    	        x += 'a' - '0' - 10;
    	    }
    	    buf[--charPos] = (char)x;
    	    v = v / radix;
    	} while (v != 0);

    	if (negative) {
    	    buf[--charPos] = '-';
    	}

    	return new String(buf, charPos, (65 - charPos));
    }

    /**
     * Parses the specified string as a signed decimal long value.
     *
     * @param string
     *            the string representation of a long value.
     * @return a {@code Long} instance containing the long value represented by
     *         {@code string}.
     * @throws NumberFormatException
     *             if {@code string} cannot be parsed as a long value.
     * @see #parseLong(String)
     */
    public static Long valueOf(String string) throws NumberFormatException {
        return valueOf(parseLong(string));
    }

    /**
     * Parses the specified string as a signed long value using the specified
     * radix.
     *
     * @param string
     *            the string representation of a long value.
     * @param radix
     *            the radix to use when parsing.
     * @return a {@code Long} instance containing the long value represented by
     *         {@code string} using {@code radix}.
     * @throws NumberFormatException
     *             if {@code string} cannot be parsed as a long value, or
     *             {@code radix < Character.MIN_RADIX ||
     *             radix > Character.MAX_RADIX}.
     * @see #parseLong(String, int)
     */
    public static Long valueOf(String string, int radix) throws NumberFormatException {
        return valueOf(parseLong(string, radix));
    }

    /**
     * Returns a {@code Long} instance for the specified long value.
     * <p>
     * If it is not necessary to get a new {@code Long} instance, it is
     * recommended to use this method instead of the constructor, since it
     * maintains a cache of instances which may result in better performance.
     *
     * @param v
     *            the long value to store in the instance.
     * @return a {@code Long} instance containing {@code v}.
     * @since 1.5
     */
    public static Long valueOf(long v) {
        return  v >= 128 || v < -128 ? new Long(v) : SMALL_VALUES[((int) v) + 128];
    }

    /**
     * A cache of instances used by {@link Long#valueOf(long)} and auto-boxing.
     */
    private static final Long[] SMALL_VALUES = new Long[256];

    static {
        for (int i = -128; i < 128; i++) {
            SMALL_VALUES[i + 128] = new Long(i);
        }
    }
}
