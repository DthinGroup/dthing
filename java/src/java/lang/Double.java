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
 * The wrapper for the primitive type {@code double}.
 *
 * @see java.lang.Number
 * @since 1.0
 */
public final class Double {

    private static final long serialVersionUID = -9172774392245257468L;

    /**
     * The value which the receiver represents.
     */
    private final double value;

    /**
     * Constant for the maximum {@code double} value, (2 - 2<sup>-52</sup>) *
     * 2<sup>1023</sup>.
     */
    public static final double MAX_VALUE = 1.79769313486231570e+308;

    /**
     * Constant for the minimum {@code double} value, 2<sup>-1074</sup>.
     */
    public static final double MIN_VALUE = 5e-324;

    /* 4.94065645841246544e-324 gets rounded to 9.88131e-324 */

    /**
     * Constant for the Not-a-Number (NaN) value of the {@code double} type.
     */
    public static final double NaN = 0.0D / 0.0D;

    /**
     * Constant for the positive infinity value of the {@code double} type.
     */
    public static final double POSITIVE_INFINITY = 1.0D / 0.0D;

    /**
     * Constant for the negative infinity value of the {@code double} type.
     */
    public static final double NEGATIVE_INFINITY = -1.0D / 0.0D;


    /**
     * Constant for the number of bits needed to represent a {@code double} in
     * two's complement form.
     *
     * @since 1.5
     */
    public static final int SIZE = 64;

    /**
     * Constructs a new {@code Double} with the specified primitive double
     * value.
     *
     * @param value
     *            the primitive double value to store in the new instance.
     */
    public Double(double value) {
        this.value = value;
    }

	/**
	 * Returns the value of this Double as a byte (by casting to a byte).
	 *
	 * @since	JDK1.1
	 */
    public byte byteValue() {
        return (byte) value;
    }

    /**
     * Returns an integer corresponding to the bits of the given
     * <a href="http://en.wikipedia.org/wiki/IEEE_754-1985">IEEE 754</a> double precision
     * {@code value}. All <em>Not-a-Number (NaN)</em> values are converted to a single NaN
     * representation ({@code 0x7ff8000000000000L}) (compare to {@link #doubleToRawLongBits}).
     */
    public static native long doubleToLongBits(double value);

    /**
     * Returns an integer corresponding to the bits of the given
     * <a href="http://en.wikipedia.org/wiki/IEEE_754-1985">IEEE 754</a> double precision
     * {@code value}. <em>Not-a-Number (NaN)</em> values are preserved (compare
     * to {@link #doubleToLongBits}).
     */
    public static native long doubleToRawLongBits(double value);

    /**
     * Gets the primitive value of this double.
     *
     * @return this object's primitive value.
     */
    public double doubleValue() {
        return value;
    }

    /**
     * Tests this double for equality with {@code object}.
     * To be equal, {@code object} must be an instance of {@code Double} and
     * {@code doubleToLongBits} must give the same value for both objects.
     *
     * <p>Note that, unlike {@code ==}, {@code -0.0} and {@code +0.0} compare
     * unequal, and {@code NaN}s compare equal by this method.
     *
     * @param object
     *            the object to compare this double with.
     * @return {@code true} if the specified object is equal to this
     *         {@code Double}; {@code false} otherwise.
     */
    @Override
    public boolean equals(Object object) {
        return (object instanceof Double) &&
                (doubleToLongBits(this.value) == doubleToLongBits(((Double) object).value));
    }

	/**
	 * Returns the float value of this Double.
	 *
	 * @return	the <code>double</code> value represented by this object is
	 *			converted to type <code>float</code> and the result of the
	 *			conversion is returned.
	 * @since	JDK1.0
	 */
    public float floatValue() {
        return (float) value;
    }

	/**
	 * Returns a hashcode for this <code>Double</code> object. The result
	 * is the exclusive OR of the two halves of the long integer bit
	 * representation, exactly as produced by the method
	 * {@link #doubleToLongBits(double)}, of the primitive
	 * <code>double</code> value represented by this <code>Double</code>
	 * object. That is, the hashcode is the value of the expression:
	 * <blockquote><pre>
	 * (int)(v^(v>>>32))
	 * </pre></blockquote>
	 * where <code>v</code> is defined by:
	 * <blockquote><pre>
	 * long v = Double.doubleToLongBits(this.doubleValue());
	 * </pre></blockquote>
	 *
	 * @return	a <code>hash code</code> value for this object.
	 */
    public int hashCode() {
        long v = doubleToLongBits(value);
        return (int) (v ^ (v >>> 32));
    }

	/**
	 * Returns the integer value of this Double (by casting to an int).
	 *
	 * @return	the <code>double</code> value represented by this object is
	 *			converted to type <code>int</code> and the result of the
	 *			conversion is returned.
	 */
    public int intValue() {
        return (int) value;
    }

    /**
     * Indicates whether this object represents an infinite value.
     *
     * @return {@code true} if the value of this double is positive or negative
     *         infinity; {@code false} otherwise.
     */
    public boolean isInfinite() {
        return isInfinite(value);
    }

    /**
     * Indicates whether the specified double represents an infinite value.
     *
     * @param d
     *            the double to check.
     * @return {@code true} if the value of {@code d} is positive or negative
     *         infinity; {@code false} otherwise.
     */
    public static boolean isInfinite(double d) {
        return (d == POSITIVE_INFINITY) || (d == NEGATIVE_INFINITY);
    }

    /**
     * Indicates whether this object is a <em>Not-a-Number (NaN)</em> value.
     *
     * @return {@code true} if this double is <em>Not-a-Number</em>;
     *         {@code false} if it is a (potentially infinite) double number.
     */
    public boolean isNaN() {
        return isNaN(value);
    }

    /**
     * Indicates whether the specified double is a <em>Not-a-Number (NaN)</em>
     * value.
     *
     * @param d
     *            the double value to check.
     * @return {@code true} if {@code d} is <em>Not-a-Number</em>;
     *         {@code false} if it is a (potentially infinite) double number.
     */
    public static boolean isNaN(double d) {
        return d != d;
    }

    /**
     * Returns the <a href="http://en.wikipedia.org/wiki/IEEE_754-1985">IEEE 754</a>
     * double precision float corresponding to the given {@code bits}.
     */
    public static native double longBitsToDouble(long bits);

	/**
	 * Returns the integer value of this Double (by casting to an int).
	 *
	 * @return	the <code>double</code> value represented by this object is
	 *			converted to type <code>int</code> and the result of the
	 *			conversion is returned.
	 */
    public long longValue() {
        return (long) value;
    }

    /**
     * Parses the specified string as a double value.
     *
     * @param string
     *            the string representation of a double value.
     * @return the primitive double value represented by {@code string}.
     * @throws NumberFormatException
     *             if {@code string} cannot be parsed as a double value.
     */
    public static double parseDouble(String string) throws NumberFormatException {
    	return FloatingDecimal.readJavaFormatString(string).doubleValue();
    }    

	/**
	 * Returns the value of this Double as a short (by casting to a short).
	 *
	 * @since	JDK1.1
	 */
    public short shortValue() {
        return (short) value;
    }

	/**
	 * Returns a String representation of this Double object.
	 * The primitive <code>double</code> value represented by this
	 * object is converted to a string exactly as if by the method
	 * <code>toString</code> of one argument.
	 *
	 * @return	a <code>String</code> representation of this object.
	 * @see		java.lang.Double#toString(double)
	 */    
    @Override
    public String toString() {
        return Double.toString(value);
    }

    /**
     * Returns a string containing a concise, human-readable description of the
     * specified double value.
     *
     * @param d
     *             the double to convert to a string.
     * @return a printable representation of {@code d}.
     */
    public static String toString(double d) {
    	return new FloatingDecimal(d).toJavaFormatString();
    }


    /**
     * Parses the specified string as a double value.
     *
     * @param string
     *            the string representation of a double value.
     * @return a {@code Double} instance containing the double value represented
     *         by {@code string}.
     * @throws NumberFormatException
     *             if {@code string} cannot be parsed as a double value.
     * @see #parseDouble(String)
     */
    public static Double valueOf(String string) throws NumberFormatException {
    	return parseDouble(string);
    }

}
