
package com.yarlungsoft.ams;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;

public class JadParser {

    /** Horizontal Tab - Unicode character 0x09. */
    private static final int HT = 0x09;

    /** Line Feed - Unicode character 0x0A. */
    private static final int LF = 0x0A;

    /** Carrage Return - Unicode character 0x0D. */
    private static final int CR = 0x0D;

    /** End Of File - Unicode character 0x1A. */
    private static final int EOF = 0x1A;

    /** Space - Unicode character 0x20. */
    private static final int SP = 0x20;

    /** Signals that there is no remainder. */
    private static final int NO_REMAINDER = -2;

    /** MIDlet property for the application archive URL. */
    private static final String JAR_URL_PROP = "MIDlet-Jar-URL";

    /**
     * Attributes that defines the MIDlet name, icon file, and classpath within the MIDlet suite for the first MIDlet.
     */
    private static final String MIDLET_1_PROP = "MIDlet-1";

    /** Holds the remainder from the last look ahead operation. */
    private int remainder = NO_REMAINDER;

    /** Buffers one line from the stream. */
    private char[] lineBuffer = null;

    /**
     * Parse downloaded jad file to get jar url and classname
     *
     * @param fname
     * @return {@link JadInfo}: jar url, applet classname
     * @throws IOException
     */
    public JadInfo parseJad(String fname) throws IOException {
        if (fname == null || fname.length() == 0) {
            return null;
        }
        InputStream is = new FileInputStream(fname);

        remainder = NO_REMAINDER;
        Reader in = new InputStreamReader(is, "UTF-8");
        lineBuffer = new char[512];
        boolean firstLine = true;
        char[] separators = new char[] {':'};
        JadInfo info = new JadInfo();

        while (true) {
            /**
             * FIXME:
             * readLine treats SPACE at line start as a continued line, this behavior
             * is defined in JAR Manifest specification which is not proper to apply
             * to JAD file too
             */
            // Get next line
            String line = readLine(in);
            if (line == null) {
                break;
            }
            if (firstLine) {
                // The first line may include byte order mask (BOM) of the file.
                // Ignore the potential BOM of UTF8 file as UTF-8 does not have
                // byte order issues and it will cause the first property
                // in manifest file invalid.
                if (line.charAt(0) == '\uFEFF') {
                    line = line.substring(1);
                }
                firstLine = false;
            }

            // blank line separate groups of properties
            if (line.length() == 0) {
                continue;
            }

            int endOfKey = getSeparatorIndex(line, separators);
            if (endOfKey <= 0) {
                continue;
            }

            String key = line.substring(0, endOfKey);

            int startOfValue = endOfKey + 1;
            if (startOfValue > line.length()) {
                continue;
            }

            String value = line.substring(startOfValue).trim();

            checkSetProperty(info, key, value);
        }

        return info;
    }

    /**
     * Reads one line using a given reader. CR, LF, or CR + LF end a line. However lines may be
     * continued by beginning the next line with a space. The end of line and end of file characters
     * and continuation space are dropped.
     *
     * @param in reader for a JAD
     * @return one line of the JAD or null at the end of the JAD
     * @exception IOException thrown by the reader
     */
    private String readLine(Reader in) throws IOException {
        int lastChar = 0;
        int room;
        int offset = 0;
        int c;
        char[] temp;

        room = lineBuffer.length;

        if (remainder != NO_REMAINDER) {
            c = remainder;
            remainder = NO_REMAINDER;
        } else {
            c = in.read();
        }

        for (; c != -1; lastChar = c, c = in.read()) {
            /*
             * if we read the end of the line last time and the next line
             * does not begin with a space we are done. But save this character
             * for next time. CR | LF | CR LF ends a line.
             */
            if (lastChar == LF) {
                if (c == SP) {
                    // Marks a continuation line, throw away the space
                    continue;
                }

                remainder = c;
                break;
            }

            if (lastChar == CR) {
                if (c == SP) {
                    /*
                     * FIXME: manifest spec says a SINGLE space in line start
                     * should be treated as continued line, but below
                     * code ignores the SINGLE restriction
                     */
                    // Marks a continuation line, throw away the space
                    continue;
                }

                if (c != LF) {
                    remainder = c;
                    break;
                }
            }

            /*
             * do not include the end of line characters and the end
             * of file character.
             */
            if (c == CR || c == LF || c == EOF) {
                continue;
            }

            if (--room < 0) {
                temp = new char[offset + 128];
                room = temp.length - offset - 1;
                System.arraycopy(lineBuffer, 0, temp, 0, offset);
                lineBuffer = temp;
            }

            lineBuffer[offset++] = (char) c;
        }

        if ((c == -1) && (offset <= 0)) {
            return null;
        }

        return new String(lineBuffer, 0, offset);
    }

    /**
     * Get the index of the separator in the name-value pair.
     * If there are multiple candidates, choose the firstcomer as the separator.
     *
     * @param line the name-value pair.
     * @param separators the allowed separators, MUST not be null.
     * @return the index of the separator, or -1 if there is no any separator in
     * the string.
     */
    private static int getSeparatorIndex(String line, char[] separators) {
        int index = line.length();

        for (int i = 0; i < separators.length; i++) {
            int tmp = line.indexOf(separators[i]);
            if (tmp != -1 && tmp < index)
                index = tmp;
        }

        if (index == line.length()) {
            // There is no any separator.
            index = -1;
        }

        return index;
    }

    /**
     * Set a property. Subclasses can use this to do any checking or other
     * book-keeping as the properties are read in.
     *
     * @param info
     * @param key the property name
     * @param value the property value
     */
    private static void checkSetProperty(JadInfo info, String key, String value) {
        // keys cannot have whitespace
        if (key.indexOf(SP) < 0 && key.indexOf(HT) < 0) {
            if (key.equals(JAR_URL_PROP)) {
                info.jarUrl = value;
            } else if (key.equals(MIDLET_1_PROP)) {
                int len = value.length();
                int offset = 0; // offset into the string
                for (int i = 0; i < 3 && offset < len; i++) {
                    int start = offset;
                    offset = value.indexOf(',', offset);
                    if (offset < 0) {
                        offset = len;
                    }
                    if (i == 2) { // 0 = name, 1 = icon, 2 = class
                        info.appClass = value.substring(start, offset).trim();
                    }
                    ++offset; // skip the comma, or break the loop if there wasn't one
                }
            }
        }
    }

    /**
     * If the jar URL doesn't start with schema, or if it is not an absolute path, make out it with jad URL.
     * Such as the following jar URL, which should be made out:
     *     foo.jar
     *     foo/foo.jar
     * But the following should not:
     *     http://foo/foo.jar
     *     file:///foo/foo.jar
     *     /foo/foo.jar
     * @param jadUrl
     * @param jarUrl
     * @return
     */
    public static String buildJarUrl(String jadUrl, String jarUrl) {
        int offset = jarUrl.indexOf("://");
        if (offset < 1 && !jarUrl.startsWith("/")) {
            offset = jadUrl.lastIndexOf('/');
            if (offset >= 0) {
                jarUrl = jadUrl.substring(0, offset + 1) + jarUrl;
            }
        }
        return jarUrl;
    }
}
