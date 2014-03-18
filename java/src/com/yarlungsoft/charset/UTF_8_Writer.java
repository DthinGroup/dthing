package com.yarlungsoft.charset;

import java.io.*;

/**
 * Class for writing UTF-8 encoded output streams.
 */
class UTF_8_Writer extends StreamWriter
{
    /** Constructs a UTF-8 writer */
    public UTF_8_Writer()
    {
    }
    
    /**
     * Write a single character encoded in UTF-8. Out of bound values are
     * truncated to 16 bits. The character with a value of zero (Unicode NULL)
     * is encoded as a zero byte in the output stream. This behaviour is
     * expected by the MIDP2 TCK.
     *
     * @param c     The character to encode.
     * @exception   IOException  If an I/O error occurs
     */
    synchronized public void write(int c) throws IOException
    {
        // Truncate values.
        c &= 0xFFFF;

        if (c < 0x80)
        {
            out.write(c);
        } else {
            if (c < 0x800)
            {
                // 11 bits or fewer encoded in two bytes.
                out.write((c >> 6) | 0xC0);
            } else {
                // 12 to 16 bits are encoded in three bytes.
                out.write((c >> 12) | 0xE0);
                out.write(((c >> 6) & 0x3F) | 0x80);
            }

            // Final 6 bits are handled the same.
            out.write((c & 0x3F) | 0x80);
        }
    }

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
        int count = 0;
        int i;
        byte[] temBuf = new byte[len*4];

        for (i = 0; i < len ; i++)
        {
            char c = cbuf[off+i];
            if (c < 0x80)
            {
            // ASCII chars (including NUL) take one byte.
                temBuf[count] = (byte) c;
                count++;
            }
            else
            {
                if (c < 0x800)
                {
                // 11 bits or fewer encoded in two bytes.
                    temBuf[count] = (byte)((c >> 6) | 0xC0);
                    count++;
                } else
                {
                    // 12 to 16 bits are encoded in three bytes.
                    temBuf[count] = (byte)((c >> 12) | 0xE0);
                    count++;
                    temBuf[count] = (byte)(((c >> 6) & 0x3F) | 0x80);
                    count++;
                }
                // Final 6 bits are handled the same.
                temBuf[count] = (byte)((c & 0x3F) | 0x80);
                count++;
            }
        }
        out.write(temBuf, 0, count);
    }


    /**
     * Get the number of bytes required to encode a number of chars from an
     * array.
     *
     * @param cbuf  The array of chars.
     * @param off   The starting offset within the array.
     * @param len   The number of chars to encode.
     * @return      The number of bytes required for the encoded form.
     */
    public int sizeOf(char[] cbuf, int off, int len)
    {
        int count = 0;

        for (len += off; off < len; off++)
        {
            char c = cbuf[off];

            if (c < 0x80)
            {
                // ASCII chars (including NUL) take one byte.
                count++;
            } else {
                // Anything else takes either two bytes if it is limited to the
                // least significant 11 bits or fewer, or three bytes if more
                // than 11 bits are required.
                count += (c < 0x800) ? 2 : 3;
            }
        }

        return count;
    }
}