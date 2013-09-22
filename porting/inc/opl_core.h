/**
 * Copyright (C) 2013 YarlungSoft. All Rights Reserved.
 * 
 * Created:         $Date: 2013/09/22 $
 * Last modified:	$Date: 2013/09/22 $
 * Version:         $ID: opl_core.h#1
 */

/**
 * The implementation of core VM portings.
 */
 
 
/**
 * Return the number of milliseconds since 1st January 1970 UTC.
 *
 * The value returned should be relative to coordinated universal time (UTC),
 * not local time.  However it is acceptable to return the time since
 * 1st of January 1970 in the current time zone (local time) if the target
 * does not support time zones, and does not have access to an absolute time
 * source.  If local time is used, there may be issues with applications that
 * communicate with external entities that have a more accurate notion of
 * time relative to UTC.
 *
 * This function is called by <code>java.lang.System.currentTimeMillis()</code>,
 * which many animated applications use to control their frame rate.
 * Therefore this function should be implemented efficiently.
 *
 * The returned time value should never decrease during a run of the VM, since
 * this might cause problems with applications running on the VM.
 *
 * This function is also used in the implementation of java.util.Timer, and
 * abrupt changes in the time returned may cause problems like too long or too
 * short delays.
 *
 * @see CPL_os_getMillisElapsed
 *
 * @return Time in <b>milliseconds</b> (1.0e-3 seconds) since
 *         00:00 January 1, 1970 UTC.
 */
int64_t OPL_core_getCurrentTimeMillis();