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
 * A modifiable {@link CharSequence sequence of characters} for use in creating
 * strings. This class is intended as a direct replacement of
 * {@link StringBuffer} for non-concurrent use; unlike {@code StringBuffer} this
 * class is not synchronized.
 *
 * <p>For particularly complex string-building needs, consider {@link java.util.Formatter}.
 *
 * <p>The majority of the modification methods on this class return {@code
 * this} so that method calls can be chained together. For example:
 * {@code new StringBuilder("a").append("b").append("c").toString()}.
 *
 * @see CharSequence
 * @see Appendable
 * @see StringBuffer
 * @see String
 * @see String#format
 * @since 1.5
 */
public final class StringBuilder {

    private static final long serialVersionUID = 4383685877147921099L;

    private StringBuffer sb;
    /**
     * Constructs an instance with an initial capacity of {@code 16}.
     *
     * @see #capacity()
     */
    public StringBuilder() {
    	sb = new StringBuffer();
    }

    /**
     * Constructs an instance with the specified capacity.
     *
     * @param capacity
     *            the initial capacity to use.
     * @throws NegativeArraySizeException
     *             if the specified {@code capacity} is negative.
     * @see #capacity()
     */
    public StringBuilder(int capacity) {
    	sb = new StringBuffer(capacity);
    }

    /**
     * Constructs an instance that's initialized with the contents of the
     * specified {@code String}. The capacity of the new builder will be the
     * length of the {@code String} plus 16.
     *
     * @param str
     *            the {@code String} to copy into the builder.
     * @throws NullPointerException
     *            if {@code str} is {@code null}.
     */
    public StringBuilder(String str) {
    	sb = new StringBuffer(str);
    }

    /**
     * Appends the string representation of the specified {@code boolean} value.
     * The {@code boolean} value is converted to a String according to the rule
     * defined by {@link String#valueOf(boolean)}.
     *
     * @param b
     *            the {@code boolean} value to append.
     * @return this builder.
     * @see String#valueOf(boolean)
     */
    public StringBuilder append(boolean b) {
    	sb.append(b);
        return this;
    }

    /**
     * Appends the string representation of the specified {@code char} value.
     * The {@code char} value is converted to a string according to the rule
     * defined by {@link String#valueOf(char)}.
     *
     * @param c
     *            the {@code char} value to append.
     * @return this builder.
     * @see String#valueOf(char)
     */
    public StringBuilder append(char c) {
    	sb.append(c);
        return this;
    }

    /**
     * Appends the string representation of the specified {@code int} value. The
     * {@code int} value is converted to a string according to the rule defined
     * by {@link String#valueOf(int)}.
     *
     * @param i
     *            the {@code int} value to append.
     * @return this builder.
     * @see String#valueOf(int)
     */
    public StringBuilder append(int i) {
    	sb.append(i);
        return this;
    }

    /**
     * Appends the string representation of the specified {@code long} value.
     * The {@code long} value is converted to a string according to the rule
     * defined by {@link String#valueOf(long)}.
     *
     * @param l
     *            the {@code long} value.
     * @return this builder.
     * @see String#valueOf(long)
     */
    public StringBuilder append(long l) {
    	sb.append(l);
        return this;
    }

    /**
     * Appends the string representation of the specified {@code float} value.
     * The {@code float} value is converted to a string according to the rule
     * defined by {@link String#valueOf(float)}.
     *
     * @param f
     *            the {@code float} value to append.
     * @return this builder.
     * @see String#valueOf(float)
     */
    public StringBuilder append(float f) {
    	sb.append(f);
        return this;
    }

    /**
     * Appends the string representation of the specified {@code double} value.
     * The {@code double} value is converted to a string according to the rule
     * defined by {@link String#valueOf(double)}.
     *
     * @param d
     *            the {@code double} value to append.
     * @return this builder.
     * @see String#valueOf(double)
     */
    public StringBuilder append(double d) {
    	sb.append(d);
        return this;
    }

    /**
     * Appends the string representation of the specified {@code Object}.
     * The {@code Object} value is converted to a string according to the rule
     * defined by {@link String#valueOf(Object)}.
     *
     * @param obj
     *            the {@code Object} to append.
     * @return this builder.
     * @see String#valueOf(Object)
     */
    public StringBuilder append(Object obj) {
    	sb.append(obj);
        return this;
    }

    /**
     * Appends the contents of the specified string. If the string is {@code
     * null}, then the string {@code "null"} is appended.
     *
     * @param str
     *            the string to append.
     * @return this builder.
     */
    public StringBuilder append(String str) {
    	sb.append(str);
        return this;
    }

    /**
     * Appends the contents of the specified {@code StringBuffer}. If the
     * StringBuffer is {@code null}, then the string {@code "null"} is
     * appended.
     *
     * @param sb
     *            the {@code StringBuffer} to append.
     * @return this builder.
     */
    public StringBuilder append(StringBuffer sb) {
    	this.sb.append(sb);
        return this;
    }

    /**
     * Appends the string representation of the specified {@code char[]}.
     * The {@code char[]} is converted to a string according to the rule
     * defined by {@link String#valueOf(char[])}.
     *
     * @param chars
     *            the {@code char[]} to append..
     * @return this builder.
     * @see String#valueOf(char[])
     */
    public StringBuilder append(char[] chars) {
    	sb.append(chars);
        return this;
    }

    /**
     * Appends the string representation of the specified subset of the {@code
     * char[]}. The {@code char[]} value is converted to a String according to
     * the rule defined by {@link String#valueOf(char[],int,int)}.
     *
     * @param str
     *            the {@code char[]} to append.
     * @param offset
     *            the inclusive offset index.
     * @param len
     *            the number of characters.
     * @return this builder.
     * @throws ArrayIndexOutOfBoundsException
     *             if {@code offset} and {@code len} do not specify a valid
     *             subsequence.
     * @see String#valueOf(char[],int,int)
     */
    public StringBuilder append(char[] str, int offset, int len) {
        sb.append(str, offset, len);
        return this;
    }

    /**
     * Deletes a sequence of characters specified by {@code start} and {@code
     * end}. Shifts any remaining characters to the left.
     *
     * @param start
     *            the inclusive start index.
     * @param end
     *            the exclusive end index.
     * @return this builder.
     * @throws StringIndexOutOfBoundsException
     *             if {@code start} is less than zero, greater than the current
     *             length or greater than {@code end}.
     */
    public StringBuilder delete(int start, int end) {
    	sb.delete(start, end);
        return this;
    }

    /**
     * Deletes the character at the specified index. shifts any remaining
     * characters to the left.
     *
     * @param index
     *            the index of the character to delete.
     * @return this builder.
     * @throws StringIndexOutOfBoundsException
     *             if {@code index} is less than zero or is greater than or
     *             equal to the current length.
     */
    public StringBuilder deleteCharAt(int index) {
    	sb.deleteCharAt(index);
        return this;
    }

    /**
     * Inserts the string representation of the specified {@code boolean} value
     * at the specified {@code offset}. The {@code boolean} value is converted
     * to a string according to the rule defined by
     * {@link String#valueOf(boolean)}.
     *
     * @param offset
     *            the index to insert at.
     * @param b
     *            the {@code boolean} value to insert.
     * @return this builder.
     * @throws StringIndexOutOfBoundsException
     *             if {@code offset} is negative or greater than the current
     *             {@code length}.
     * @see String#valueOf(boolean)
     */
    public StringBuilder insert(int offset, boolean b) {
    	sb.insert(offset, b);
        return this;
    }

    /**
     * Inserts the string representation of the specified {@code char} value at
     * the specified {@code offset}. The {@code char} value is converted to a
     * string according to the rule defined by {@link String#valueOf(char)}.
     *
     * @param offset
     *            the index to insert at.
     * @param c
     *            the {@code char} value to insert.
     * @return this builder.
     * @throws IndexOutOfBoundsException
     *             if {@code offset} is negative or greater than the current
     *             {@code length()}.
     * @see String#valueOf(char)
     */
    public StringBuilder insert(int offset, char c) {
    	sb.insert(offset, c);
        return this;
    }

    /**
     * Inserts the string representation of the specified {@code int} value at
     * the specified {@code offset}. The {@code int} value is converted to a
     * String according to the rule defined by {@link String#valueOf(int)}.
     *
     * @param offset
     *            the index to insert at.
     * @param i
     *            the {@code int} value to insert.
     * @return this builder.
     * @throws StringIndexOutOfBoundsException
     *             if {@code offset} is negative or greater than the current
     *             {@code length()}.
     * @see String#valueOf(int)
     */
    public StringBuilder insert(int offset, int i) {
    	sb.insert(offset, i);
        return this;
    }

    /**
     * Inserts the string representation of the specified {@code long} value at
     * the specified {@code offset}. The {@code long} value is converted to a
     * String according to the rule defined by {@link String#valueOf(long)}.
     *
     * @param offset
     *            the index to insert at.
     * @param l
     *            the {@code long} value to insert.
     * @return this builder.
     * @throws StringIndexOutOfBoundsException
     *             if {@code offset} is negative or greater than the current
     *             {code length()}.
     * @see String#valueOf(long)
     */
    public StringBuilder insert(int offset, long l) {
    	sb.insert(offset, l);
        return this;
    }

    /**
     * Inserts the string representation of the specified {@code float} value at
     * the specified {@code offset}. The {@code float} value is converted to a
     * string according to the rule defined by {@link String#valueOf(float)}.
     *
     * @param offset
     *            the index to insert at.
     * @param f
     *            the {@code float} value to insert.
     * @return this builder.
     * @throws StringIndexOutOfBoundsException
     *             if {@code offset} is negative or greater than the current
     *             {@code length()}.
     * @see String#valueOf(float)
     */
    public StringBuilder insert(int offset, float f) {
    	sb.insert(offset, f);
        return this;
    }

    /**
     * Inserts the string representation of the specified {@code double} value
     * at the specified {@code offset}. The {@code double} value is converted
     * to a String according to the rule defined by
     * {@link String#valueOf(double)}.
     *
     * @param offset
     *            the index to insert at.
     * @param d
     *            the {@code double} value to insert.
     * @return this builder.
     * @throws StringIndexOutOfBoundsException
     *             if {@code offset} is negative or greater than the current
     *             {@code length()}.
     * @see String#valueOf(double)
     */
    public StringBuilder insert(int offset, double d) {
    	sb.insert(offset, d);
        return this;
    }

    /**
     * Inserts the string representation of the specified {@code Object} at the
     * specified {@code offset}. The {@code Object} value is converted to a
     * String according to the rule defined by {@link String#valueOf(Object)}.
     *
     * @param offset
     *            the index to insert at.
     * @param obj
     *            the {@code Object} to insert.
     * @return this builder.
     * @throws StringIndexOutOfBoundsException
     *             if {@code offset} is negative or greater than the current
     *             {@code length()}.
     * @see String#valueOf(Object)
     */
    public StringBuilder insert(int offset, Object obj) {
    	sb.insert(offset, obj);
        return this;
    }

    /**
     * Inserts the specified string at the specified {@code offset}. If the
     * specified string is null, then the String {@code "null"} is inserted.
     *
     * @param offset
     *            the index to insert at.
     * @param str
     *            the {@code String} to insert.
     * @return this builder.
     * @throws StringIndexOutOfBoundsException
     *             if {@code offset} is negative or greater than the current
     *             {@code length()}.
     */
    public StringBuilder insert(int offset, String str) {
    	sb.insert(offset, str);
        return this;
    }

    /**
     * Inserts the string representation of the specified {@code char[]} at the
     * specified {@code offset}. The {@code char[]} value is converted to a
     * String according to the rule defined by {@link String#valueOf(char[])}.
     *
     * @param offset
     *            the index to insert at.
     * @param ch
     *            the {@code char[]} to insert.
     * @return this builder.
     * @throws StringIndexOutOfBoundsException
     *             if {@code offset} is negative or greater than the current
     *             {@code length()}.
     * @see String#valueOf(char[])
     */
    public StringBuilder insert(int offset, char[] ch) {
    	sb.insert(offset, ch);
        return this;
    }

    /**
     * Inserts the string representation of the specified subsequence of the
     * {@code char[]} at the specified {@code offset}. The {@code char[]} value
     * is converted to a String according to the rule defined by
     * {@link String#valueOf(char[],int,int)}.
     *
     * @param offset
     *            the index to insert at.
     * @param str
     *            the {@code char[]} to insert.
     * @param strOffset
     *            the inclusive index.
     * @param strLen
     *            the number of characters.
     * @return this builder.
     * @throws StringIndexOutOfBoundsException
     *             if {@code offset} is negative or greater than the current
     *             {@code length()}, or {@code strOffset} and {@code strLen} do
     *             not specify a valid subsequence.
     * @see String#valueOf(char[],int,int)
     */
    public StringBuilder insert(int offset, char[] str, int strOffset,
            int strLen) {
        sb.insert(offset, str, strOffset, strLen);
        return this;
    }

    /**
     * Replaces the specified subsequence in this builder with the specified
     * string.
     *
     * @param start
     *            the inclusive begin index.
     * @param end
     *            the exclusive end index.
     * @param str
     *            the replacement string.
     * @return this builder.
     * @throws StringIndexOutOfBoundsException
     *             if {@code start} is negative, greater than the current
     *             {@code length()} or greater than {@code end}.
     * @throws NullPointerException
     *            if {@code str} is {@code null}.
     */
    public StringBuilder replace(int start, int end, String str) {
    	sb.replace(start, end, str);
        return this;
    }

    /**
     * Reverses the order of characters in this builder.
     *
     * @return this buffer.
     */
    public StringBuilder reverse() {
    	sb.reverse();
        return this;
    }

    /**
     * Returns the contents of this builder.
     *
     * @return the string representation of the data in this builder.
     */
    //@Override
    public String toString() {
    	return sb.toString();
    }
    
    public int length()
    {
    	return sb.length();
    }
    
    public CharSequence subSequence(int start, int end) {
    	return sb.toString().subSequence(start, end);
	}
    
}
