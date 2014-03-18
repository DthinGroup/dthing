package com.yarlungsoft.util;

import java.io.IOException;
import java.io.OutputStream;

public class SystemPrintStream extends OutputStream {
	
    boolean isSysOut;

    /**
     * Constructor
     * @param isSysOut is true if this stream is System.out
     */
    public SystemPrintStream(boolean isSysOut)
    {
        this.isSysOut = isSysOut;
    }

    /**
     * Output the characters of a string (converting
     * each character to a byte discarding any high bits)
     * @param s String to write
     */
    public void write(String s) throws IOException
    {
        int len = s.length();
        for (int idx = 0; idx < len; idx++)
            write(s.charAt(idx));
    }

    /**
     * Flushes this output stream and forces any buffered output bytes
     * to be written out. The general contract of <code>flush</code> is
     * that calling it is an indication that, if any bytes previously
     * written have been buffered by the implementation of the output
     * stream, such bytes should immediately be written to their
     * intended destination.
     * <p>
     * The <code>flush</code> method of <code>OutputStream</code> does nothing.
     *
     * @exception  IOException  if an I/O error occurs.
     */
	@Override
    public native void flush() throws IOException;


    /**
     * Output a single byte.
     * @param b byte to write
     */
	@Override
    public native void write(int oneByte) throws IOException;

}
