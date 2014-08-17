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
 * The wrapper for the primitive type {@code byte}.
 *
 * @since 1.1
 */
public final class Byte {

    private static final long serialVersionUID = -7183698231559129828L;

    /**
     * The value which the receiver represents.
     */
    private final byte value;

    /**
     * The maximum {@code Byte} value, 2<sup>7</sup>-1.
     */
    public static final byte MAX_VALUE = (byte) 0x7F;

    /**
     * The minimum {@code Byte} value, -2<sup>7</sup>.
     */
    public static final byte MIN_VALUE = (byte) 0x80;

    /**
     * The number of bits needed to represent a {@code Byte} value in two's
     * complement form.
     *
     * @since 1.5
     */
    public static final int SIZE = 8;

    /**
     * Constructs a new {@code Byte} with the specified primitive byte value.
     *
     * @param value
     *            the primitive byte value to store in the new instance.
     */
    public Byte(byte value) {
        this.value = value;
    }

    /**
     * Gets the primitive value of this byte.
     *
     * @return this object's primitive value.
     */
    public byte byteValue() {
        return value;
    }

    /**
     * Compares this object with the specified object and indicates if they are
     * equal. In order to be equal, {@code object} must be an instance of
     * {@code Byte} and have the same byte value as this object.
     *
     * @param object
     *            the object to compare this byte with.
     * @return {@code true} if the specified object is equal to this
     *         {@code Byte}; {@code false} otherwise.
     */
    //@Override
    public boolean equals(Object object) {
        return (object == this) || ((object instanceof Byte) && (((Byte) object).value == value));
    }

    //@Override
    public int hashCode() {
        return value;
    }

    /**
     * Parses the specified string as a signed decimal byte value. The ASCII
     * character \u002d ('-') is recognized as the minus sign.
     *
     * @param string
     *            the string representation of a single byte value.
     * @return the primitive byte value represented by {@code string}.
     * @throws NumberFormatException
     *             if {@code string} can not be parsed as a byte value.
     */
    public static byte parseByte(String string) throws NumberFormatException {
        return parseByte(string, 10);
    }

    /**
     * Parses the specified string as a signed byte value using the specified
     * radix. The ASCII character \u002d ('-') is recognized as the minus sign.
     *
     * @param string
     *            the string representation of a single byte value.
     * @param radix
     *            the radix to use when parsing.
     * @return the primitive byte value represented by {@code string} using
     *         {@code radix}.
     * @throws NumberFormatException
     *             if {@code string} can not be parsed as a byte value, or
     *             {@code radix < Character.MIN_RADIX ||
     *             radix > Character.MAX_RADIX}.
     */
    public static byte parseByte(String string, int radix) throws NumberFormatException {
        int intValue = Integer.parseInt(string, radix);
        byte result = (byte) intValue;
        if (result == intValue) {
            return result;
        }
        throw new NumberFormatException("Value out of range for byte: \"" + string + "\"");
    }

    //@Override
    public String toString() {
        return Integer.toString(value);
    }

    /**
     * Returns a string containing a concise, human-readable description of the
     * specified byte value.
     *
     * @param value
     *            the byte to convert to a string.
     * @return a printable representation of {@code value}.
     */
    public static String toString(byte value) {
        return Integer.toString(value);
    }
}
