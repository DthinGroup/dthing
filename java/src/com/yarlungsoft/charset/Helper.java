package com.yarlungsoft.charset;

import java.io.*;

public class Helper {

    /**
     * The name of the default character encoding
     */
    private static final String DEFAULT_ENCODING = "ISO8859_1";

    /**
     * Package of the charset classes
     */
    private static final String CHARSET_PACKAGE = "com.yarlungsoft.charset";

    /**
     * Built-in encoding name table.
     * <p/>
     * Maps historical and canonical encoding names to canonical encoding names and internal names
     * used to find the reader/writer classes.
     * <p/>
     * Other mappings can be added using the properties:
     * <ul>
     * <li>&lt;normalized name&gt;_InternalEncodingName</li>
     * <li>&lt;normalized name&gt;_CanonicalEncodingName</li>
     * </ul>
     * If no match is found then the normalized name is used for both the canonical and internal
     * name.
     * <p/>
     * When matching names, ignore the case. Preserve the case in the returned canonical and
     * internal names.
     */
    private static final String ENCODING_MAPS[] = {
        // Normalized historical name   Canonical name  Internal name
        "ISO_8859_1",                   "ISO8859_1",    "ISO8859_1",
        "8859_1",                       "ISO8859_1",    "ISO8859_1",
        "UTF_8",                        "UTF8",         "UTF_8",

        // US-ASCII is subclass of ISO-8859-1 so we do not need a separate reader for it.
        "US_ASCII",                     "ASCII",        "ISO8859_1"
    };

    /*---------------------------------------------------------------------------*/
    /* Character encoding */
    /*---------------------------------------------------------------------------*/

    /**
     * Return an appropriate reader/writer for the given encoding.
     *
     * @param encoding Encoding name.
     * @param forReader {@code true} for reader class, while {@code false} for writer class
     * @return reader/writer for the given encoding.
     * @throws NullPointerException if {@code encoding} is {@code null}.
     * @throws UnsupportedEncodingException if the encoding can't be found.
     * @throws RuntimeException for other problems with the encoding.
     */
    private static Object getEncodingClass(String encoding, boolean forReader)
            throws UnsupportedEncodingException, RuntimeException {
        // Get the reader/writer class name
        String suffix = forReader ? "_Reader" : "_Writer";
        String className = CHARSET_PACKAGE + '.' + internalNameForEncoding(encoding) + suffix;

        try {
            Class<?> clazz = Class.forName(className);
            return clazz.newInstance();
        } catch (ClassNotFoundException x) {
            throw new UnsupportedEncodingException("Encoding " + encoding + " not found");
        } catch (Throwable t) {
            throw new RuntimeException(t.getClass().getName() + ' ' + t.getMessage());
        }
    }

    /**
     * Get a reader of default encoding ISO8859_1 for an {@link InputStream}
     *
     * @param is The input stream for the reader
     * @return A new reader for the stream
     * @throws NullPointerException if {@code is} is {@code null}.
     * @throws RuntimeException for other problems with the encoding.
     */
    public static Reader getStreamReader(InputStream is) throws NullPointerException,
            RuntimeException {
        try {
            return getStreamReader(is, DEFAULT_ENCODING);
        } catch (UnsupportedEncodingException e) {
            System.out.println("Unexpected exception when create stream reader of default encoding: "
                               + e);
            return null;
        }
    }

    /**
     * Get a reader of the specified encoding for an {@link InputStream}
     *
     * @param is The input stream for the reader
     * @param encoding Encoding name
     * @return A new reader for the stream
     * @throws NullPointerException if {@code is} or {@code encoding} is {@code null}.
     * @throws UnsupportedEncodingException if the encoding can't be found.
     * @throws RuntimeException for other problems with the encoding.
     */
    public static Reader getStreamReader(InputStream is, String encoding)
            throws NullPointerException, UnsupportedEncodingException, RuntimeException {
        if (is == null) {
            throw new NullPointerException();
        }

        /* Get the reader from the encoding */
        StreamReader reader = (StreamReader) getEncodingClass(encoding, true);

        /* Open the connection and return */
        return reader.open(is, encoding);
    }

    /**
     * Get a writer of default encoding ISO8859_1 for an {@link OutputStream}
     *
     * @param os The output stream for the writer
     * @return A new writer for the stream
     * @throws NullPointerException if {@code os} is {@code null}.
     * @throws RuntimeException for other problems with the encoding.
     */
    public static Writer getStreamWriter(OutputStream os) throws NullPointerException,
            RuntimeException {
        try {
            return getStreamWriter(os, DEFAULT_ENCODING);
        } catch (UnsupportedEncodingException e) {
            System.out.println("Unexpected exception when create stream writer of default encoding: "
                    + e);
            return null;
        }
    }

    /**
     * Get a writer of the specified encoding for an {@link OutputStream}
     *
     * @param os The output stream for the writer
     * @param encoding Encoding name
     * @return A new writer for the stream
     * @throws NullPointerException if {@code os} or {@code encoding} is {@code null}.
     * @throws UnsupportedEncodingException if the encoding can't be found.
     * @throws RuntimeException for other problems with the encoding.
     */
    public static Writer getStreamWriter(OutputStream os, String encoding)
            throws NullPointerException, UnsupportedEncodingException, RuntimeException {
        if (os == null) {
            throw new NullPointerException();
        }

        /* Get the writer from the encoding */
        StreamWriter writer = (StreamWriter) getEncodingClass(encoding, false);

        /* Open it on the output stream and return */
        return writer.open(os, encoding);
    }

    /**
     * Convert a byte array to a char array using the default encoding (ISO8859_1).
     *
     * @param buffer The byte array buffer
     * @param offset The offset
     * @param length The length
     * @return A new char array
     * @throws NullPointerException if {@code buffer} is {@code null}.
     * @throws IndexOutOfBoundsException If buffer, offset, or length not valid.
     */
    public static char[] byteToCharArray(byte[] buffer, int offset, int length)
        throws IndexOutOfBoundsException {
        /*
         * Rely on implicit exception checks to validate the incoming buffer, offset and length
         * arguments.
         * <ul>
         * <li>a) length < 0 causes a NegativeArraySizeException</li>
         * <li>b) offset thru offset+length-1 outside of buffer causes an
         * ArrayIndexOutOfBoundsException</li>
         * </ul>
         * However, we need to avoid creating the character array if length is way too big, as this
         * may cause an OutOfMemoryException instead (before hitting condition b). So add an
         * explicit, albeit somewhat inaccurate check.
         */
        if (length > buffer.length) {
            length = -1; // force an exception
        }

        try {
            char[] carray = new char[length];
            for (int i = length; --i >= 0;) {
                carray[i] = (char) (buffer[offset + i] & 0xff);
            }
            return carray;
        } catch (NegativeArraySizeException ex1) {
        } catch (ArrayIndexOutOfBoundsException ex2) {
        }

        // Callers are expecting an IndexOutOfBoundsException instead.
        throw new IndexOutOfBoundsException();
    }

    /**
     * Convert a char array to a byte array using the default encoding (ISO8859_1).
     *
     * @param buffer The char array buffer
     * @param offset The offset
     * @param length The length
     * @return A new byte array
     * @throws NullPointerException if {@code buffer} is {@code null}.
     * @throws IndexOutOfBoundsException If buffer, offset, or length not valid.
     */
    public static byte[] charToByteArray(char[] buffer, int offset, int length)
            throws IndexOutOfBoundsException {
        /*
         * Rely on implicit exception checks to validate the incoming buffer, offset and length
         * arguments.
         * <ul>
         * <li>a) length < 0 causes a NegativeArraySizeException</li>
         * <li>b) offset thru offset+length-1 outside of buffer causes an
         * ArrayIndexOutOfBoundsException</li>
         * </ul>
         * However, we need to avoid creating the character array if length is way too big, as this
         * may cause an OutOfMemoryException instead (before hitting condition b). So add an
         * explicit, albeit somewhat inaccurate check.
         */
        if (length > buffer.length) {
            length = -1; // force an exception
        }

        try {
            byte[] barray = new byte[length];
            for (int i = length; --i >= 0;) {
                int ch = buffer[offset + i];
                barray[i] = (byte) ((ch > 255) ? '?' : ch);
            }
            return barray;
        } catch (NegativeArraySizeException ex1) {
        } catch (ArrayIndexOutOfBoundsException ex2) {
        }

        // Callers are expecting an IndexOutOfBoundsException instead.
        throw new IndexOutOfBoundsException();
    }

    /**
     * Cached variables for byteToCharArray
     */
    private static String sLastReaderEncoding;
    private static StreamReader sLastReader;

    /**
     * Convert a byte array to a char array.
     * FIXME other VMs "recover" from corrupt encoding bytes and continue processing. Our
     * implementation just gives up early. Would need to fix all the _Reader classes.
     *
     * @param buffer The byte array buffer
     * @param offset The offset
     * @param length The length
     * @param encoding The character encoding
     * @return A new char array
     * @throws NullPointerException if {@code buffer} or {@code encoding} is {@code null}.
     * @throws IndexOutOfBoundsException If buffer, offset, or length not valid.
     * @throws UnsupportedEncodingException If the encoding is not known.
     */
    public static synchronized char[] byteToCharArray(byte[] buffer, int offset, int length, String encoding)
            throws IndexOutOfBoundsException, UnsupportedEncodingException {
        // Ensure the given off and len are valid for buffer[]
        if ((offset | length | (offset + length) | (buffer.length - (offset + length))) < 0) {
            throw new IndexOutOfBoundsException();
        }

        encoding = internalNameForEncoding(encoding);

        /* If we don't have a cached reader then make one */
        if (sLastReaderEncoding == null || !sLastReaderEncoding.equals(encoding)) {
            sLastReader = (StreamReader) getEncodingClass(encoding, true);
            sLastReaderEncoding = encoding;
        }
        // Now sLastReaderEncoding is the correct StreamReader for encoding

        /* Ask the stream for the size the output will be */
        int size = sLastReader.sizeOf(buffer, offset, length);

        /* Allocate a buffer of that size */
        char[] outbuf = new char[size];

        /* Open the reader on a ByteArrayInputStream */
        sLastReader.open(new ByteArrayInputStream(buffer, offset, length), encoding);

        try {
            /* Read the input */
            sLastReader.read(outbuf, 0, size);
            /* Close the reader */
            sLastReader.close();
        } catch (IOException x) {
            // Behavior undefined: just give up and return what we have
        }

        /* And return the buffer */
        return outbuf;
    }

    /**
     * Cached variables for charToByteArray
     */
    private static String sLastWriterEncoding;
    private static StreamWriter sLastWriter;

    /**
     * Convert a char array to a byte array.
     * FIXME other VMs "recover" from corrupt encoding bytes and continue processing. Our
     * implementation just gives up early. Would need to fix all the _Writer classes.
     *
     * @param buffer The char array buffer
     * @param offset The offset
     * @param length The length
     * @param encoding The character encoding
     * @return A new byte array
     * @throws NullPointerException if {@code buffer} or {@code encoding} is {@code null}.
     * @throws IndexOutOfBoundsException If buffer, offset, or length not valid.
     * @throws UnsupportedEncodingException If the encoding is not known.
     */
    public static synchronized byte[] charToByteArray(char[] buffer, int offset, int length, String encoding)
            throws IndexOutOfBoundsException, UnsupportedEncodingException {
        // Ensure the given off and len are valid for buf[]
        if ((offset | length | (offset + length) | (buffer.length - (offset + length))) < 0) {
            throw new IndexOutOfBoundsException();
        }

        encoding = internalNameForEncoding(encoding);

        /* If we don't have a cached writer then make one */
        if (sLastWriterEncoding == null || !sLastWriterEncoding.equals(encoding)) {
            sLastWriter = (StreamWriter) getEncodingClass(encoding, false);
            sLastWriterEncoding = encoding;
        }

        /* Ask the writer for the size the output will be */
        int size = sLastWriter.sizeOf(buffer, offset, length);

        /* Get the output stream */
        ByteArrayOutputStream os = new ByteArrayOutputStream(size);

        /* Open the writer */
        sLastWriter.open(os, encoding);

        try {
            /* Convert */
            sLastWriter.write(buffer, offset, length);
            /* Close the writer */
            sLastWriter.close();
        } catch (IOException x) {
            // Behavior undefined: just give up and return what we have
        }

        /* Return the array */
        return os.toByteArray();
    }

    /**
     * Get the internal name for an encoding.
     *
     * @param encoding encoding name
     * @return internal name for this encoding
     * @throws NullPointerException if specified {@code encoding} is {@code null}.
     */
    private static String internalNameForEncoding(String encoding) {
        // Optimize for the (very common) default encoding case
        if (encoding.equals(DEFAULT_ENCODING)) {
            return DEFAULT_ENCODING;
        }

        encoding = normalizeEncodingName(encoding);

        // Check the built-in encoding map
        for (int i = ENCODING_MAPS.length - 3; i >= 0; i -= 3) {
            // Compare the historical name and canonical names
            if (encoding.equals(ENCODING_MAPS[i])
                || matchCanonicalName(encoding, ENCODING_MAPS[i + 1])) {
                // Return the internal encoding name
                return ENCODING_MAPS[i + 2];
            }
        }

        /*
         * Some IANA character encoding names start with digits, which cannot be used as Java reader
         * class name. We provide a way to configure alternate names for those encodings.
         * <p/>
         * Note: The encoding names must be normalized, i.e. only contains upper case characters,
         * digits and '_' while only begins with upper case character.
         */
        String property = System.getProperty(encoding + "_InternalEncodingName");
        if (property != null) {
            return property;
        }

        return encoding;
    }

    /**
     * Get the canonical name for an encoding.
     *
     * @param encoding encoding name or {@code null} for the default encoding
     * @return canonical name for this encoding
     */
    public static String canonicalEncodingName(String encoding) {
        if (encoding == null) {
            encoding = DEFAULT_ENCODING;
        }

        encoding = normalizeEncodingName(encoding);

        // Check the built-in encoding map
        for (int i = ENCODING_MAPS.length - 3; i >= 0; i -= 3) {
            // Compare with the historical and canonical names
            if (encoding.equals(ENCODING_MAPS[i])
                || matchCanonicalName(encoding, ENCODING_MAPS[i + 1])) {
                // Return the canonical encoding name
                return ENCODING_MAPS[i + 1];
            }
        }

        /*
         * Some IANA character encoding names start with digits, which cannot be used as Java reader
         * class name. We provide a way to configure alternate names for those encodings.
         * <p/>
         * Note: The encoding names must be normalized, i.e. only contains upper case characters,
         * digits and '_' while only begins with upper case character.
         */
        String property = System.getProperty(encoding + "_CanonicalEncodingName");
        if (property != null) {
            return property;
        }

        return encoding;
    }

    /**
     * Converts "-" and ":" in a string to "_" and converts the name to upper case.
     * <p/>
     * This is needed because the names of IANA character encodings have characters that are not
     * allowed for java class names and IANA encoding names are not case sensitive.
     *
     * @param encodingName encoding name
     * @return normalized name
     * @throws NullPointerException if {@code encodingName} is {@code null}.
     */
    private static String normalizeEncodingName(String encodingName) {
        StringBuffer normalizedName = new StringBuffer(encodingName);

        int length = normalizedName.length();
        for (int i = 0; i < length; i++) {
            char currentChar = normalizedName.charAt(i);

            if (currentChar == '-' || currentChar == ':') {
                normalizedName.setCharAt(i, '_');
            } else {
                normalizedName.setCharAt(i, Character.toUpperCase(currentChar));
            }
        }

        return normalizedName.toString();
    }

    /**
     * Compares a normalized encoding name with a canonical encoding name.
     * <p/>
     * Characters in the canonical name are converted as if normalizeEncodingName() were called.
     *
     * @param normalizedName internal encoding name returned by normalizeEncodingName()
     * @param canonicalName canonical encoding name
     * @return {@code true} if the names match, {@code false} otherwise
     */
    private static boolean matchCanonicalName(String normalizedName, String canonicalName) {
        canonicalName = normalizeEncodingName(canonicalName);
        return normalizedName.equals(canonicalName);
    }
}
