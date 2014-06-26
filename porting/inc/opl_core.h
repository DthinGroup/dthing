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

 #ifndef __OPL_CORE_H__
 #define __OPL_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif
 
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
 * @return Time in <b>milliseconds</b> (1.0e-3 seconds) since
 *         00:00 January 1, 1970 UTC.
 */
int64_t OPL_core_getCurrentTimeMillis();

/**
 * Print a character to the debugging log.
 *
 * The output of the <code>out</code> and <code>err</code> streams in
 * <code>java.lang.System</code> is redirected to this function.
 * <p>
 * It is possible to receive 16-bit unicode characters through this function,
 * but most debugging information only uses 7-bit ASCII.  If a port is not able
 * to handle unicode characters, it is suggested that they are replaced by
 * a suitable alternative (for example '?').
 *
 * A port is also free to completely discard all of this output, but there are
 * many classes in Jbed have a compile-time switch to print debugging
 * information, and this can be a very useful debugging technique.
 *
 * @param chr the character to print
 */
void OPL_core_logChar(int chr);


#ifdef __cplusplus
}
#endif

#endif //__OPL_CORE_H__
