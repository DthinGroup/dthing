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

import com.yarlungsoft.util.FloatingDecimal;

/**
 * The wrapper for the primitive type {@code float}.
 *
 * @see java.lang.Number
 * @since 1.0
 */
public final class Float  {

    private static final long serialVersionUID = -2671257302660747028L;

    /**
     * The value which the receiver represents.
     */
    private final float value;

    /**
     * Constant for the maximum {@code float} value, (2 - 2<sup>-23</sup>) * 2<sup>127</sup>.
     */
    public static final float MAX_VALUE = 3.40282346638528860e+38f;

    /**
     * Constant for the minimum {@code float} value, 2<sup>-149</sup>.
     */
    public static final float MIN_VALUE = 1.40129846432481707e-45f;

    /**
     * Constant for the Not-a-Number (NaN) value of the {@code float} type.
     */
    public static final float NaN = 0.0f / 0.0f;

    /**
     * Constant for the positive infinity value of the {@code float} type.
     */
    public static final float POSITIVE_INFINITY = 1.0f / 0.0f;

    /**
     * Constant for the negative infinity value of the {@code float} type.
     */
    public static final float NEGATIVE_INFINITY = -1.0f / 0.0f;

    /**
     * Constant for the number of bits needed to represent a {@code float} in
     * two's complement form.
     *
     * @since 1.5
     */
    public static final int SIZE = 32;

    /**
     * Constructs a new {@code Float} with the specified primitive float value.
     *
     * @param value
     *            the primitive float value to store in the new instance.
     */
    public Float(float value) {
        this.value = value;
    }

    /**
     * Constructs a new {@code Float} with the specified primitive double value.
     *
     * @param value
     *            the primitive double value to store in the new instance.
     */
    public Float(double value) {
        this.value = (float) value;
    }

	/**
	 * Returns the value of this Float as a byte (by casting to a byte).
	 *
	 * @since	JDK1.1
	 */
    public byte byteValue() {
        return (byte) value;
    }

	/**
	 * Returns the double value of this <tt>Float</tt> object.
	 *
	 * @return the <code>float</code> value represented by this
	 *		   object is converted to type <code>double</code> and the
	 *		   result of the conversion is returned.
	 */
    public double doubleValue() {
        return (double)value;
    }

    /**
     * Tests this double for equality with {@code object}.
     * To be equal, {@code object} must be an instance of {@code Float} and
     * {@code floatToIntBits} must give the same value for both objects.
     *
     * <p>Note that, unlike {@code ==}, {@code -0.0} and {@code +0.0} compare
     * unequal, and {@code NaN}s compare equal by this method.
     *
     * @param object
     *            the object to compare this float with.
     * @return {@code true} if the specified object is equal to this
     *         {@code Float}; {@code false} otherwise.
     */
    //@Override
    public boolean equals(Object object) {
        return (object instanceof Float) &&
                (floatToIntBits(this.value) == floatToIntBits(((Float) object).value));
    }

    /**
     * Returns an integer corresponding to the bits of the given
     * <a href="http://en.wikipedia.org/wiki/IEEE_754-1985">IEEE 754</a> single precision
     * float {@code value}. All <em>Not-a-Number (NaN)</em> values are converted to a single NaN
     * representation ({@code 0x7fc00000}) (compare to {@link #floatToRawIntBits}).
     */
    public static native int floatToIntBits(float value);

    /**
     * Returns an integer corresponding to the bits of the given
     * <a href="http://en.wikipedia.org/wiki/IEEE_754-1985">IEEE 754</a> single precision
     * float {@code value}. <em>Not-a-Number (NaN)</em> values are preserved (compare
     * to {@link #floatToIntBits}).
     */
    public static native int floatToRawIntBits(float value);

    /**
     * Gets the primitive value of this float.
     *
     * @return this object's primitive value.
     */
    public float floatValue() {
        return value;
    }

	/**
	 * Returns a hashcode for this <tt>Float</tt> object. The result
	 * is the integer bit representation, exactly as produced
	 * by the method {@link #floatToIntBits(float)}, of the primitive float
	 * value represented by this <tt>Float</tt> object.
	 *
	 * @return	a hash code value for this object.
	 */
    //@Override
    public int hashCode() {
        return floatToIntBits(value);
    }

    /**
     * Returns the <a href="http://en.wikipedia.org/wiki/IEEE_754-1985">IEEE 754</a>
     * single precision float corresponding to the given {@code bits}.
     */
    public static native float intBitsToFloat(int bits);

	/**
	 * Returns the integer value of this Float (by casting to an int).
	 *
	 * @return	the <code>float</code> value represented by this object
	 *			converted to type <code>int</code> and the result of the
	 *			conversion is returned.
	 */
    public int intValue() {
        return (int) value;
    }

    /**
     * Indicates whether this object represents an infinite value.
     *
     * @return {@code true} if the value of this float is positive or negative
     *         infinity; {@code false} otherwise.
     */
    public boolean isInfinite() {
        return isInfinite(value);
    }

    /**
     * Indicates whether the specified float represents an infinite value.
     *
     * @param f
     *            the float to check.
     * @return {@code true} if the value of {@code f} is positive or negative
     *         infinity; {@code false} otherwise.
     */
    public static boolean isInfinite(float f) {
        return (f == POSITIVE_INFINITY) || (f == NEGATIVE_INFINITY);
    }

    /**
     * Indicates whether this object is a <em>Not-a-Number (NaN)</em> value.
     *
     * @return {@code true} if this float is <em>Not-a-Number</em>;
     *         {@code false} if it is a (potentially infinite) float number.
     */
    public boolean isNaN() {
        return isNaN(value);
    }

    /**
     * Indicates whether the specified float is a <em>Not-a-Number (NaN)</em>
     * value.
     *
     * @param f
     *            the float value to check.
     * @return {@code true} if {@code f} is <em>Not-a-Number</em>;
     *         {@code false} if it is a (potentially infinite) float number.
     */
    public static boolean isNaN(float f) {
        return f != f;
    }

	/**
	 * Returns the long value of this Float (by casting to a long).
	 *
	 * @return	the <code>float</code> value represented by this object is
	 *			converted to type <code>long</code> and the result of the
	 *			conversion is returned.
	 */
    public long longValue() {
        return (long) value;
    }

    /**
     * Parses the specified string as a float value.
     *
     * @param string
     *            the string representation of a float value.
     * @return the primitive float value represented by {@code string}.
     * @throws NumberFormatException
     *             if {@code string} can not be parsed as a float value.
     * @see #valueOf(String)
     * @since 1.2
     */
    public static float parseFloat(String string) throws NumberFormatException {
    	return FloatingDecimal.readJavaFormatString(string).floatValue();
    }

	/**
	 * Returns the long value of this Float (by casting to a long).
	 *
	 * @return	the <code>float</code> value represented by this object is
	 *			converted to type <code>long</code> and the result of the
	 *			conversion is returned.
	 */
    public short shortValue() {
        return (short) value;
    }

	/**
	 * Returns a String representation of this Float object.
	 * The primitive <code>float</code> value represented by this object
	 * is converted to a <code>String</code> exactly as if by the method
	 * <code>toString</code> of one argument.
	 *
	 * @return	a <code>String</code> representation of this object.
	 * @see		java.lang.Float#toString(float)
	 */
    //@Override
    public String toString() {
        return Float.toString(value);
    }

    /**
     * Returns a string containing a concise, human-readable description of the
     * specified float value.
     *
     * @param f
     *             the float to convert to a string.
     * @return a printable representation of {@code f}.
     */
    public static String toString(float f) {
        return new FloatingDecimal(f).toJavaFormatString();
    }

    /**
     * Parses the specified string as a float value.
     *
     * @param string
     *            the string representation of a float value.
     * @return a {@code Float} instance containing the float value represented
     *         by {@code string}.
     * @throws NumberFormatException
     *             if {@code string} can not be parsed as a float value.
     * @see #parseFloat(String)
     */
    public static Float valueOf(String string) throws NumberFormatException {
        return parseFloat(string);
    }
}
