package com.yarlungsoft.util;

import java.io.IOException;
import java.io.InputStream;

/**
 * Simple InputStream class that is used for System.in.
 * It uses asynchronous IO native methods to do blocking reads.
 */
public class SystemInputStream extends InputStream
{
    /**
     * Read a single byte of standard input.
     * Block until we get one.
     */
    public int read() throws IOException
    {
        byte[] ch = new byte[1];
        read(ch, 0, 1);
        return ch[0] & 0xff;
    }

    /**
     * Read multiple bytes of standard input.
     * Expect characters to arrive intermittently, and therefore the
     * normal case is to return less than requested.
     * Block until we have one or more characters, but try to return
     * as many as possible, up to the requested limit.
     * Synchronize to protect native code against being reentered.
     */
    public synchronized int read(byte[] b, int off, int len) throws IOException
    {
		// Strict preamble from JLS general contract for InputStream.read(...).

        if ((off | len | (off + len) | (b.length - (off + len))) < 0) {
			throw new ArrayIndexOutOfBoundsException();
		}
        if (len == 0) return 0;

        int count = 0;
//        while (AsyncIO.loop()) {
        	//how to control asyncIO?
            count = readN(b, off, len);
//        }
        return count;
    }

    /**
     * Read multiple characters (blocking, but allow smaller read).
     * Throw EOFException if input stream is at the end.
     */
    public native int readN(byte[] b, int off, int len) throws IOException;

}