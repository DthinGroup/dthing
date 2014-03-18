package com.yarlungsoft.charset;

import java.io.*;

public class Helper {

    /**
     * The name of the default character encoding
     */
    private static String defaultEncoding;

    /**
     * Default path to the J2ME classes
     */
    private static String defaultMEPath;

    /**
     * Built-in encoding name table.
     *
     * Maps historical and canonical encoding names to canonical encoding
     * names and internal names used to find the reader/writer classes.
     *
     * Other mappings can be added using the properties:
     *  <normalized name>_InternalEncodingName
     *  <normalized name>_CanonicalEncodingName
     *
     * If no match is found then the normalized name is used for both the
     * canonical and internal name.
     *
     * When matching names, ignore the case. Preserve the case in the returned
     * canonical and internal names.
     */
    private final static String encodingMapping[] =
    {
        // Normalized historical name   Canonical name  Internal name
        "ISO_8859_1",                   "ISO8859_1",    "ISO8859_1",
        "8859_1",                       "ISO8859_1",    "ISO8859_1",
        "UTF_8",                        "UTF8",         "UTF_8",    

        // US-ASCII is subclass of ISO-8859-1 so we do not need a
        // separate reader for it.
        "US_ASCII",                     "ASCII",       "ISO8859_1",
    };

    /**
     * Class initializer
     */
    static {

    	defaultEncoding = "ISO8859_1";

        /*  Get the default encoding name */
        defaultMEPath = "com.yarlungsoft.charset";
    }

/*---------------------------------------------------------------------------*/
/*                               Character encoding                          */
/*---------------------------------------------------------------------------*/

    /**
     * Return an appropriate reader/writer for the given encoding.
     *
     * @param   encoding    Encoding name
     * @param   who         either "_Reader" or "_Writer"
     * @return  Returns either a writer or a reader for the given encoding.
     *
     * @exception NullPointerException if encoding is null
     * @exception UnsupportedEncodingException if the encoding can't be found.
     * @exception RuntimeException for other problems with the enconding.
     */
    private static Object getEncodingClass(String encoding, String who)
        throws UnsupportedEncodingException {

        Throwable th;

        // Rely on internalNameForEncoding() throwing a NullPointerException

        // Get the reader/writer class name

        String className = defaultMEPath + '.' +
            internalNameForEncoding(encoding) + who;

        try {
            Class<?> clazz = Class.forName(className);
            return clazz.newInstance();
        } catch(ClassNotFoundException x) {
            throw new UnsupportedEncodingException(
                "Encoding " + encoding + " not found");
        } catch(InstantiationException x) {
            th = x;
        } catch(IllegalAccessException x) {
            th = x;
        }

        throw new RuntimeException(th.getClass().getName()+' '+th.getMessage());
    }

    /**
     * Get a reader for an InputStream
     *
     * @param  is              The input stream the reader is for
     * @return                 A new reader for the stream
     */
    public static Reader getStreamReader(InputStream is) {
        try {
            return getStreamReader(is, defaultEncoding);
        } catch(UnsupportedEncodingException x) {
            throw new RuntimeException(x.getMessage());
        }
    }

    /**
     * Get a reader for an InputStream
     *
     * @param  is              The input stream the reader is for
     * @param  name            The name of the decoder
     * @return                 A new reader for the stream
     * @exception UnsupportedEncodingException  If the encoding is not known
     * @exception NullPointerException If either argument is null
     */
    public static Reader getStreamReader(InputStream is, String name)
        throws UnsupportedEncodingException {

        /* Test for null arguments */
        if (is == null)
            throw new NullPointerException();

        // Rely on getEncodingClass() to do null check on name.

        /* Get the reader from the encoding */
        StreamReader fr = (StreamReader)getEncodingClass(name, "_Reader");

        /* Open the connection and return*/
        return fr.open(is, name);
    }

    /**
     * Get a writer for an OutputStream
     *
     * @param  os              The output stream the reader is for
     * @return                 A new writer for the stream
     */
    public static Writer getStreamWriter(OutputStream os) {
        try {
            return getStreamWriter(os, defaultEncoding);
        } catch(UnsupportedEncodingException x) {
            throw new RuntimeException(x.getMessage());
        }
    }


    /**
     * Get a writer for an OutputStream
     *
     * @param  os              The output stream the reader is for
     * @param  name            The name of the decoder
     * @return                 A new writer for the stream
     * @exception UnsupportedEncodingException  If the encoding is not known
     */
    public static Writer getStreamWriter(OutputStream os, String name)
        throws UnsupportedEncodingException {

        /* Test for null arguments */
        if (os == null)
            throw new NullPointerException();

        // Rely on getEncodingClass() to do null check on name.

        /* Get the writer from the encoding */
        StreamWriter sw = (StreamWriter)getEncodingClass(name, "_Writer");

        /* Open it on the output stream and return */
        return sw.open(os, name);
    }

    /**
     * Convert a byte array to a char array using the default encoding.
     *
     * @param  buffer          The byte array buffer
     * @param  offset          The offset
     * @param  length          The length
     * @return                 A new char array
     */
    public static
    char[] byteToCharArray(byte[] buffer, int offset, int length) {

        /* Optimise the very common default encoding. We can do a direct
         * test for equality here rather than using String.equals()
         * because of the careful way this private variable has been
         * initialised.
         */
        if (defaultEncoding == "ISO8859_1"){

            // Rely on implicit exception checks to validate the
            // incomming buffer, offset and length arguments.
            // a) length < 0 causes a NegativeArraySizeException
            // b) offset thru offset+length-1 outside of buffer causes
            //    an ArrayIndexOutOfBoundsException
            //
            // However, we need to avoid creating the character array
            // if length is way too big, as this may cause an
            // OutOfMemoryException instead (before hitting condition b).
            // So add an explicit, albeit somewhat inaccurate check.
            if (length > buffer.length)
                length = -1;    // Force an exception

            try {
                char [] carray = new char[length];
                for (int i = length; --i >= 0; )
                    carray[i] = (char)(buffer[offset + i] & 0xff);
                return carray;
            } catch(NegativeArraySizeException ex1) {
            } catch(ArrayIndexOutOfBoundsException ex2) {
            }

            // Callers are expecting an IndexOutOfBoundsException instead.
            throw new IndexOutOfBoundsException();
        }

        try {
            return byteToCharArray(buffer, offset, length, defaultEncoding);
        } catch(UnsupportedEncodingException x) {
            throw new RuntimeException(x.getMessage());
        }
    }

    /**
     * Convert a char array to a byte array
     *
     * @param  buffer          The char array buffer
     * @param  offset          The offset
     * @param  length          The length
     * @return                 A new byte array
     */
    public static
    byte[] charToByteArray(char[] buffer, int offset, int length) {

        /* Optimise the very common default encoding. We can do a direct
         * test for equality here rather than using String.equals()
         * because of the careful way this private variable has been
         * initialised.
         */
         if (defaultEncoding == "ISO8859_1") {
            byte[] barray = new byte[length];
            for (int i = length; --i >= 0; ) {
                int ch = buffer[offset + i];
                barray[i] = (byte)((ch > 255)? '?' : ch);
            }
            return barray;
        }

        try {
            return charToByteArray(buffer, offset, length, defaultEncoding);
        } catch(UnsupportedEncodingException x) {
            throw new RuntimeException(x.getMessage());
        }
    }

    /*
     * Cached variables for byteToCharArray
     */
    static class CachedReader
    {
        final public String encoding;
        final public StreamReader stream;

    public CachedReader(String enc) throws UnsupportedEncodingException
    {
      encoding = enc;
      stream =  (StreamReader)getEncodingClass(enc, "_Reader");
    }

	}
    private static CachedReader cachedReader;

    /**
     * Convert a byte array to a char array.
     * FIXME other VMs "recover" from corrupt encoding bytes and continue
     * processing. Our implementation just gives up early.
     * Would need to fix all the _Reader classes.
     *
     * @param  b    The byte array buffer
     * @param  off  The offset
     * @param  len  The length
     * @param  enc  The character encoding
     * @return      A new char array
     * @exception UnsupportedEncodingException  If the encoding is not known.
     * @exception IndexOutOfBoundsException If b, off, or len not valid.
     */
    public static char[] byteToCharArray(
        byte[] b, int off, int len, String enc)
        throws UnsupportedEncodingException {

        // Ensure the given off and len are valid for b[]
        if ((off | len | (off + len) | (b.length - (off + len))) < 0)
            throw new IndexOutOfBoundsException();

        if(cachedReader==null)
            cachedReader=new CachedReader(enc);

        CachedReader reader = cachedReader;
        String internalName = internalNameForEncoding(enc);
        if (!internalName.equals(reader.encoding))
        {
            reader = new CachedReader(internalName);
            cachedReader = reader;
        }

        // Now "reader.stream" is the correct StreamReader for "enc"
        StreamReader stream = reader.stream;
        synchronized(stream)
        {
            /* Ask the stream for the size the output will be */
            int size = stream.sizeOf(b, off, len);

            /* Allocate a buffer of that size */
            char[] outbuf = new char[size];

            /* Open the reader on a ByteArrayInputStream */
            stream.open(new ByteArrayInputStream(b, off, len),
            internalNameForEncoding(enc));

            try {
                /* Read the input */
                stream.read(outbuf, 0, size);
                /* Close the reader */
                stream.close();
            } catch(IOException x) {

            }

            /* And return the buffer */
            return outbuf;
        }
    }


    /*
     * Cached variables for charToByteArray
     */
    private static String lastWriterEncoding;
    private static StreamWriter lastWriter;

    /**
     * Convert a char array to a byte array.
     * FIXME other VMs "recover" from corrupt encoding bytes and continue
     * processing. Our implementation just gives up early.
     * Would need to fix all the _Writer classes.
     *
     * @param  buffer          The char array buffer
     * @param  offset          The offset
     * @param  length          The length
     * @param  enc             The character encoding
     * @return                 A new byte array
     * @exception UnsupportedEncodingException  If the encoding is not known
     */
    public static synchronized byte[] charToByteArray(
        char[] buffer, int offset, int length, String enc)
        throws UnsupportedEncodingException {

        /* If we don't have a cached writer then make one */
        if(lastWriterEncoding == null || !lastWriterEncoding.equals(enc)) {
            lastWriter = (StreamWriter)getEncodingClass(enc, "_Writer");
            lastWriterEncoding = internalNameForEncoding(enc);
        }

        /* Ask the writeer for the size the output will be */
        int size = lastWriter.sizeOf(buffer, offset, length);

        /* Get the output stream */
        ByteArrayOutputStream os = new ByteArrayOutputStream(size);

        /* Open the writer */
        lastWriter.open(os, internalNameForEncoding(enc));

        try {
            /* Convert */
            lastWriter.write(buffer, offset, length);
            /* Close the writer */
            lastWriter.close();
        } catch(IOException x) {
            // Behaviour undefined: just give up and return what we have
        }

        /* Close the output stream */
        try {
            os.close();
        } catch(IOException x) {};

        /* Return the array */
        return os.toByteArray();
    }

    /**
     * Get the internal name for an encoding.
     *
     * @param encodingName encoding name
     *
     * @return internal name for this encoding
     *
     * @exception NullPointerException if specified encodingName is null.
     */
    private static String internalNameForEncoding(String encodingName) {
        String internalName;
        String property;

        // Rely on encodingName.equals() to throw a NullPointerException

        // Optimise for the (very common) default encoding case
        if (encodingName.equals("ISO8859_1"))
            return "ISO8859_1";

        internalName = normalizeEncodingName(encodingName);

        // Check the built-in encoding map
        for (int i = encodingMapping.length - 3; i >= 0; i -=3)
        {
            // Compare the historical name and canonical names
            if (internalName.equals(encodingMapping[i]) ||
                matchCanonicalName(internalName, encodingMapping[i + 1]))
            {
                // Return the internal encoding name
                return encodingMapping[i + 2];
            }
        }

        /*
         * Since IANA character encoding names can start with a digit
         * and that some Reader class names that do not match the standard
         * name, we have a way to configure alternate names for encodings.
         *
         * Note: The names must normalized, digits, upper case only with "_"
         *       and "_" substituted for ":" and "-".
         */
        property = System.getProperty(internalName + "_InternalEncodingName");
        if (property != null) {
            return property;
        }

        return internalName;
    }

    /**
     * Get the canonical name for an encoding.
     *
     * @param encodingName encoding name or null for the default encoding
     *
     * @return canonical name for this encoding
     */
    public static String canonicalEncodingName(String encodingName)
    {
        String internalName;
        String property;

        if (encodingName == null)
            encodingName = defaultEncoding;

        internalName = normalizeEncodingName(encodingName);

        // Check the built-in encoding map
        for (int i = encodingMapping.length - 3; i >= 0; i -=3)
        {
            // Compare with the historical and canonical names
            if (internalName.equals(encodingMapping[i]) ||
                matchCanonicalName(internalName, encodingMapping[i + 1]))
            {
                // Return the canonical encoding name
                return encodingMapping[i + 1];
            }
        }

        /*
         * Since IANA character encoding names can start with a digit
         * and that some Reader class names that do not match the standard
         * name, we have a way to configure alternate names for encodings.
         *
         * Note: The names must normalized, digits, upper case only with "_"
         *       and "_" substituted for ":" and "-".
         */
        property = System.getProperty(internalName + "_CanonicalEncodingName");
        if (property != null) {
            return property;
        }

        return internalName;
    }
    
    /**
     * Converts "-" and ":" in a string to "_" and converts the name
     * to upper case.
     * This is needed because the names of IANA character encodings have
     * characters that are not allowed for java class names and
     * IANA encoding names are not case sensitive.
     *
     * @param encodingName encoding name
     *
     * @return normalized name
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
     *
     * Characters in the canonical name are converted as if
     * normalizeEncodingName() were called.
     *
     * @param normalizedName internal encoding name returned by
     *        normalizeEncodingName()
     * @param canonicalName canonical encoding name
     *
     * @return true if the names match, false otherwise
     */
    private static boolean matchCanonicalName(String normalizedName,
        String canonicalName)
    {
        int length = canonicalName.length();
        if (normalizedName.length() != length)
            return false;

        for (int i = 0; i < length; i++) {
            char nc = normalizedName.charAt(i);
            char cc = canonicalName.charAt(i);
            if ((cc == '-') || (cc == ':'))
                cc = '_';
            if ((nc != cc) && (nc != Character.toUpperCase(cc)))
                return false;
        }

        return true;
    }
}