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
 * The wrapper for the primitive type {@code int}.
 * <p>
 * Implementation note: The "bit twiddling" methods in this class use techniques
 * described in <a href="http://www.hackersdelight.org/">Henry S. Warren,
 * Jr.'s Hacker's Delight, (Addison Wesley, 2002)</a> and <a href=
 * "http://graphics.stanford.edu/~seander/bithacks.html">Sean Anderson's
 * Bit Twiddling Hacks.</a>
 *
 * @see java.lang.Long
 * @since 1.0
 */
public final class Integer {

    private static final long serialVersionUID = 1360826667806852920L;

    /**
     * The int value represented by this Integer
     */
    private final int value;

    /**
     * Constant for the maximum {@code int} value, 2<sup>31</sup>-1.
     */
    public static final int MAX_VALUE = 0x7FFFFFFF;

    /**
     * Constant for the minimum {@code int} value, -2<sup>31</sup>.
     */
    public static final int MIN_VALUE = 0x80000000;

    /**
     * Constant for the number of bits needed to represent an {@code int} in
     * two's complement form.
     *
     * @since 1.5
     */
    public static final int SIZE = 32;

    /**
     * Constructs a new {@code Integer} with the specified primitive integer
     * value.
     *
     * @param value
     *            the primitive integer value to store in the new instance.
     */
    public Integer(int value) {
        this.value = value;
    }

    /**
     * Constructs a new {@code Integer} with the specified big endian bytes.
     *
     * @param bytes big endian bytes.
     */
    public Integer(byte[] bytes) {
        int v = 0;
        if (bytes != null && bytes.length > 0) {
            int max = bytes.length > 4 ? 4 : bytes.length;
            for (int i = 0; i < max; i++) {
                v ^= (bytes[i] & 0x0FF) << ((max - 1 - i) * 8);
            }
        }
        value = v;
    }

    /**
     * Returns the value of this Integer as a byte.
     *
     * @return the value of this Integer as a byte.
     *
     * @since   JDK1.1
     */
    public byte byteValue() {
        return (byte) value;
    }

    private static NumberFormatException invalidInt(String s) {
        throw new NumberFormatException("Invalid int: \"" + s + "\"");
    }

    /**
     * Parses the specified string and returns a {@code Integer} instance if the
     * string can be decoded into an integer value. The string may be an
     * optional minus sign "-" followed by a hexadecimal ("0x..." or "#..."),
     * octal ("0..."), or decimal ("...") representation of an integer.
     *
     * @param string
     *            a string representation of an integer value.
     * @return an {@code Integer} containing the value represented by
     *         {@code string}.
     * @throws NumberFormatException
     *             if {@code string} cannot be parsed as an integer value.
     */
    public static Integer decode(String string) throws NumberFormatException {
        int length = string.length(), i = 0;
        if (length == 0) {
            throw invalidInt(string);
        }
        char firstDigit = string.charAt(i);
        boolean negative = firstDigit == '-';
        if (negative) {
            if (length == 1) {
                throw invalidInt(string);
            }
            firstDigit = string.charAt(++i);
        }

        int base = 10;
        if (firstDigit == '0') {
            if (++i == length) {
                return valueOf(0);
            }
            if ((firstDigit = string.charAt(i)) == 'x' || firstDigit == 'X') {
                if (++i == length) {
                    throw invalidInt(string);
                }
                base = 16;
            } else {
                base = 8;
            }
        } else if (firstDigit == '#') {
            if (++i == length) {
                throw invalidInt(string);
            }
            base = 16;
        }

        int result = parse(string, i, base, negative);
        return valueOf(result);
    }

    /**
     * Returns the value of this Integer as a double.
     *
     * @return  the <code>double</code> value represented by this object.
     */
    public double doubleValue() {
        return value;
    }

    /**
     * Compares this instance with the specified object and indicates if they
     * are equal. In order to be equal, {@code o} must be an instance of
     * {@code Integer} and have the same integer value as this object.
     *
     * @param o
     *            the object to compare this integer with.
     * @return {@code true} if the specified object is equal to this
     *         {@code Integer}; {@code false} otherwise.
     */
    //@Override
    public boolean equals(Object o) {
        return (o instanceof Integer) && (((Integer) o).value == value);
    }

    /**
     * Returns the value of this Integer as a float.
     *
     * @return  the <code>float</code> value represented by this object.
     */
    public float floatValue() {
        return value;
    }

    /**
     * Returns a hashcode for this Integer.
     *
     * @return  a hash code value for this object, equal to the
     *          primitive <tt>int</tt> value represented by this
     *          <tt>Integer</tt> object.
     */
    //@Override
    public int hashCode() {
        return value;
    }

    /**
     * Gets the primitive value of this int.
     *
     * @return this object's primitive value.
     */
    public int intValue() {
        return value;
    }

    /**
     * Returns the value of this Integer as a <tt>long</tt>.
     *
     * @return  the <code>int</code> value represented by this object that is
     *          converted to type <code>long</code> and the result of the
     *          conversion is returned.
     */
    public long longValue() {
        return value;
    }

    /**
     * Parses the specified string as a signed decimal integer value. The ASCII
     * character \u002d ('-') is recognized as the minus sign.
     *
     * @param string
     *            the string representation of an integer value.
     * @return the primitive integer value represented by {@code string}.
     * @throws NumberFormatException
     *             if {@code string} cannot be parsed as an integer value.
     */
    public static int parseInt(String string) throws NumberFormatException {
        return parseInt(string, 10);
    }

    /**
     * Parses the specified string as a signed integer value using the specified
     * radix. The ASCII character \u002d ('-') is recognized as the minus sign.
     *
     * @param string
     *            the string representation of an integer value.
     * @param radix
     *            the radix to use when parsing.
     * @return the primitive integer value represented by {@code string} using
     *         {@code radix}.
     * @throws NumberFormatException
     *             if {@code string} cannot be parsed as an integer value,
     *             or {@code radix < Character.MIN_RADIX ||
     *             radix > Character.MAX_RADIX}.
     */
    public static int parseInt(String string, int radix) throws NumberFormatException {
        if (radix < Character.MIN_RADIX || radix > Character.MAX_RADIX) {
            throw new NumberFormatException("Invalid radix: " + radix);
        }
        if (string == null) {
            throw invalidInt(string);
        }
        int length = string.length(), i = 0;
        if (length == 0) {
            throw invalidInt(string);
        }
        boolean negative = string.charAt(i) == '-';
        if (negative && ++i == length) {
            throw invalidInt(string);
        }

        return parse(string, i, radix, negative);
    }

    private static int parse(String string, int offset, int radix, boolean negative) throws NumberFormatException {
        int max = Integer.MIN_VALUE / radix;
        int result = 0, length = string.length();
        while (offset < length) {
            int digit = Character.digit(string.charAt(offset++), radix);
            if (digit == -1) {
                throw invalidInt(string);
            }
            if (max > result) {
                throw invalidInt(string);
            }
            int next = result * radix - digit;
            if (next > result) {
                throw invalidInt(string);
            }
            result = next;
        }
        if (!negative) {
            result = -result;
            if (result < 0) {
                throw invalidInt(string);
            }
        }
        return result;
    }

    /**
     * Returns the value of this Integer as a short.
     *
     * @return the value of this Integer as a short.
     *
     * @since   JDK1.1
     */
    public short shortValue() {
        return (short) value;
    }

    /**
     * Convert the integer to an unsigned number.
     */
    private static String toUnsignedString(int i, int shift) {

        char[] buf = new char[32];
        int charPos = 32;
        int radix = 1 << shift;
        int mask = radix - 1;
        do {
            int x = '0' + (i & mask);
            if (x > '9') {
                x += 'a' - '0' - 10;
            }
            buf[--charPos] = (char)x;
            i >>>= shift;
        } while (i != 0);

        return new String(buf, charPos, (32 - charPos));
    }

    /**
     * Converts the specified integer into its binary string representation. The
     * returned string is a concatenation of '0' and '1' characters.
     *
     * @param i
     *            the integer to convert.
     * @return the binary string representation of {@code i}.
     */
    public static String toBinaryString(int i) {
        return toUnsignedString(i, 1);
    }

    /**
     * Converts the specified integer into its hexadecimal string
     * representation. The returned string is a concatenation of characters from
     * '0' to '9' and 'a' to 'f'.
     *
     * @param i
     *            the integer to convert.
     * @return the hexadecimal string representation of {@code i}.
     */
    public static String toHexString(int i) {
        return toUnsignedString(i, 4);
    }

    /**
     * Converts the specified integer into its octal string representation. The
     * returned string is a concatenation of characters from '0' to '7'.
     *
     * @param i
     *            the integer to convert.
     * @return the octal string representation of {@code i}.
     */
    public static String toOctalString(int i) {
        return toUnsignedString(i, 3);
    }

    /**
     * Returns a String object representing this Integer's value. The
     * value is converted to signed decimal representation and returned
     * as a string, exactly as if the integer value were given as an
     * argument to the {@link java.lang.Integer#toString(int)} method.
     *
     * @return  a string representation of the value of this object in
     *          base&nbsp;10.
     */
    //@Override
    public String toString() {
        return Integer.toString(value);
    }

    /**
     * Converts the specified integer into its decimal string representation.
     * The returned string is a concatenation of a minus sign if the number is
     * negative and characters from '0' to '9'.
     *
     * @param i
     *            the integer to convert.
     * @return the decimal string representation of {@code i}.
     */
    public static String toString(int i) {
    	return toString(i, 10);
    }

    /**
     * Converts the specified signed integer into a string representation based on the
     * specified radix. The returned string is a concatenation of a minus sign
     * if the number is negative and characters from '0' to '9' and 'a' to 'z',
     * depending on the radix. If {@code radix} is not in the interval defined
     * by {@code Character.MIN_RADIX} and {@code Character.MAX_RADIX} then 10 is
     * used as the base for the conversion.
     *
     * <p>This method treats its argument as signed. If you want to convert an
     * unsigned value to one of the common non-decimal bases, you may find
     * {@link #toBinaryString}, {@code #toHexString}, or {@link #toOctalString}
     * more convenient.
     *
     * @param i
     *            the signed integer to convert.
     * @param radix
     *            the base to use for the conversion.
     * @return the string representation of {@code i}.
     */
    public static String toString(int i, int radix) {

    	if (radix < Character.MIN_RADIX || radix > Character.MAX_RADIX)
    	    radix = 10;

    	char buf[] = new char[33];
    	boolean negative = (i < 0);
    	int charPos = 33;

    	if (!negative) {
    	    i = -i;
    	}

    	do {
    	    int x = '0' - (i % radix);
    	    if (x > '9') {
    	        x += 'a' - '0' - 10;
    	    }
    	    buf[--charPos] = (char)x;
    	    i = i / radix;
    	} while (i != 0);

    	if (negative) {
    	    buf[--charPos] = '-';
    	}

    	return new String(buf, charPos, (33 - charPos));
    }

    /**
     * Parses the specified string as a signed decimal integer value.
     *
     * @param string
     *            the string representation of an integer value.
     * @return an {@code Integer} instance containing the integer value
     *         represented by {@code string}.
     * @throws NumberFormatException
     *             if {@code string} cannot be parsed as an integer value.
     * @see #parseInt(String)
     */
    public static Integer valueOf(String string) throws NumberFormatException {
        return valueOf(parseInt(string));
    }

    /**
     * Parses the specified string as a signed integer value using the specified
     * radix.
     *
     * @param string
     *            the string representation of an integer value.
     * @param radix
     *            the radix to use when parsing.
     * @return an {@code Integer} instance containing the integer value
     *         represented by {@code string} using {@code radix}.
     * @throws NumberFormatException
     *             if {@code string} cannot be parsed as an integer value, or
     *             {@code radix < Character.MIN_RADIX ||
     *             radix > Character.MAX_RADIX}.
     * @see #parseInt(String, int)
     */
    public static Integer valueOf(String string, int radix) throws NumberFormatException {
        return valueOf(parseInt(string, radix));
    }

    /**
     * Returns a {@code Integer} instance for the specified integer value.
     * <p>
     * If it is not necessary to get a new {@code Integer} instance, it is
     * recommended to use this method instead of the constructor, since it
     * maintains a cache of instances which may result in better performance.
     *
     * @param i
     *            the integer value to store in the instance.
     * @return a {@code Integer} instance containing {@code i}.
     * @since 1.5
     */
    private static Integer valueOf(int i) {
        return  i >= 128 || i < -128 ? new Integer(i) : SMALL_VALUES[i + 128];
    }

    /**
     * A cache of instances used by {@link Integer#valueOf(int)} and auto-boxing
     */
    private static final Integer[] SMALL_VALUES = new Integer[256];

    static {
        for (int i = -128; i < 128; i++) {
            SMALL_VALUES[i + 128] = new Integer(i);
        }
    }
}
