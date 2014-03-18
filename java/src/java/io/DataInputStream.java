/*
 *  Licensed to the Apache Software Foundation (ASF) under one or more
 *  contributor license agreements.  See the NOTICE file distributed with
 *  this work for additional information regarding copyright ownership.
 *  The ASF licenses this file to You under the Apache License, Version 2.0
 *  (the "License"); you may not use this file except in compliance with
 *  the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

package java.io;


/**
 * Wraps an existing {@link InputStream} and reads big-endian typed data from it.
 * Typically, this stream has been written by a DataOutputStream. Types that can
 * be read include byte, 16-bit short, 32-bit int, 32-bit float, 64-bit long,
 * 64-bit double, byte strings, and strings encoded in
 * {@link DataInput modified UTF-8}.
 *
 * @see DataOutputStream
 */
public class DataInputStream extends FilterInputStream implements DataInput {

    /**
     * Constructs a new DataInputStream on the InputStream {@code in}. All
     * reads are then filtered through this stream. Note that data read by this
     * stream is not in a human readable format and was most likely created by a
     * DataOutputStream.
     *
     * <p><strong>Warning:</strong> passing a null source creates an invalid
     * {@code DataInputStream}. All operations on such a stream will fail.
     *
     * @param in
     *            the source InputStream the filter reads from.
     * @see DataOutputStream
     * @see RandomAccessFile
     */
    public DataInputStream(InputStream in) {
        super(in);
    }

    /**
     * Returns the number of bytes that can be read from this input
     * stream without blocking.
     * <p>
     * This method
     * simply performs <code>in.available(n)</code> and
     * returns the result.
     *
     * @return     the number of bytes that can be read from the input stream
     *             without blocking.
     * @exception  IOException  if an I/O error occurs.
     */
    public int available() throws IOException {
        return in.available();
    }


    /**
     * Closes this input stream and releases any system resources
     * associated with the stream.
     * This
     * method simply performs <code>in.close()</code>.
     *
     * @exception  IOException  if an I/O error occurs.
     */
    public void close() throws IOException {
        in.close();
    }
    

    /**
     * Marks the current position in this input stream. A subsequent
     * call to the <code>reset</code> method repositions this stream at
     * the last marked position so that subsequent reads re-read the same bytes.
     * <p>
     * The <code>readlimit</code> argument tells this input stream to
     * allow that many bytes to be read before the mark position gets
     * invalidated.
     * <p>
     * This method simply performs <code>in.mark(readlimit)</code>.
     *
     * @param   readlimit   the maximum limit of bytes that can be read before
     *                      the mark position becomes invalid.
     */
    public synchronized void mark(int readlimit) {
        in.mark(readlimit);
    }
    
    /**
     * Tests if this input stream supports the <code>mark</code>
     * and <code>reset</code> methods.
     * This method
     * simply performs <code>in.markSupported()</code>.
     *
     * @return  <code>true</code> if this stream type supports the
     *          <code>mark</code> and <code>reset</code> method;
     *          <code>false</code> otherwise.
     */
    public boolean markSupported() {
        return in.markSupported();
    }
    
    /**
     * Repositions this stream to the position at the time the
     * <code>mark</code> method was last called on this input stream.
     * <p>
     * This method
     * simply performs <code>in.reset()</code>.
     * <p>
     * Stream marks are intended to be used in
     * situations where you need to read ahead a little to see what's in
     * the stream. Often this is most easily done by invoking some
     * general parser. If the stream is of the type handled by the
     * parse, it just chugs along happily. If the stream is not of
     * that type, the parser should toss an exception when it fails.
     * If this happens within readlimit bytes, it allows the outer
     * code to reset the stream and try another parser.
     *
     * @exception  IOException  if the stream has not been marked or if the
     *             mark has been invalidated.
     */
    public synchronized void reset() throws IOException {
        in.reset();
    }

    
    // overridden to add 'final'
    @Override 
    public final int read(byte[] buffer) throws IOException {
        return super.read(buffer);
    }

    /**
     * Reads at most {@code length} bytes from this stream and stores them in
     * the byte array {@code buffer} starting at {@code offset}. Returns the
     * number of bytes that have been read or -1 if no bytes have been read and
     * the end of the stream has been reached.
     *
     * @param buffer
     *            the byte array in which to store the bytes read.
     * @param offset
     *            the initial position in {@code buffer} to store the bytes
     *            read from this stream.
     * @param length
     *            the maximum number of bytes to store in {@code buffer}.
     * @return the number of bytes that have been read or -1 if the end of the
     *         stream has been reached.
     * @throws IOException
     *             if a problem occurs while reading from this stream.
     * @see DataOutput#write(byte[])
     * @see DataOutput#write(byte[], int, int)
     */
    @Override
    public final int read(byte[] buffer, int offset, int length) throws IOException {
        return in.read(buffer, offset, length);
    }

    public final boolean readBoolean() throws IOException {
        int temp = in.read();
        if (temp < 0) {
            throw new EOFException();
        }
        return temp != 0;
    }

    public final byte readByte() throws IOException {
        int temp = in.read();
        if (temp < 0) {
            throw new EOFException();
        }
        return (byte) temp;
    }

    public final char readChar() throws IOException {
        return (char) readShort();
    }

    public final double readDouble() throws IOException {
        return Double.longBitsToDouble(readLong());
    }

    public final float readFloat() throws IOException {
        return Float.intBitsToFloat(readInt());
    }

    public final void readFully(byte[] dst) throws IOException {
        readFully(dst, 0, dst.length);
    }

    public final void readFully(byte[] dst, int offset, int byteCount) throws IOException {
        if (byteCount < 0) {
            throw new IndexOutOfBoundsException();
        }
        int n = 0;
        while (n < byteCount) {
            int count = read(dst, offset + n, byteCount - n);
            if (count < 0) {
                throw new EOFException();
            }
            n += count;
        }
    }

    public final int readInt() throws IOException {
        int ch1 = read();
        int ch2 = read();
        int ch3 = read();
        int ch4 = read();
        if ((ch1 | ch2 | ch3 | ch4) < 0) {
             throw new EOFException();
        }
        return ((ch1 << 24) + (ch2 << 16) + (ch3 << 8) + ch4);
    }

    public final long readLong() throws IOException {
    	return ((long)(readInt()) << 32) + (readInt() & 0xFFFFFFFFL);
    }

    public final short readShort() throws IOException {
        return (short)readUnsignedShort();
    }

    public final int readUnsignedByte() throws IOException {
        int temp = in.read();
        if (temp < 0) {
            throw new EOFException();
        }
        return temp;
    }

    public final int readUnsignedShort() throws IOException {
        int ch1 = read();
        int ch2 = read();
        if ((ch1 | ch2) < 0) {
             throw new EOFException();
        }
        return (ch1 << 8) + ch2;
    }

    public final String readUTF() throws IOException {
    	return readUTF(this);
    }

    public static final String readUTF(DataInput in) throws IOException {
        int utflen = in.readUnsignedShort();
        StringBuffer str = new StringBuffer(utflen);
        byte bytearr [] = new byte[utflen];
        int c, char2, char3;
        int count = 0;

        in.readFully(bytearr, 0, utflen);

        while (count < utflen) {
            c = (int) bytearr[count] & 0xff;
            switch (c >> 4) {
                case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
                    /* 0xxxxxxx*/
                    count++;
                    str.append((char)c);
                    break;
                case 12: case 13:
                    /* 110x xxxx   10xx xxxx*/
                    count += 2;
                    if (count > utflen)
                        throw new UTFDataFormatException();
                    char2 = (int) bytearr[count-1];
                    if ((char2 & 0xC0) != 0x80)
                        throw new UTFDataFormatException();
                    str.append((char)(((c & 0x1F) << 6) | (char2 & 0x3F)));
                    break;
                case 14:
                    /* 1110 xxxx  10xx xxxx  10xx xxxx */
                    count += 3;
                    if (count > utflen)
                        throw new UTFDataFormatException();
                    char2 = (int) bytearr[count-2];
                    char3 = (int) bytearr[count-1];
                    if (((char2 & 0xC0) != 0x80) || ((char3 & 0xC0) != 0x80))
                        throw new UTFDataFormatException();
                    str.append((char)(((c     & 0x0F) << 12) |
                                      ((char2 & 0x3F) << 6)  |
                                      ((char3 & 0x3F)     )));
                    break;
                default:
                    /* 10xx xxxx,  1111 xxxx */
                    throw new UTFDataFormatException();
                }
        }
        // The number of chars produced may be less than utflen
        return new String(str);
    }

    /**
     * Skips over and discards <code>n</code> bytes of data from the
     * input stream. The <code>skip</code> method may, for a variety of
     * reasons, end up skipping over some smaller number of bytes,
     * possibly <code>0</code>. The actual number of bytes skipped is
     * returned.
     * <p>
     * This method
     * simply performs <code>in.skip(n)</code>.
     *
     * @param      n   the number of bytes to be skipped.
     * @return     the actual number of bytes skipped.
     * @exception  IOException  if an I/O error occurs.
     */
    public long skip(long n) throws IOException {
        return in.skip(n);
    }

    /**
     * Skips {@code count} number of bytes in this stream. Subsequent {@code
     * read()}s will not return these bytes unless {@code reset()} is used.
     *
     * This method will not throw an {@link EOFException} if the end of the
     * input is reached before {@code count} bytes where skipped.
     *
     * @param count
     *            the number of bytes to skip.
     * @return the number of bytes actually skipped.
     * @throws IOException
     *             if a problem occurs during skipping.
     * @see #mark(int)
     * @see #reset()
     */
    public final int skipBytes(int count) throws IOException {
        int skipped = 0;
        long skip;
        while (skipped < count && (skip = in.skip(count - skipped)) != 0) {
            skipped += skip;
        }
        return skipped;
    }
}
