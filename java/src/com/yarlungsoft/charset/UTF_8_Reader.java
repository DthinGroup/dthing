package com.yarlungsoft.charset;

import java.io.*;

/** Reader for UTF-8 encoded input streams. */
class UTF_8_Reader extends StreamReader {

	/** Partial data for a character */
	int partChar;

	/** Number of remaining bytes needed to complete the current char */
	int pending;

	byte[] localBuf;

	/** Size of the buffer used for reading bytes to convert */
	static final int BUFSIZE = 128;

	/** Constructs a UTF-8 reader */
	public UTF_8_Reader() {
	}

	/**
	 * Read a block of UTF8 characters.
	 *
	 * @param cbuf output buffer for converted characters read
	 * @param off initial offset into the provided buffer
	 * @param len length of characters in the buffer
	 * @return the number of converted characters with
	 *			-1 indicating end of stream.
	 * @exception IOException is thrown if the input stream
	 * could not be read for the raw unconverted character
	 */
	public int read(char cbuf[], int off, int len) throws IOException
	{
		int count = 0;
		if (localBuf == null) {
			localBuf = new byte[BUFSIZE];
		}
		boolean cont_flag = true;
		while(0 < len && cont_flag)
		{
			int toRead = Math.min(len, BUFSIZE);
			int i;
			int result = in.read(localBuf, 0, toRead);
			/*to avoid blocking in network, we treat less read 
			 than we required as EOF and will never try to read more*/
			if ((result < toRead )&& (result !=0))
			{
				cont_flag = false;
			}
			if (result >= 0)
			{
				for (i =0; i <result; i ++)
				{
					int ch = localBuf[i]&0xff;
					//  7 bits: 0xxx xxxx
					//  ? bits: 10xx xxxx  INVALID
					// 11 bits: 110x xxxx  10xx xxxx
					// 16 bits: 1110 xxxx  10xx xxxx  10xx xxxx
					//  ? bits: 1111 xxxx  INVALID
					// Invalid or partial combinations must give an exception
            	
					if (pending == 0)
					{
						if ((ch & ~0x7f) == 0)	// VERY COMMON: 7-bit case?
						{
							cbuf[off + count] = (char)ch;
							count++;
							len --;
						}
						else if ((ch & 0xE0) == 0xC0) // 11-bit case?
						{
							pending = 1;
							partChar = ch & 0x1F;
						}
						else if ((ch & 0xF0) == 0xE0) // 16-bit case?
						{
							pending = 2;
							partChar = ch & 0x0F;
						}
						else
						{
							throw new UTFDataFormatException("invalid first byte " +
								Integer.toBinaryString(ch));
						}
					}
					else // Subsequent bytes
					{
						if ((ch & 0xC0) != 0x80)
						{
							throw new UTFDataFormatException("invalid byte " +
								Integer.toBinaryString(ch));
						}
	        	
						partChar = (partChar << 6) | (ch & 0x3f);
	        	
						if (--pending == 0)
						{
							cbuf[off + count] = (char)partChar;
							count++;
							len --;
						}
					}
				}
			}
			else
			{
				if (pending != 0)
					throw new UTFDataFormatException("partial character");
				if (count == 0)
					count = -1;
				break;
			}
		}
		return count;
	}

	/**
	 * Tell whether this reader supports the mark() operation.
	 * The UTF-8 implementation always returns false because it does not
	 * support mark().
	 *
	 * @return false
	 */
	public boolean markSupported() {
		/*
		 * For readers mark() is in characters, since UTF-8 character are
		 * variable length, so we can't just forward this to the underlying
		 * byte InputStream like other readers do.
		 * So this reader does not support mark at this time.
		 */
		return false;
	}

	/**
	 * Mark a read ahead character is not supported for UTF8
	 * readers.
	 * @param readAheadLimit number of characters to buffer ahead
	 * @exception IOException is thrown, for all calls to this method
	 * because marking is not supported for UTF8 readers
	 */
	public void mark(int readAheadLimit) throws IOException {
		throw new IOException("mark() not supported");
	}

	/**
	 * Reset the read ahead marks is not supported for UTF8 readers.
	 * @exception IOException is thrown, for all calls to this method
	 * because marking is not supported for UTF8 readers
	 */
	public void reset() throws IOException {
		throw new IOException("reset() not supported");
	}

	/**
	 * Get the size in chars of an array of bytes
	 *
	 * @param      array  Source buffer
	 * @param      offset Offset at which to start counting characters
	 * @param      length number of bytes to use for counting
	 *
	 * @return     number of characters that would be converted
	 */
	/*
	 * This method is only used by our internal Helper class in the method
	 * byteToCharArray to know how much to allocate before using a
	 * reader. If we encounter bad encoding we should return a count
	 * that includes that character so the reader will throw an IOException
	 */
	public int sizeOf(byte[] array, int offset, int length)
	{
		int count = 0;
		int endOfArray = offset + length;

		while(offset < endOfArray)
		{
			count++;

			int ch = array[offset];	// NB Will be sign extended

			//  7 bits: 0xxx xxxx
			//  ? bits: 10xx xxxx  INVALID
			// 11 bits: 110x xxxx  10xx xxxx
			// 16 bits: 1110 xxxx  10xx xxxx  10xx xxxx
			//  ? bits: 1111 xxxx  INVALID
			// Count must include first invalid or partial char

			if (ch >= 0) // VERY COMMON: 7-bit case (ch is sign extended byte)
			{
				offset++;
			}
			else if ((ch & 0xE0) == 0xC0) // 11-bit case?
			{
				offset += 2;
			}
			else if ((ch & 0xF0) == 0xE0) // 16-bit case?
			{
				offset += 3;
			}
			else // Invalid case
			{
				break;
			}
		}

		return count;
	}
}
