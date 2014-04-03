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

package java.util;

/**
 * {@code Date} represents a specific moment in time, to the millisecond.
 *
 * @see System#currentTimeMillis
 * @see Calendar
 * @see GregorianCalendar
 * @see SimpleTimeZone
 * @see TimeZone
 */
public class Date {

    private static final long serialVersionUID = 7523967970034938905L;


    private transient long milliseconds;

    /**
     * Initializes this {@code Date} instance to the current time.
     */
    public Date() {
        this(System.currentTimeMillis());
    }
    
    /**
     * Initializes this {@code Date} instance using the specified millisecond value. The
     * value is the number of milliseconds since Jan. 1, 1970 GMT.
     *
     * @param milliseconds
     *            the number of milliseconds since Jan. 1, 1970 GMT.
     */
    public Date(long milliseconds) {
        this.milliseconds = milliseconds;
    }


    /**
     * Returns if this {@code Date} is after the specified Date.
     *
     * @param date
     *            a Date instance to compare.
     * @return {@code true} if this {@code Date} is after the specified {@code Date},
     *         {@code false} otherwise.
     */
    public boolean after(Date date) {
        return milliseconds > date.milliseconds;
    }

    /**
     * Returns if this {@code Date} is before the specified Date.
     *
     * @param date
     *            a {@code Date} instance to compare.
     * @return {@code true} if this {@code Date} is before the specified {@code Date},
     *         {@code false} otherwise.
     */
    public boolean before(Date date) {
        return milliseconds < date.milliseconds;
    }

    /**
     * Returns a new {@code Date} with the same millisecond value as this {@code Date}.
     *
     * @return a shallow copy of this {@code Date}.
     *
     * @see java.lang.Cloneable
     */
    @Override
    public Object clone() {
        try {
            return super.clone();
        } catch (CloneNotSupportedException e) {
            throw new AssertionError(e);
        }
    }

    /**
     * Compare the receiver to the specified {@code Date} to determine the relative
     * ordering.
     *
     * @param date
     *            a {@code Date} to compare against.
     * @return an {@code int < 0} if this {@code Date} is less than the specified {@code Date}, {@code 0} if
     *         they are equal, and an {@code int > 0} if this {@code Date} is greater.
     */
    public int compareTo(Date date) {
        if (milliseconds < date.milliseconds) {
            return -1;
        }
        if (milliseconds == date.milliseconds) {
            return 0;
        }
        return 1;
    }

    /**
     * Compares the specified object to this {@code Date} and returns if they are equal.
     * To be equal, the object must be an instance of {@code Date} and have the same millisecond
     * value.
     *
     * @param object
     *            the object to compare with this object.
     * @return {@code true} if the specified object is equal to this {@code Date}, {@code false}
     *         otherwise.
     *
     * @see #hashCode
     */
    @Override
    public boolean equals(Object object) {
        return (object == this) || (object instanceof Date)
                && (milliseconds == ((Date) object).milliseconds);
    }

    /**
     * Returns this {@code Date} as a millisecond value. The value is the number of
     * milliseconds since Jan. 1, 1970, midnight GMT.
     *
     * @return the number of milliseconds since Jan. 1, 1970, midnight GMT.
     */
    public long getTime() {
        return milliseconds;
    }

    /**
     * Returns an integer hash code for the receiver. Objects which are equal
     * return the same value for this method.
     *
     * @return this {@code Date}'s hash.
     *
     * @see #equals
     */
    @Override
    public int hashCode() {
        return (int) (milliseconds >>> 32) ^ (int) milliseconds;
    }

    /**
     * Sets this {@code Date} to the specified millisecond value. The value is the
     * number of milliseconds since Jan. 1, 1970 GMT.
     *
     * @param milliseconds
     *            the number of milliseconds since Jan. 1, 1970 GMT.
     */
    public void setTime(long milliseconds) {
        this.milliseconds = milliseconds;
    }

    /**
     * Returns a string representation of this {@code Date}.
     * The formatting is equivalent to using a {@code SimpleDateFormat} with
     * the format string "EEE MMM dd HH:mm:ss zzz yyyy", which looks something
     * like "Tue Jun 22 13:07:00 PDT 1999". The current default time zone and
     * locale are used. If you need control over the time zone or locale,
     * use {@code SimpleDateFormat} instead.
     */
    @Override
    public String toString() {
    	// TODO: to be implemented
    	return null;
    }
}
