package com.yarlungsoft.charset;

import java.io.*;

public abstract class StreamWriter extends Writer {

    /** Output stream to write to */
    public OutputStream out;

    /**
     * Open the writer
     */
    public Writer open(OutputStream out, String enc)
        throws UnsupportedEncodingException {

        this.out = out;
        return this;
    }

    /**
     * Flush the writer and the output stream.
     *
     * @exception  IOException  If an I/O error occurs
     */
    public void flush() throws IOException {
        out.flush();   
    }

    /**
     * Close the writer and the output stream.
     *
     * @exception  IOException  If an I/O error occurs
     */
    public void close() throws IOException {
        out.close();      
    }

    /**
     * Get the size in bytes of an array of chars
     */
    public abstract int sizeOf(char[] array, int offset, int length);

}