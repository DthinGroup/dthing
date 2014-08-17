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
 * strings, where all accesses are synchronized. This class has mostly been replaced
 * by {@link StringBuilder} because this synchronization is rarely useful. This
 * class is mainly used to interact with legacy APIs that expose it.
 *
 * <p>For particularly complex string-building needs, consider {@link java.util.Formatter}.
 *
 * <p>The majority of the modification methods on this class return {@code
 * this} so that method calls can be chained together. For example:
 * {@code new StringBuffer("a").append("b").append("c").toString()}.
 *
 * @see CharSequence
 * @see Appendable
 * @see StringBuilder
 * @see String
 * @see String#format
 * @since 1.0
 */
public final class StringBuffer {

    private static final long serialVersionUID = 3388685877147921107L;

    /**
     * The value is used for character storage.
     *
     * @serial
     */
    private char value[];

    /**
     * The count is the number of characters in the buffer.
     *
     * @serial
     */
    private int count;
    
    /**
     * A flag indicating whether the buffer is shared.
     * When shared, this field refers to the string that is sharing
     * the character array.  null means not-shared.
     *
     * Note that due to the StringBuffer.toString() being unsynchronized,
     * sharedStr should be nullified before reducing count. See toString().
     *
     * This field is not serialisable.
     */
    private transient String sharedStr;
    
    /**
     * Constructs a new StringBuffer using the default capacity which is 16.
     */
    public StringBuffer() {
    	value = new char[16];
    }

    /**
     * Constructs a new StringBuffer using the specified capacity.
     *
     * @param capacity
     *            the initial capacity.
     */
    public StringBuffer(int capacity) {
    	value = new char[capacity];
    }

    /**
     * Constructs a new StringBuffer containing the characters in the specified
     * string. The capacity of the new buffer will be the length of the
     * {@code String} plus the default capacity.
     *
     * @param string
     *            the string content with which to initialize the new instance.
     * @throws NullPointerException
     *            if {@code string} is {@code null}.
     */
    public StringBuffer(String string) {
        value = new char[string.count + 16];
        count = string.count;
        System.arraycopy(string.value, string.offset, value, 0, string.count);
    }

    /**
     * Adds the string representation of the specified boolean to the end of
     * this StringBuffer.
     * <p>
     * If the argument is {@code true} the string {@code "true"} is appended,
     * otherwise the string {@code "false"} is appended.
     *
     * @param b
     *            the boolean to append.
     * @return this StringBuffer.
     * @see String#valueOf(boolean)
     */
    public StringBuffer append(boolean b) {
        return append(b ? "true" : "false");
    }

    /**
     * Adds the specified character to the end of this buffer.
     *
     * @param ch
     *            the character to append.
     * @return this StringBuffer.
     * @see String#valueOf(char)
     */
    public synchronized StringBuffer append(char ch) {

        // This routine is called a lot in some applications, so optimise
        // for the common case (when the buffer is big enough).
        int myCount = count;
        if (myCount < value.length) {
            value[myCount] = ch;
            count = myCount+1;
            return this;
        }
        expandCapacity(myCount+1);
        value[myCount] = ch;
        count = myCount+1;
        return this;
    }

    /**
     * Adds the string representation of the specified double to the end of this
     * StringBuffer.
     *
     * @param d
     *            the double to append.
     * @return this StringBuffer.
     * @see String#valueOf(double)
     */
    public StringBuffer append(double d) {
    	return append(Double.toString(d));
    }

    /**
     * Adds the string representation of the specified float to the end of this
     * StringBuffer.
     *
     * @param f
     *            the float to append.
     * @return this StringBuffer.
     * @see String#valueOf(float)
     */
    public StringBuffer append(float f) {
    	return append(Float.toString(f));
    }

    /**
     * Adds the string representation of the specified integer to the end of
     * this StringBuffer.
     *
     * @param i
     *            the integer to append.
     * @return this StringBuffer.
     * @see String#valueOf(int)
     */
    public StringBuffer append(int i) {
    	return append(Integer.toString(i, 10));
    }

    /**
     * Adds the string representation of the specified long to the end of this
     * StringBuffer.
     *
     * @param l
     *            the long to append.
     * @return this StringBuffer.
     * @see String#valueOf(long)
     */
    public StringBuffer append(long l) {
    	return append(Long.toString(l, 10));
    }

    /**
     * Adds the string representation of the specified object to the end of this
     * StringBuffer.
     * <p>
     * If the specified object is {@code null} the string {@code "null"} is
     * appended, otherwise the objects {@code toString} is used to get its
     * string representation.
     *
     * @param obj
     *            the object to append (may be null).
     * @return this StringBuffer.
     * @see String#valueOf(Object)
     */
    public synchronized StringBuffer append(Object obj) {
    	return append((obj != null) ? obj.toString() : "null");
    }

    /**
     * Adds the specified string to the end of this buffer.
     * <p>
     * If the specified string is {@code null} the string {@code "null"} is
     * appended, otherwise the contents of the specified string is appended.
     *
     * @param string
     *            the string to append (may be null).
     * @return this StringBuffer.
     */
    public synchronized StringBuffer append(String string) {
        if (string == null) {
        	string = "null";
        }
        int len = string.count;
        int newcount = count + len;
        if (newcount > value.length)
            expandCapacity(newcount);
        System.arraycopy(string.value, string.offset, value, count, len);
        count = newcount;
        return this;
    }

    /**
     * Adds the specified StringBuffer to the end of this buffer.
     * <p>
     * If the specified StringBuffer is {@code null} the string {@code "null"}
     * is appended, otherwise the contents of the specified StringBuffer is
     * appended.
     *
     * @param sb
     *            the StringBuffer to append (may be null).
     * @return this StringBuffer.
     *
     * @since 1.4
     */
    public synchronized StringBuffer append(StringBuffer sb) {
        if (sb == null) {
            return append((String)null);
        }

        int len = sb.length();
        int newcount = count + len;
        if (newcount > value.length)
            expandCapacity(newcount);
        sb.getChars(0, len, value, count);
        count = newcount;
        return this;
    }

    /**
     * Adds the character array to the end of this buffer.
     *
     * @param chars
     *            the character array to append.
     * @return this StringBuffer.
     * @throws NullPointerException
     *            if {@code chars} is {@code null}.
     */
    public synchronized StringBuffer append(char[] chars) {
	    if (chars == null) {
	        throw new NullPointerException();
	    }

	    int len = chars.length;
	    int newcount = count + len;
	    if (newcount > value.length)
	        expandCapacity(newcount);
	    System.arraycopy(chars, 0, value, count, len);
	    count = newcount;
	    return this;
    }

    /**
     * Adds the specified sequence of characters to the end of this buffer.
     *
     * @param chars
     *            the character array to append.
     * @param start
     *            the starting offset.
     * @param length
     *            the number of characters.
     * @return this StringBuffer.
     * @throws ArrayIndexOutOfBoundsException
     *             if {@code length < 0} , {@code start < 0} or {@code start +
     *             length > chars.length}.
     * @throws NullPointerException
     *            if {@code chars} is {@code null}.
     */
    public synchronized StringBuffer append(char[] chars, int start, int length) {
        int newcount = count + length;
        if (newcount > value.length)
            expandCapacity(newcount);
        System.arraycopy(chars, start, value, count, length);
        count = newcount;
        return this;
    }

    /**
     * This implements the expansion semantics of ensureCapacity but is
     * unsynchronized for use internally by methods which have already
     * tested the capacity.
     *
     * @see java.lang.StringBuffer#ensureCapacity(int)
     */
    private void expandCapacity(int minimumCapacity) {
        int newCapacity = (value.length + 1) * 2;
        if (newCapacity < minimumCapacity) {
            newCapacity = minimumCapacity;
        }
        char newValue[] = new char[newCapacity];
        System.arraycopy(value, 0, newValue, 0, count);
        value = newValue;
        sharedStr = null;
    }

    /**
     * Returns the current capacity of the String buffer. The capacity
     * is the amount of storage available for newly inserted
     * characters; beyond which an allocation will occur.
     *
     * @return  the current capacity of this string buffer.
     */
    public int capacity() {
        return value.length;
    }
    
    /**
     * The specified character of the sequence currently represented by
     * the string buffer, as indicated by the <code>index</code> argument,
     * is returned. The first character of a string buffer is at index
     * <code>0</code>, the next at index <code>1</code>, and so on, for
     * array indexing.
     * <p>
     * The index argument must be greater than or equal to
     * <code>0</code>, and less than the length of this string buffer.
     *
     * @param      index   the index of the desired character.
     * @return     the character at the specified index of this string buffer.
     * @exception  IndexOutOfBoundsException  if <code>index</code> is
     *             negative or greater than or equal to <code>length()</code>.
     * @see        java.lang.StringBuffer#length()
     */
    public char charAt(int index) {
        if (index >= 0 && index < count) {
            return value[index];
        }
        throw new StringIndexOutOfBoundsException();
    }
    
    /**
     * Copies the buffer value.  This is normally only called when sharedStr
     * is non-null.  It should only be called from a noReschedule method.
     */
    private final void copy() {
        char newValue[] = new char[value.length];
        System.arraycopy(value, 0, newValue, 0, count);
        value = newValue;
        sharedStr = null;
    }

    
    /**
     * Deletes a range of characters.
     *
     * @param start
     *            the offset of the first character.
     * @param end
     *            the offset one past the last character.
     * @return this StringBuffer.
     * @throws StringIndexOutOfBoundsException
     *             if {@code start < 0}, {@code start > end} or {@code end >
     *             length()}.
     */
    public synchronized StringBuffer delete(int start, int end) {
        int len = end - start;

        // if (start < 0 || start > end)
        if ((start | end | len) < 0)
            throw new StringIndexOutOfBoundsException();

        if (len > 0) {
            if (sharedStr != null)
                copy();
            System.arraycopy(value, end, value, start, count-end);
            count -= len;
        }
        return this;
    }

    /**
     * Deletes the character at the specified offset.
     *
     * @param location
     *            the offset of the character to delete.
     * @return this StringBuffer.
     * @throws StringIndexOutOfBoundsException
     *             if {@code location < 0} or {@code location >= length()}
     */
    public synchronized StringBuffer deleteCharAt(int location) {
        if ((location < 0) || (location >= count))
            throw new StringIndexOutOfBoundsException();
        if (sharedStr != null)
            copy();
        System.arraycopy(value, location+1, value, location, count-location-1);
        count--;
        return this;
    }

    /**
     * Ensures that the capacity of the buffer is at least equal to the
     * specified minimum.
     * If the current capacity of this string buffer is less than the
     * argument, then a new internal buffer is allocated with greater
     * capacity. The new capacity is the larger of:
     * <ul>
     * <li>The <code>minimumCapacity</code> argument.
     * <li>Twice the old capacity, plus <code>2</code>.
     * </ul>
     * If the <code>minimumCapacity</code> argument is nonpositive, this
     * method takes no action and simply returns.
     *
     * @param   minimumCapacity   the minimum desired capacity.
     */
    public synchronized void ensureCapacity(int min) {
        if (min <= value.length)
            return;
        expandCapacity(min);
    }

    /**
     * Copies the requested sequence of characters to the {@code char[]} passed
     * starting at {@code idx}.
     *
     * @param start
     *            the starting offset of characters to copy.
     * @param end
     *            the ending offset of characters to copy.
     * @param buffer
     *            the destination character array.
     * @param idx
     *            the starting offset in the character array.
     * @throws IndexOutOfBoundsException
     *             if {@code start < 0}, {@code end > length()}, {@code start >
     *             end}, {@code index < 0}, {@code end - start > buffer.length -
     *             index}
     */
    public synchronized void getChars(int start, int end, char[] buffer, int idx) {
        if ((start | end | (count - end) | (end - start)) >= 0) {
            System.arraycopy(value, start, buffer, idx, end - start);
            return;
        }
        throw new StringIndexOutOfBoundsException();
    }

    /**
     *  The following method is needed by String to efficiently
     *  convert a StringBuffer into a String.  It is not public.
     *  It shouldn't be called by anyone but String.
     *  
     *  @return the String buffer value.
     */
    final char[] getValue() { 
    	return value; 
	}
    
    /**
     * Returns the index within this string of the first occurrence of the
     * specified substring, starting at the specified index.  The integer
     * returned is the smallest value <tt>k</tt> for which:
     * <blockquote><pre>
     *     k >= Math.min(fromIndex, str.length()) &&
     *                   this.toString().startsWith(str, k)
     * </pre></blockquote>
     * If no such value of <i>k</i> exists, then -1 is returned.
     *
     * @param   str         the substring for which to search.
     * @param   fromIndex   the index from which to start the search.
     * @return  the index within this string of the first occurrence of the
     *          specified substring, starting at the specified index.
     * @exception java.lang.NullPointerException if <code>str</code> is
     *            <code>null</code>.
     * @since   1.4
     */
    public synchronized int indexOf(String subString, int start) {
        return String.indexOf(this.value, 0, this.count,
        		subString.value, subString.offset, subString.count, start);
    }

    /**
     * Inserts the character into this buffer at the specified offset.
     *
     * @param index
     *            the index at which to insert.
     * @param ch
     *            the character to insert.
     * @return this buffer.
     * @throws ArrayIndexOutOfBoundsException
     *             if {@code index < 0} or {@code index > length()}.
     */
    public synchronized StringBuffer insert(int index, char ch) {

        int newcount = count + 1;
        if (newcount > value.length)
            expandCapacity(newcount);
        else if (sharedStr != null)
            copy();
        System.arraycopy(value, index, value, index + 1, count - index);
        value[index] = ch;
        count = newcount;
        return this;
    }

    /**
     * Inserts the string representation of the specified boolean into this
     * buffer at the specified offset.
     *
     * @param index
     *            the index at which to insert.
     * @param b
     *            the boolean to insert.
     * @return this buffer.
     * @throws StringIndexOutOfBoundsException
     *             if {@code index < 0} or {@code index > length()}.
     */
    public StringBuffer insert(int index, boolean b) {
        return insert(index, b ? "true" : "false");
    }

    /**
     * Inserts the string representation of the specified integer into this
     * buffer at the specified offset.
     *
     * @param index
     *            the index at which to insert.
     * @param i
     *            the integer to insert.
     * @return this buffer.
     * @throws StringIndexOutOfBoundsException
     *             if {@code index < 0} or {@code index > length()}.
     */
    public StringBuffer insert(int index, int i) {
        return insert(index, Integer.toString(i));
    }

    /**
     * Inserts the string representation of the specified long into this buffer
     * at the specified offset.
     *
     * @param index
     *            the index at which to insert.
     * @param l
     *            the long to insert.
     * @return this buffer.
     * @throws StringIndexOutOfBoundsException
     *             if {@code index < 0} or {@code index > length()}.
     */
    public StringBuffer insert(int index, long l) {
        return insert(index, Long.toString(l));
    }

    /**
     * Inserts the string representation of the specified into this buffer
     * double at the specified offset.
     *
     * @param index
     *            the index at which to insert.
     * @param d
     *            the double to insert.
     * @return this buffer.
     * @throws StringIndexOutOfBoundsException
     *             if {@code index < 0} or {@code index > length()}.
     */
    public StringBuffer insert(int index, double d) {
        return insert(index, Double.toString(d));
    }

    /**
     * Inserts the string representation of the specified float into this buffer
     * at the specified offset.
     *
     * @param index
     *            the index at which to insert.
     * @param f
     *            the float to insert.
     * @return this buffer.
     * @throws StringIndexOutOfBoundsException
     *             if {@code index < 0} or {@code index > length()}.
     */
    public StringBuffer insert(int index, float f) {
        return insert(index, Float.toString(f));
    }

    /**
     * Inserts the string representation of the specified object into this
     * buffer at the specified offset.
     * <p>
     * If the specified object is {@code null}, the string {@code "null"} is
     * inserted, otherwise the objects {@code toString} method is used to get
     * its string representation.
     *
     * @param index
     *            the index at which to insert.
     * @param obj
     *            the object to insert (may be null).
     * @return this buffer.
     * @throws StringIndexOutOfBoundsException
     *             if {@code index < 0} or {@code index > length()}.
     */
    public StringBuffer insert(int index, Object obj) {
        return insert(index, obj == null ? "null" : obj.toString());
    }

    /**
     * Inserts the string into this buffer at the specified offset.
     * <p>
     * If the specified string is {@code null}, the string {@code "null"} is
     * inserted, otherwise the contents of the string is inserted.
     *
     * @param index
     *            the index at which to insert.
     * @param string
     *            the string to insert (may be null).
     * @return this buffer.
     * @throws StringIndexOutOfBoundsException
     *             if {@code index < 0} or {@code index > length()}.
     */
    public synchronized StringBuffer insert(int index, String string) {
        if ((index < 0) || (index > count)) {
            throw new StringIndexOutOfBoundsException();
        }

        if (string == null) {
        	string = "null";
        }
        int len = string.count;
        int newcount = count + len;
        if (newcount > value.length)
            expandCapacity(newcount);
        else if (sharedStr != null)
            copy();
        System.arraycopy(value, index, value, index + len, count - index);
        System.arraycopy(string.value, string.offset, value, index, len);
        count = newcount;
        return this;
    }

    /**
     * Inserts the character array into this buffer at the specified offset.
     *
     * @param index
     *            the index at which to insert.
     * @param chars
     *            the character array to insert.
     * @return this buffer.
     * @throws StringIndexOutOfBoundsException
     *             if {@code index < 0} or {@code index > length()}.
     * @throws NullPointerException
     *            if {@code chars} is {@code null}.
     */
    public synchronized StringBuffer insert(int index, char[] chars) {
        if ((index < 0) || (index > count)) {
            throw new StringIndexOutOfBoundsException();
        }
        int len = chars.length;
        int newcount = count + len;
        if (newcount > value.length)
            expandCapacity(newcount);
        else if (sharedStr != null)
            copy();
        System.arraycopy(value, index, value, index + len, count - index);
        System.arraycopy(chars, 0, value, index, len);
        count = newcount;
        return this;
    }

    /**
     * Inserts the specified subsequence of characters into this buffer at the
     * specified index.
     *
     * @param index
     *            the index at which to insert.
     * @param chars
     *            the character array to insert.
     * @param start
     *            the starting offset.
     * @param length
     *            the number of characters.
     * @return this buffer.
     * @throws NullPointerException
     *             if {@code chars} is {@code null}.
     * @throws StringIndexOutOfBoundsException
     *             if {@code length < 0}, {@code start < 0}, {@code start +
     *             length > chars.length}, {@code index < 0} or {@code index >
     *             length()}
     */
    public synchronized StringBuffer insert(int index, char[] chars, int start, int length) {
        if ((index < 0) || (index > count))
            throw new StringIndexOutOfBoundsException();
        if ((start < 0) || (start + length < 0) || (start + length > chars.length))
            throw new StringIndexOutOfBoundsException();
        if (length < 0)
            throw new StringIndexOutOfBoundsException();
        int newCount = count + length;
        if (newCount > value.length)
            expandCapacity(newCount);
        else if (sharedStr != null)
            copy();
        System.arraycopy(value, index, value, index + length, count - index);
        System.arraycopy(chars, start, value, index, length);
        count = newCount;
        return this;
    }

    /**
     * Returns the index within this string of the rightmost occurrence
     * of the specified substring.  The rightmost empty string "" is
     * considered to occur at the index value <code>this.length()</code>.
     * The returned index is the largest value <i>k</i> such that
     * <blockquote><pre>
     * this.toString().startsWith(str, k)
     * </pre></blockquote>
     * is true.
     *
     * @param   str   the substring to search for.
     * @return  if the string argument occurs one or more times as a substring
     *          within this object, then the index of the first character of
     *          the last such substring is returned. If it does not occur as
     *          a substring, <code>-1</code> is returned.
     * @exception java.lang.NullPointerException  if <code>str</code> is
     *          <code>null</code>.
     * @since   1.4
     */
    public int lastIndexOf(String string) {
        return lastIndexOf(string, count);
    }
    
    /**
     * Returns the index within this string of the last occurrence of the
     * specified substring. The integer returned is the largest value <i>k</i>
     * such that:
     * <blockquote><pre>
     *     k <= Math.min(fromIndex, str.length()) &&
     *                   this.toString().startsWith(str, k)
     * </pre></blockquote>
     * If no such value of <i>k</i> exists, then -1 is returned.
     *
     * @param   str         the substring to search for.
     * @param   fromIndex   the index to start the search from.
     * @return  the index within this string of the last occurrence of the
     *          specified substring.
     * @exception java.lang.NullPointerException if <code>str</code> is
     *          <code>null</code>.
     * @since   1.4
     */
    public synchronized int lastIndexOf(String subString, int start) {
        return String.lastIndexOf(this.value, 0, this.count,
        		subString.value, subString.offset, subString.count, start);
    }
    

    /**
     * Returns the length of this string.
     * The length is equal to the number of 16-bit
     * Unicode characters in the string.
     *
     * @return  the length of the sequence of characters represented by this
     *          object.
     */
    public int length() {
        return count;
    }

    
    /**
     * Replaces the characters in the specified range with the contents of the
     * specified string.
     *
     * @param start
     *            the inclusive begin index.
     * @param end
     *            the exclusive end index.
     * @param string
     *            the string that will replace the contents in the range.
     * @return this buffer.
     * @throws StringIndexOutOfBoundsException
     *             if {@code start} or {@code end} are negative, {@code start}
     *             is greater than {@code end} or {@code end} is greater than
     *             the length of {@code s}.
     */
    public synchronized StringBuffer replace(int start, int end, String string) {
        if (end > count)
            end = count;
        if ((start | end | (end - start)) < 0)
            throw new StringIndexOutOfBoundsException();

        int len = string.length();
        int newCount = count + len - (end - start);
        if (newCount > value.length)
            expandCapacity(newCount);
        else if (sharedStr != null)
            copy();

        System.arraycopy(value, end, value, start + len, count - end);
        System.arraycopy(string.value, string.offset, value, start, len);
        count = newCount;
        return this;
    }

    /**
     * Reverses the order of characters in this buffer.
     *
     * @return this buffer.
     */
    public synchronized StringBuffer reverse() {
        if (sharedStr != null)
            copy();
        int n = count - 1;
        char[] val = value;
        for (int j = (n-1) >> 1; j >= 0; --j) {
            char temp = val[j];
            val[j] = val[n - j];
            val[n - j] = temp;
        }
        return this;
    }

    /**
     * The specified character of the sequence currently represented by
     * the string buffer, as indicated by the <code>index</code> argument,
     * is returned. The first character of a string buffer is at index
     * <code>0</code>, the next at index <code>1</code>, and so on, for
     * array indexing.
     * <p>
     * The index argument must be greater than or equal to
     * <code>0</code>, and less than the length of this string buffer.
     *
     * @param      index   the index of the desired character.
     * @return     the character at the specified index of this string buffer.
     * @exception  IndexOutOfBoundsException  if <code>index</code> is
     *             negative or greater than or equal to <code>length()</code>.
     * @see        java.lang.StringBuffer#length()
     */
    public synchronized void setCharAt(int index, char ch) {
        if ((index < 0) || (index >= count)) {
            throw new StringIndexOutOfBoundsException();
        }
        if (sharedStr != null)
            copy();
        value[index] = ch;
    }

    /**
     * Sets the length of this String buffer.
     * This string buffer is altered to represent a new character sequence
     * whose length is specified by the argument. For every nonnegative
     * index <i>k</i> less than <code>newLength</code>, the character at
     * index <i>k</i> in the new character sequence is the same as the
     * character at index <i>k</i> in the old sequence if <i>k</i> is less
     * than the length of the old character sequence; otherwise, it is the
     * null character <code>'\u0000'</code>.
     *
     * In other words, if the <code>newLength</code> argument is less than
     * the current length of the string buffer, the string buffer is
     * truncated to contain exactly the number of characters given by the
     * <code>newLength</code> argument.
     * <p>
     * If the <code>newLength</code> argument is greater than or equal
     * to the current length, sufficient null characters
     * (<code>'&#92;u0000'</code>) are appended to the string buffer so that
     * length becomes the <code>newLength</code> argument.
     * <p>
     * The <code>newLength</code> argument must be greater than or equal
     * to <code>0</code>.
     *
     * @param      newLength   the new length of the buffer.
     * @exception  StringIndexOutOfBoundsException  if the
     *               <code>newLength</code> argument is negative.
     * @see        java.lang.StringBuffer#length()
     */
    public synchronized void setLength(int length) {
        if (length < 0) {
            throw new StringIndexOutOfBoundsException();
        }

        if (length > value.length) {
            expandCapacity(length);
        }

        if (length >= count) { // Expand StringBuffer
            // No need to copy() as any shared string will not change
            for (int i = count; i < length; i++) {
                value[i] = '\0';
            }
            count = length;
        } else if (sharedStr == null) { // Truncate unshared StringBuffer
            count = length;
        } else { // Truncate a shared StringBuffer
            // NB Reduce count AFTER setting sharedStr to null
            sharedStr = null;
            count = length;

            if (length > 0) {
                copy();
            } else {
                // If newLength is zero, assume the StringBuffer is being
                // stripped for reuse; Make new buffer of default size
                value = new char[16];
            }
        }
    }

    /**
     * Returns a new <code>String</code> that contains a subsequence of
     * characters currently contained in this <code>StringBuffer</code>.The
     * substring begins at the specified index and extends to the end of the
     * <code>StringBuffer</code>.
     *
     * @param	   start	The beginning index, inclusive.
     * @return	   The new string.
     * @exception  StringIndexOutOfBoundsException	if <code>start</code> is
     *			   less than zero, or greater than the length of this
     *			   <code>StringBuffer</code>.
     * @since	   1.2
     */
    public synchronized String substring(int start) {
    	return substring(start, this.count);
    }

    /**
     * Returns a new <code>String</code> that contains a subsequence of
     * characters currently contained in this <code>StringBuffer</code>. The
     * substring begins at the specified <code>start</code> and
     * extends to the character at index <code>end - 1</code>. An
     * exception is thrown if
     *
     * @param	   start	The beginning index, inclusive.
     * @param	   end		The ending index, exclusive.
     * @return	   The new string.
     * @exception  StringIndexOutOfBoundsException	if <code>start</code>
     *			   or <code>end</code> are negative or greater than
     *			   <code>length()</code>, or <code>start</code> is
     *			   greater than <code>end</code>.
     * @since	   1.2
     */
    public synchronized String substring(int start, int end) {
        if ((end | (this.count - end)) >= 0) {
            return new String (this.value, start, end - start);
        }
        throw new StringIndexOutOfBoundsException();
    }

    /**
     * Package private method used only by String(StringBuffer sb) to
     * efficiently convert a StringBuffer into a String.
     * @param str   String to initialise
     */
    final void shareToString(String string) {
    	string.value = value;
    	string.count = count;
        sharedStr = string;
    }
    
    /**
     * Converts to a string representing the data in this string buffer.
     * A new <code>String</code> object is allocated and initialized to
     * contain the character sequence currently represented by this
     * string buffer. This <code>String</code> is then returned. Subsequent
     * changes to the string buffer do not affect the contents of the
     * <code>String</code>.
     * <p>
     * Implementation advice: This method can be coded so as to create a new
     * <code>String</code> object without allocating new memory to hold a
     * copy of the character sequence. Instead, the string can share the
     * memory used by the string buffer. Any subsequent operation that alters
     * the content or capacity of the string buffer must then make a copy of
     * the internal buffer at that time. This strategy is effective for
     * reducing the amount of memory allocated by a string concatenation
     * operation when it is implemented using a string buffer.
     *
     * @return  a string representation of the string buffer.
     */
    //@Override
    public synchronized String toString() {
        if ((sharedStr != null) && (sharedStr.count == count)) {
            return sharedStr;
        }
        sharedStr = new String(count);
        sharedStr.value = value;
        return sharedStr;
    }
}
