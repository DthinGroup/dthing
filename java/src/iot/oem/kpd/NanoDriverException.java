/*
 * Copyright 2014 Yarlung Soft. All rights reserved.
 */

package iot.oem.kpd;

public class NanoDriverException extends RuntimeException {
    /**
     * Constructs an <code>NanoDriverException</code> with <code>null</code>
     * as its error detail message.
     */
    public NanoDriverException() {
        super();
    }

    /**
     * Constructs an <code>NanoDriverException</code> with the specified detail
     * message. The error message string <code>s</code> can later be
     * retrieved by the <code>{@link java.lang.Throwable#getMessage}</code>
     * method of class <code>java.lang.Throwable</code>.
     *
     * @param s the detail message.
     */
    public NanoDriverException(String s) {
        super(s);
    }
}
