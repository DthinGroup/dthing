
package com.yarlungsoft.util;

import com.yarlungsoft.ams.AmsConfig;

/**
 * API for sending log output.
 */
public final class Log {

    private Log() {
        /* cannot be instantiated */
    }

    /**
     * Send an AMS log message.
     *
     * @param tag Used to identify the source of a log message. It usually identifies the class or
     * activity where the log call occurs.
     * @param msg The message you would like logged.
     */
    public static void amsLog(String tag, String msg) {
        amsLog(tag, msg, null);
    }

    /**
     * Send an AMS log message and log the exception.
     *
     * @param tag Used to identify the source of a log message. It usually identifies the class or
     * activity where the log call occurs.
     * @param msg The message you would like logged.
     * @param tr An exception to log
     */
    public static void amsLog(String tag, String msg, Throwable tr) {
        if (AmsConfig.debug()) {
            log("ams", tag, msg, tr);
        }
    }

    /**
     * Send a log message
     *
     * @param tag Used to identify the source of a log message. It usually identifies the class or
     * activity where the log call occurs.
     * @param msg The message you would like logged.
     */
    public static void log(String tag, String msg) {
        log(null, tag, msg, null);
    }

    /**
     * Send a log message and log the exception.
     *
     * @param module Used to identify the module of a log message.
     * @param tag Used to identify the source of a log message. It usually identifies the class or
     * activity where the log call occurs.
     * @param msg The message you would like logged.
     * @param tr An exception to log
     */
    private static void log(String module, String tag, String msg, Throwable tr) {
        if (!TextUtils.isEmpty(msg)) {
            if (!TextUtils.isEmpty(module)) {
                tag = module + "." + tag;
            }
            System.out.println("[" + tag + "] " + msg);
        }
        if (tr != null) {
            tr.printStackTrace();
        }
    }
}
