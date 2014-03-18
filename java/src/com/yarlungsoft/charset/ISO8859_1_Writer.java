package com.yarlungsoft.charset;

import java.io.*;

class ISO8859_1_Writer extends StreamWriter
{
    /**
     * Write a single character.
     *
     * @exception  IOException  If an I/O error occurs
     */
    synchronized public void write(int c) throws IOException
    {
        // Writer.write(I) ignores the top 16 bits
        c &= 0xffff;

        if (c > 255)
            c = '?'; // was throw new RuntimeException("Unknown character "+c);

        out.write(c);
    }

    /** Buffer to convert into to speedup block writing. */
    private byte[] localBuf;

    /**
     * Write a portion of an array of characters.
     *
     * @param  cbuf  Buffer of characters to be written
     * @param  off   Offset from which to start reading characters
     * @param  len   Number of characters to be written
     *
     * @exception  IOException  If an I/O error occurs
     */
    synchronized public void write(char cbuf[], int off, int len)
        throws IOException
    {
        if (localBuf == null) {
            localBuf = new byte[128];
        }
        while (len > 0)
        {
            int bufLen = localBuf.length;
            if (bufLen > len) bufLen = len;
            for (int i = 0; i < bufLen; i++)
            {
                char ch = cbuf[off + i];
                localBuf[i] = ((ch > 255) ? (byte)'?' : (byte)ch);
            }
            out.write(localBuf, 0, bufLen);
            off += bufLen;
            len -= bufLen;
        }
    }

    /**
     * Write a portion of a string.
     *
     * @param  str  String to be written
     * @param  off  Offset from which to start reading characters
     * @param  len  Number of characters to be written
     *
     * @exception  IOException  If an I/O error occurs
     */
    synchronized public void write(String str, int off, int len)
        throws IOException
    {
        // Reuse len as the end of the range to write (bytecode more compact)
        for(len += off; off < len; off++)
        {
            int ch = str.charAt(off);
            out.write((ch > 255) ? '?' : ch);
        }
    }

    /**
     * Get the size in bytes of an array of chars
     */
    public int sizeOf(char[] array, int offset, int length)
    {
        return length;
    }
}