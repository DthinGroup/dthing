package com.yarlungsoft.charset;

import java.io.*;

class ISO8859_1_Reader extends StreamReader {

    /**
     * Read a single character.
     *
     * @exception  IOException  If an I/O error occurs
     */
    synchronized public int read() throws IOException {
        return in.read();
    }

    /** Size of the buffer used for reading bytes to convert */
    private static final int BUFSIZE = 1;//128;

    /** A buffer used by read() to speed up conversion of large reads */
    private byte[] buffer = new byte[BUFSIZE];

    /**
     * Read characters into a portion of an array.
     *
     * @exception  IOException  If an I/O error occurs
     */
    synchronized public int read(char cbuf[], int off, int len)
        throws IOException {

        int totalRead = 0;

        while(len > 0 && (totalRead == 0 || in.available() > 0)) {

            int toRead = Math.min(len, BUFSIZE);

            int count = in.read(buffer, 0, toRead);
            if (count == -1)
                break;

            for (int i = 0; i < count; i++) {
                cbuf[off] = (char)(buffer[i] & 0xff);
                off++;
            }

            totalRead += count;
            len -= count;
        }

        if (totalRead == 0 && len > 0) {
            // we failed to read anything so has to be EOF
            totalRead = -1;
        }

        return totalRead;
    }

    /**
     * Get the size in chars of an array of bytes
     */
    public int sizeOf(byte[] array, int offset, int length) {
        return length;
    }

}
