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

import com.yarlungsoft.charset.Helper;

/**
 * A class for turning a character stream into a byte stream. Data written to
 * the target input stream is converted into bytes by either a default or a
 * provided character converter. The default encoding is taken from the
 * "file.encoding" system property. {@code OutputStreamWriter} contains a buffer
 * of bytes to be written to target stream and converts these into characters as
 * needed. The buffer size is 8K.
 *
 * @see InputStreamReader
 */
public class OutputStreamWriter extends Writer {

	/**
	 * The underlying character-output stream.
	 */
	private Writer out;

	/**
	 * The character encoding. Null is the default.
	 */
	private String encoding;

    /**
     * Constructs a new OutputStreamWriter using {@code out} as the target
     * stream to write converted characters to. The default character encoding
     * is used.
     *
     * @param out
     *            the non-null target stream to write converted bytes to.
     * @throws UnsupportedEncodingException 
     */
    public OutputStreamWriter(OutputStream out) {
    	this.out = Helper.getStreamWriter(out);
    }

    /**
     * Constructs a new OutputStreamWriter using {@code out} as the target
     * stream to write converted characters to and {@code enc} as the character
     * encoding. If the encoding cannot be found, an
     * UnsupportedEncodingException error is thrown.
     *
     * @param out
     *            the target stream to write converted bytes to.
     * @param enc
     *            the string describing the desired character encoding.
     * @throws NullPointerException
     *             if {@code enc} is {@code null}.
     * @throws UnsupportedEncodingException
     *             if the encoding specified by {@code enc} cannot be found.
     */
    public OutputStreamWriter(OutputStream out, final String enc)
            throws UnsupportedEncodingException {
    	this.out = Helper.getStreamWriter(out, enc);
    	this.encoding = enc;
    }

    /** Check to make sure that the stream has not been closed */
    private void ensureOpen() throws IOException {
        if (out == null) {
            throw new IOException("Stream closed");
        }
    }
    
    /**
     * Closes this writer. This implementation flushes the buffer as well as the
     * target stream. The target stream is then closed and the resources for the
     * buffer and converter are released.
     *
     * <p>Only the first invocation of this method has any effect. Subsequent calls
     * do nothing.
     *
     * @throws IOException
     *             if an error occurs while closing this writer.
     */
    @Override
    public void close() throws IOException {
    	if (out != null) {
    	    out.close();
    	    out = null;
    	}
    }

    /**
     * Flushes this writer. This implementation ensures that all buffered bytes
     * are written to the target stream. After writing the bytes, the target
     * stream is flushed as well.
     *
     * @throws IOException
     *             if an error occurs while flushing this writer.
     */
    @Override
    public void flush() throws IOException {
    	ensureOpen();
    	out.flush();
    }

    /**
     * Returns the historical name of the encoding used by this writer to convert characters to
     * bytes, or null if this writer has been closed. Most callers should probably keep
     * track of the String or Charset they passed in; this method may not return the same
     * name.
     */
    public String getEncoding() {
    	if (out == null)
    	    return null;
    	else
    	    return Helper.canonicalEncodingName(encoding);
    }

    /**
     * Writes {@code count} characters starting at {@code offset} in {@code buf}
     * to this writer. The characters are immediately converted to bytes by the
     * character converter and stored in a local buffer. If the buffer gets full
     * as a result of the conversion, this writer is flushed.
     *
     * @param buffer
     *            the array containing characters to write.
     * @param offset
     *            the index of the first character in {@code buf} to write.
     * @param count
     *            the maximum number of characters to write.
     * @throws IndexOutOfBoundsException
     *             if {@code offset < 0} or {@code count < 0}, or if
     *             {@code offset + count} is greater than the size of
     *             {@code buf}.
     * @throws IOException
     *             if this writer has already been closed or another I/O error
     *             occurs.
     */
    @Override
    public void write(char[] buffer, int offset, int count) throws IOException {
    	ensureOpen();

    	// Ensure the given off and count are valid for buffer[]
    	if ((offset | count | (offset + count) | (buffer.length - (offset + count))) < 0)
    	    throw new IndexOutOfBoundsException();

    	if (count > 0)
    	    out.write(buffer, offset, count);
    }

    /**
     * Writes the character {@code oneChar} to this writer. The lowest two bytes
     * of the integer {@code oneChar} are immediately converted to bytes by the
     * character converter and stored in a local buffer. If the buffer gets full
     * by converting this character, this writer is flushed.
     *
     * @param oneChar
     *            the character to write.
     * @throws IOException
     *             if this writer is closed or another I/O error occurs.
     */
    @Override
    public void write(int oneChar) throws IOException {
    	ensureOpen();
    	out.write(oneChar);
    }

    /**
     * Writes {@code count} characters starting at {@code offset} in {@code str}
     * to this writer. The characters are immediately converted to bytes by the
     * character converter and stored in a local buffer. If the buffer gets full
     * as a result of the conversion, this writer is flushed.
     *
     * @param str
     *            the string containing characters to write.
     * @param offset
     *            the start position in {@code str} for retrieving characters.
     * @param count
     *            the maximum number of characters to write.
     * @throws IOException
     *             if this writer has already been closed or another I/O error
     *             occurs.
     * @throws IndexOutOfBoundsException
     *             if {@code offset < 0} or {@code count < 0}, or if
     *             {@code offset + count} is bigger than the length of
     *             {@code str}.
     */
    @Override
    public void write(String str, int offset, int count) throws IOException {
    	ensureOpen();

        // Ensure the given off and count are valid for b[]
        if ((offset | count | (offset + count) | (str.length() - (offset + count))) < 0)
        	throw new IndexOutOfBoundsException();

        if (count > 0)
        	out.write(str, offset, count);
    }
}
