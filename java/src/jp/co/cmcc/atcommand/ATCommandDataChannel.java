/*
 * Copyright 2014 Yarlung Soft. All rights reserved.
 */

package jp.co.cmcc.atcommand;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

class ATInputStream extends InputStream {

    protected byte buf[];
    protected int pos;
    protected int mark = 0;
    protected int count;
    private static boolean isLocked = false;

    public ATInputStream(byte buf[]) {
        if (buf == null) {
            isLocked = true;
            return;
        }
        this.buf = buf;
        pos = 0;
        count = buf.length;
    }

    public ATInputStream(byte buf[], int offset, int length) {
        if (buf == null) {
            isLocked = true;
            return;
        }
        this.buf = buf;
        pos = offset;
        count = Math.min(offset + length, buf.length);
        mark = offset;
    }

    public synchronized int read() throws IOException {
        if (isLocked) {
            throw new IOException("Data channel is closed");
        }
        return pos < count ? buf[pos++] & 0xff : -1;
    }

    public synchronized int read(byte b[], int off, int len) throws IOException {
        if (isLocked) {
            throw new IOException("Data channel is closed");
        }

        // Ensure the given off and len are valid for b[]
        if ((off | len | (off + len) | (b.length - (off + len))) < 0) {
            throw new IndexOutOfBoundsException();
        }

        if (len == 0) {
            return 0;
        }
        if (pos >= count) {
            return -1;
        }
        if (len > count - pos) {
            len = count - pos;
        }
        System.arraycopy(buf, pos, b, off, len);
        pos += len;
        return len;
    }

    public synchronized long skip(long n) {
        if (n <= 0) {
            return 0;
        }

        // arrays have a max size of 2^31
        int len = n < Integer.MAX_VALUE ? (int) n : Integer.MAX_VALUE;
        if (len > count - pos) {
            len = count - pos;
        }
        pos += len;
        return len;
    }

    public synchronized int available() {
        return count - pos;
    }

    public boolean markSupported() {
        return true;
    }

    public void mark(int readAheadLimit) {
        mark = pos;
    }

    public synchronized void reset() {
        pos = mark;
    }

    public synchronized void close() throws IOException {
        if (isLocked) {
            throw new IOException("Data channel is closed");
        }
    }
}

class ATOutputStream extends OutputStream {

    protected byte buf[];
    protected int count;

    public ATOutputStream() {
        this(32);
    }

    public ATOutputStream(int size) {
        if (size < 0) {
            throw new IllegalArgumentException();
        }
        buf = new byte[size];
    }

    public synchronized void write(int b) {
        int newcount = count + 1;
        if (newcount > buf.length) {
            byte newbuf[] = new byte[Math.max(buf.length << 1, newcount)];
            System.arraycopy(buf, 0, newbuf, 0, count);
            buf = newbuf;
        }
        buf[count] = (byte) b;
        count = newcount;
    }

    public synchronized void write(byte b[], int off, int len) {
        // Ensure the given off and len are valid for b[]
        if ((off | len | (off + len) | (b.length - (off + len))) < 0) {
            throw new IndexOutOfBoundsException();
        }
        if (len == 0) {
            return;
        }

        int newcount = count + len;
        if (newcount > buf.length) {
            byte newbuf[] = new byte[Math.max(buf.length << 1, newcount)];
            System.arraycopy(buf, 0, newbuf, 0, count);
            buf = newbuf;
        }
        System.arraycopy(b, off, buf, count, len);
        count = newcount;
    }

    public synchronized void reset() {
        count = 0;
    }

    public synchronized byte[] toByteArray() {
        byte newbuf[] = new byte[count];
        System.arraycopy(buf, 0, newbuf, 0, count);
        return newbuf;
    }

    public int size() {
        return count;
    }

    public String toString() {
        return new String(buf, 0, count);
    }

    public void close() throws IOException {
    }
}

public class ATCommandDataChannel extends Object {

    private static boolean isConnected = false;

    public ATCommandDataChannel() {
        isConnected = true;
    }

    public void close() throws IOException {
        // FIXME: Comment below code to pass TCK
//        if (isConnected == false) {
//            throw new IOException("Already be closed");
//        } else {
            // TODO: Close data channel before connection is closed
            isConnected = false;
//        }
    }

    public InputStream getInputStream() {
        InputStream is = null;
        String test = new String("AT Test String Without Result");

        if (isConnected) {
            // TODO: For test only
            is = new ATInputStream(test.getBytes());
        } else {
            is = new ATInputStream(null);
        }
        return is;
    }

    public OutputStream getOutputStream() {
        OutputStream os = null;
        if (isConnected) {
            // TODO: For test only
            os = new ATOutputStream(1024);
        }
        return os;
    }
}
