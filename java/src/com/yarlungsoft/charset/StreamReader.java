package com.yarlungsoft.charset;

import java.io.*;

public abstract class StreamReader extends Reader {

    /** Input stream to read from */
    public InputStream in;

    /**
     * Open the reader
     */
    public Reader open(InputStream in, String enc) throws UnsupportedEncodingException {
        this.in = in;
        return this;
    }

    /**
     * Tell whether the underlying byte stream is ready to be read.
     * @return {@code false} for those streams that do not support available(), such as the Win32 console stream.
     */
    public boolean ready() {
        try {
            return in.available() > 0;
        } catch (IOException x) {
            return false;
        }
    }

    /**
     * Tell whether this stream supports the mark() operation.
     */
    public boolean markSupported() {
        return in.markSupported();
    }

    /**
     * Mark the present position in the stream.
     *
     * @throws IOException If an I/O error occurs
     */
    public void mark(int readAheadLimit) throws IOException {
        if (in.markSupported()) {
            in.mark(readAheadLimit);
        } else {
            throw new IOException("mark() not supported");
        }
    }

    /**
     * Reset the stream.
     *
     * @throws IOException If an I/O error occurs
     */
    public void reset() throws IOException {
        in.reset();
    }

    /**
     * Close the stream.
     *
     * @throws IOException If an I/O error occurs
     */
    public void close() throws IOException {
        in.close();
        in = null;
    }

    /**
     * Get the size in chars of an array of bytes
     */
    public abstract int sizeOf(byte[] array, int offset, int length);
}
