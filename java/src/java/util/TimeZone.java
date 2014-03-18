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
 * {@code TimeZone} represents a time zone, primarily used for configuring a {@link Calendar} or
 * {@link java.text.SimpleDateFormat} instance.
 *
 * <p>Most applications will use {@link #getDefault} which returns a {@code TimeZone} based on
 * the time zone where the program is running.
 *
 * <p>You can also get a specific {@code TimeZone} {@link #getTimeZone by id}.
 *
 * <p>It is highly unlikely you'll ever want to use anything but the factory methods yourself.
 * Let classes like {@link Calendar} and {@link java.text.SimpleDateFormat} do the date
 * computations for you.
 *
 * <p>If you do need to do date computations manually, there are two common cases to take into
 * account:
 * <ul>
 * <li>Somewhere like California, where daylight time is used.
 * The {@link #useDaylightTime} method will always return true, and {@link #inDaylightTime}
 * must be used to determine whether or not daylight time applies to a given {@code Date}.
 * The {@link #getRawOffset} method will return a raw offset of (in this case) -8 hours from UTC,
 * which isn't usually very useful. More usefully, the {@link #getOffset} methods return the
 * actual offset from UTC <i>for a given point in time</i>; this is the raw offset plus (if the
 * point in time is {@link #inDaylightTime in daylight time}) the applicable
 * {@link #getDSTSavings DST savings} (usually, but not necessarily, 1 hour).
 * <li>Somewhere like Japan, where daylight time is not used.
 * The {@link #useDaylightTime} and {@link #inDaylightTime} methods both always return false,
 * and the raw and actual offsets will always be the same.
 * </ul>
 *
 * <p>Note the type returned by the factory methods {@link #getDefault} and {@link #getTimeZone} is
 * implementation dependent. This may introduce serialization incompatibility issues between
 * different implementations. Android returns instances of {@link SimpleTimeZone} so that
 * the bytes serialized by Android can be deserialized successfully on other
 * implementations, but the reverse compatibility cannot be guaranteed.
 *
 * @see Calendar
 * @see GregorianCalendar
 * @see SimpleDateFormat
 * @see SimpleTimeZone
 */
public abstract class TimeZone {
    private static final long serialVersionUID = 3581463369166924961L;

    /**
     * The short display name style, such as {@code PDT}. Requests for this
     * style may yield GMT offsets like {@code GMT-08:00}.
     */
    public static final int SHORT = 0;

    /**
     * The long display name style, such as {@code Pacific Daylight Time}.
     * Requests for this style may yield GMT offsets like {@code GMT-08:00}.
     */
    public static final int LONG = 1;

    static final TimeZone GMT = new SimpleTimeZone(0, "GMT"); // Greenwich Mean Time

    //private static TimeZone defaultTimeZone;

    private String ID;

    public TimeZone() {}

    /**
     * Returns a new time zone with the same ID, raw offset, and daylight
     * savings time rules as this time zone.
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
     * Returns the system's installed time zone IDs. Any of these IDs can be
     * passed to {@link #getTimeZone} to lookup the corresponding time zone
     * instance.
     */
    public static synchronized String[] getAvailableIDs() {
        //@UN-IMPLEMENT;
    	return null;
    }

    /**
     * Returns the IDs of the time zones whose offset from UTC is {@code
     * offsetMillis}. Any of these IDs can be passed to {@link #getTimeZone} to
     * lookup the corresponding time zone instance.
     *
     * @return a possibly-empty array.
     */
    public static synchronized String[] getAvailableIDs(int offsetMillis) {
        //@UN-IMPLEMENT;
    	return null;
    }

    /**
     * Returns the user's preferred time zone. This may have been overridden for
     * this process with {@link #setDefault}.
     *
     * <p>Since the user's time zone changes dynamically, avoid caching this
     * value. Instead, use this method to look it up for each use.
     */
    public static synchronized TimeZone getDefault() {
        //@UN-IMPLEMENT;
    	return null;
    }

    /**
     * Returns the ID of this {@code TimeZone}, such as
     * {@code America/Los_Angeles}, {@code GMT-08:00} or {@code UTC}.
     */
    public String getID() {
        return ID;
    }


    /**
     * Returns the offset in milliseconds from UTC for this time zone at {@code
     * time}. The offset includes daylight savings time if the specified
     * date is within the daylight savings time period.
     *
     * @param time the date in milliseconds since January 1, 1970 00:00:00 UTC
     */
    public int getOffset(long time) {
        //@UN-IMPLEMENT;
    	return 0;
    }

    /**
     * Returns this time zone's offset in milliseconds from UTC at the specified
     * date and time. The offset includes daylight savings time if the date
     * and time is within the daylight savings time period.
     *
     * <p>This method is intended to be used by {@link Calendar} to compute
     * {@link Calendar#DST_OFFSET} and {@link Calendar#ZONE_OFFSET}. Application
     * code should have no reason to call this method directly. Each parameter
     * is interpreted in the same way as the corresponding {@code Calendar}
     * field. Refer to {@link Calendar} for specific definitions of this
     * method's parameters.
     */
    public abstract int getOffset(int era, int year, int month, int day,
            int dayOfWeek, int timeOfDayMillis);

    /**
     * Returns the offset in milliseconds from UTC of this time zone's standard
     * time.
     */
    public abstract int getRawOffset();

    /**
     * Returns a {@code TimeZone} suitable for {@code id}, or {@code GMT} on failure.
     *
     * <p>An id can be an Olson name of the form <i>Area</i>/<i>Location</i>, such
     * as {@code America/Los_Angeles}. The {@link #getAvailableIDs} method returns
     * the supported names.
     *
     * <p>This method can also create a custom {@code TimeZone} using the following
     * syntax: {@code GMT[+|-]hh[[:]mm]}. For example, {@code TimeZone.getTimeZone("GMT+14:00")}
     * would return an object with a raw offset of +14 hours from UTC, and which does <i>not</i>
     * use daylight savings. These are rarely useful, because they don't correspond to time
     * zones actually in use.
     *
     * <p>Other than the special cases "UTC" and "GMT" (which are synonymous in this context,
     * both corresponding to UTC), Android does not support the deprecated three-letter time
     * zone IDs used in Java 1.1.
     */
    public static synchronized TimeZone getTimeZone(String id) {
        //@UN-IMPLEMENT;
    	return null;
    }


    /**
     * Returns true if this time zone has a future transition to or from
     * daylight savings time.
     *
     * <p><strong>Warning:</strong> this returns false for time zones like
     * {@code Asia/Kuala_Lumpur} that have previously used DST but do not
     * currently. A hypothetical country that has never observed daylight
     * savings before but plans to start next year would return true.
     *
     * <p><strong>Warning:</strong> this returns true for time zones that use
     * DST, even when it is not active.
     *
     * <p>Use {@link #inDaylightTime} to find out whether daylight savings is
     * in effect at a specific time.
     *
     * <p>Most applications should not use this method.
     */
    public abstract boolean useDaylightTime();
}
