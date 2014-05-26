
package jp.co.cmcc.http;

import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Enumeration;
import java.util.Hashtable;

import com.yarlungsoft.util.Log;

public class HttpConnectionImpl {

    /** HTTP Head method. */
    public final static String HEAD = "HEAD";
    /** HTTP Get method. */
    public final static String GET = "GET";
    /** HTTP Post method. */
    public final static String POST = "POST";

    /** 2XX: generally "OK" */
    /** 200: The request has succeeded. */
    public static final int HTTP_OK = 200;

    /**
     * 201: The request has been fulfilled and resulted in a new
     * resource being created.
     */
    public static final int HTTP_CREATED = 201;

    /**
     * 202: The request has been accepted for processing, but the processing
     * has not been completed.
     */
    public static final int HTTP_ACCEPTED = 202;

    /**
     * 203: The returned meta-information in the entity-header is not the
     * definitive set as available from the origin server.
     */
    public static final int HTTP_NOT_AUTHORITATIVE = 203;

    /**
     * 204: The server has fulfilled the request but does not need to
     * return an entity-body, and might want to return updated
     * meta-information.
     */
    public static final int HTTP_NO_CONTENT = 204;

    /**
     * 205: The server has fulfilled the request and the user agent SHOULD reset
     * the document view which caused the request to be sent.
     */
    public static final int HTTP_RESET = 205;

    /**
     * 206: The server has fulfilled the partial GET request for the resource.
     */
    public static final int HTTP_PARTIAL = 206;

    /** 3XX: relocation/redirect */

    /**
     * 300: The requested resource corresponds to any one of a set of
     * representations, each with its own specific location, and agent-
     * driven negotiation information is being provided so that
     * the user (or user agent) can select a preferred representation and
     * redirect its request to that location.
     */
    public static final int HTTP_MULT_CHOICE = 300;

    /**
     * 301: The requested resource has been assigned a new permanent URI and
     * any future references to this resource SHOULD use one of the returned
     * URIs.
     */
    public static final int HTTP_MOVED_PERM = 301;

    /**
     * 302: The requested resource resides temporarily under a
     * different URI. (<strong>Note:</strong> the name of this
     * status code
     * reflects the earlier publication of RFC2068, which
     * was changed in RFC2616 from "moved temporalily" to
     * "found". The semantics were not changed. The <code>Location</code> header indicates where the
     * application should resend
     * the request.)
     */
    public static final int HTTP_MOVED_TEMP = 302;

    /**
     * 303: The response to the request can be found under a different URI and
     * SHOULD be retrieved using a GET method on that resource.
     */
    public static final int HTTP_SEE_OTHER = 303;

    /**
     * 304: If the client has performed a conditional GET request and access is
     * allowed, but the document has not been modified, the server SHOULD
     * respond with this status code.
     */
    public static final int HTTP_NOT_MODIFIED = 304;

    /**
     * 305: The requested resource MUST be accessed through the proxy given by
     * the Location field.
     */
    public static final int HTTP_USE_PROXY = 305;

    /**
     * 307: The requested resource resides temporarily under a different
     * URI.
     */
    public static final int HTTP_TEMP_REDIRECT = 307;

    /** 4XX: client error */
    /**
     * 400: The request could not be understood by the server due to malformed
     * syntax.
     */
    public static final int HTTP_BAD_REQUEST = 400;

    /**
     * 401: The request requires user authentication. The response MUST
     * include a WWW-Authenticate header field containing a challenge
     * applicable to the requested resource.
     */
    public static final int HTTP_UNAUTHORIZED = 401;

    /** 402: This code is reserved for future use. */
    public static final int HTTP_PAYMENT_REQUIRED = 402;

    /**
     * 403: The server understood the request, but is refusing to fulfill it.
     * Authorization will not help and the request SHOULD NOT be repeated.
     */
    public static final int HTTP_FORBIDDEN = 403;

    /**
     * 404: The server has not found anything matching the Request-URI. No
     * indication is given of whether the condition is temporary or
     * permanent.
     */
    public static final int HTTP_NOT_FOUND = 404;

    /**
     * 405: The method specified in the Request-Line is not allowed for the
     * resource identified by the Request-URI.
     */
    public static final int HTTP_BAD_METHOD = 405;

    /**
     * 406: The resource identified by the request is only capable of generating
     * response entities which have content characteristics not acceptable
     * according to the accept headers sent in the request.
     */
    public static final int HTTP_NOT_ACCEPTABLE = 406;

    /**
     * 407: This code is similar to 401 (Unauthorized), but indicates that the
     * client must first authenticate itself with the proxy.
     */
    public static final int HTTP_PROXY_AUTH = 407;

    /**
     * 408: The client did not produce a request within the time that the server
     * was prepared to wait. The client MAY repeat the request without
     * modifications at any later time.
     */
    public static final int HTTP_CLIENT_TIMEOUT = 408;

    /**
     * 409: The request could not be completed due to a conflict with
     * the current state of the resource.
     */
    public static final int HTTP_CONFLICT = 409;

    /**
     * 410: The requested resource is no longer available at the server and no
     * forwarding address is known.
     */
    public static final int HTTP_GONE = 410;

    /**
     * 411: The server refuses to accept the request without a defined Content-
     * Length.
     */
    public static final int HTTP_LENGTH_REQUIRED = 411;

    /**
     * 412: The precondition given in one or more of the request-header fields
     * evaluated to false when it was tested on the server.
     */
    public static final int HTTP_PRECON_FAILED = 412;

    /**
     * 413: The server is refusing to process a request because the request
     * entity is larger than the server is willing or able to process.
     */
    public static final int HTTP_ENTITY_TOO_LARGE = 413;

    /**
     * 414: The server is refusing to service the request because the
     * Request-URI is longer than the server is willing to interpret.
     */
    public static final int HTTP_REQ_TOO_LONG = 414;

    /**
     * 415: The server is refusing to service the request because the entity of
     * the request is in a format not supported by the requested resource
     * for the requested method.
     */
    public static final int HTTP_UNSUPPORTED_TYPE = 415;

    /**
     * 416: A server SHOULD return a response with this status code if a request
     * included a Range request-header field , and none of
     * the range-specifier values in this field overlap the current extent
     * of the selected resource, and the request did not include an If-Range
     * request-header field.
     */
    public static final int HTTP_UNSUPPORTED_RANGE = 416;

    /**
     * 417: The expectation given in an Expect request-header field
     * could not be met by this server, or, if the server is a proxy,
     * the server has unambiguous evidence that the request could not be met
     * by the next-hop server.
     */
    public static final int HTTP_EXPECT_FAILED = 417;

    /** 5XX: server error */
    /**
     * 500: The server encountered an unexpected condition which prevented it
     * from fulfilling the request.
     */
    public static final int HTTP_INTERNAL_ERROR = 500;

    /**
     * 501: The server does not support the functionality required to
     * fulfill the request.
     */
    public static final int HTTP_NOT_IMPLEMENTED = 501;

    /**
     * 502: The server, while acting as a gateway or proxy, received an invalid
     * response from the upstream server it accessed in attempting to
     * fulfill the request.
     */
    public static final int HTTP_BAD_GATEWAY = 502;

    /**
     * 503: The server is currently unable to handle the request due to a
     * temporary overloading or maintenance of the server.
     */
    public static final int HTTP_UNAVAILABLE = 503;

    /**
     * 504: The server, while acting as a gateway or proxy, did not receive a
     * timely response from the upstream server specified by the URI
     * or some other auxiliary server it needed
     * to access in attempting to complete the request.
     */
    public static final int HTTP_GATEWAY_TIMEOUT = 504;

    /**
     * 505: The server does not support, or refuses to support, the HTTP
     * protocol version that was used in the request message.
     */
    public static final int HTTP_VERSION = 505;

    /**
     * Access mode READ.
     */
    public final static int READ = 1;

    /**
     * Access mode WRITE.
     */
    public final static int WRITE = 2;

    /**
     * Access mode READ_WRITE.
     */
    public final static int READ_WRITE = (READ | WRITE);

    private final static String SCHEME = "http";

    private int index; // used by URL parsing functions
    private String url;
    private String host;
    private String file;
    private String ref;
    private String query;
    private int port = 80;
    private int responseCode;
    private String responseMsg;
    private Hashtable reqProperties;
    Hashtable headerFields; // accessed by inner classes
    private String[] headerFieldNames;
    private String[] headerFieldValues;
    private String method;
    int opens; // accessed by inner classes
    private int mode;
    boolean checkTimeout;

    boolean connected; // accessed by inner classes

    /*
     * In/out streams used to buffer input and output
     */
    private http_PrivateInputStream in;
    private http_PrivateOutputStream out;

    /*
     * The data streams provided to the application.
     * They wrap up the in and out streams.
     */
    private DataInputStream appDataIn;
    private DataOutputStream appDataOut;

    /*
     * The streams from the underlying socket connection.
     */
    private Socket socketConnnection;
    private DataOutputStream streamOutput;
    DataInputStream streamInput; // accessed by inner classes

    public static final String APP_NOT_DEFINED = "application/octet-stream";

    /*
     * A shared temporary buffer used in a couple of places
     */
    private StringBuffer stringbuffer;

    private void log(String msg) {
        Log.netLog("HttpConnectionImpl", msg);
    }

    public HttpConnectionImpl() {
        reqProperties = new Hashtable();
        headerFields = new Hashtable();
        stringbuffer = new StringBuffer(32);
        opens = 0;
        connected = false;
        method = GET;
        responseCode = -1;
    }

    public void CreateHttpConnection(String url, int mode, boolean timeouts) throws IOException {
        if (opens > 0) {
            throw new IOException("already created");
        }

        if (!url.toLowerCase().startsWith(SCHEME)) {
            throw new IOException("invalid url");
        }

        opens++;

        if (mode != READ && mode != WRITE && mode != READ_WRITE) {
            throw new IOException("illegal mode: " + mode);
        }

        int colon = url.indexOf(':');
        this.url = url.substring(colon + 1);
        this.mode = mode;
        checkTimeout = timeouts;

        log("CreateHttpConnection() url = " + this.url);

        parseURL();
    }

    public void DestroyHttpConnection() throws IOException {
        if (!connected) {
            throw new IOException("Not in connected state.");
        }

        try {
            if (socketConnnection != null) {
                streamInput.close();
                streamOutput.close();
                socketConnnection.close();
                socketConnnection = null;
            }
        } catch (IOException ioe) {/* do nothing */
        }

        responseCode = -1;
        responseMsg = null;
        opens = 0;
        connected = false;
    }

    public void doConnection() throws IOException {
        if (connected || (opens == 0)) {
            throw new IOException("Wrong state!");
        }

        // Open socket connection
        socketConnnection = new Socket(host, port);
        // Open data output stream
        streamOutput = new DataOutputStream(socketConnnection.getOutputStream());

        // HTTP 1.1 requests must contain content length for proxies
        if (getRequestProperty("Content-Length") == null) {
            setRequestProperty("Content-Length", "" + (out == null ? 0 : out.size()));
        }

        StringBuffer reqLine = (new StringBuffer(method)).append(' ');

        if (getFile() != null) {
            reqLine.append(getFile());
        } else {
            reqLine.append("/");
        }

        if (getRef() != null) {
            reqLine.append('#').append(getRef());
        }

        if (getQuery() != null) {
            reqLine.append('?').append(getQuery());
        }

        reqLine.append(" HTTP/1.1\n");

        streamOutput.write(reqLine.toString().getBytes());

        // HTTP 1/1 requests require the Host header to distinguish virtual host locations.
        setRequestProperty("Host", host + ':' + port);

        Enumeration reqKeys = reqProperties.keys();
        while (reqKeys.hasMoreElements()) {
            String key = (String) reqKeys.nextElement();
            String reqPropLine = key + ": " + reqProperties.get(key) + "\n";
            streamOutput.write((reqPropLine).getBytes());
        }

        streamOutput.write("\n".getBytes());

        if (out != null) {
            streamOutput.write(out.toByteArray());
            // ***Bug 4485901*** streamOutput.write("\n".getBytes());
        }
        streamOutput.flush();

        streamInput = new DataInputStream(socketConnnection.getInputStream());

        readResponseMessage(streamInput);
        readHeaders(streamInput);

        // Ignore a continuation header and read the true headers again.
        // (Bug# 4382226 discovered with Jetty HTTP 1.1 web server.
        if (responseCode == 100) {
            readResponseMessage(streamInput);
            readHeaders(streamInput);
        }

        connected = true;
    }

    public String getContentEncoding() {
        if (!connected) {
            return null;
        }

        try {
            return getHeaderField("content-encoding");
        } catch (IOException ie) {
            log("getContentEncoding() exception happened!");
            return null;
        }
    }

    public long getContentLength() {
        if (!connected) {
            return -1;
        }

        int ret = getHeaderFieldInt("content-length", -1);
        return (ret == 0) ? -1 : ret;
    }

    public String getContentType() {
        if (!connected) {
            return null;
        }

        try {
            String ret = getHeaderField("content-type");
            return (ret.equals(APP_NOT_DEFINED)) ? null : ret;
        } catch (IOException ie) {
            log("getContentType() exception happened!");
            return null;
        }
    }

    public String getHeaderField(String name) throws IOException {
        if (!connected) {
            throw new IOException("wrong state!");
        }

        return (String) headerFields.get(name.toLowerCase());
    }

    public int getHeaderFieldInt(String name, int def) {
        try {
            return Integer.parseInt(getHeaderField(name));
        } catch (Throwable t) {
        }

        return def;
    }

    public int getResponseCode() throws IOException {
        if (!connected) {
            throw new IOException("wrong state!");
        }
        return responseCode;
    }

    public String getResponseMessage() throws IOException {
        if (!connected) {
            throw new IOException("wrong state!");
        }
        return responseMsg;
    }

    public String getURL() throws IOException {
        if (!connected) {
            throw new IOException("wrong state!");
        }

        // RFC: Add back SCHEME stripped by Content Connection.
        return SCHEME + ':' + url;
    }

    public DataInputStream openDataInputStream() throws IOException {

        if (appDataIn != null) {
            throw new IOException("already open");
        }

        // TBD: throw in exception if the connection has been closed.
        if (in == null) {
            openInputStream();
        }

        appDataIn = new DataInputStream(in);
        return appDataIn;
    }

    public DataOutputStream openDataOutputStream() throws IOException {

        if (mode != WRITE && mode != READ_WRITE) {
            throw new IOException("read-only connection");
        }

        // If the connection was opened and closed before the
        // data output stream is accessed, throw an IO exception
        if (opens == 0) {
            throw new IOException("connection is closed");
        }

        if (out != null) {
            throw new IOException("already open");
        }

        opens++;
        out = new http_PrivateOutputStream();
        return new DataOutputStream(out);
    }

    /*
     * Open the input stream if it has not already been opened.
     * @exception IOException is thrown if it has already been
     * opened.
     */
    public InputStream openInputStream() throws IOException {

        if (in != null) {
            throw new IOException("already open");
        }

        if (!connected) {
            throw new IOException("Not in connected state.");
        }

        // Check that the connection was opened for reading
        if (mode != READ && mode != READ_WRITE) {
            throw new IOException("write-only connection");
        }

        opens++;

        in = new http_PrivateInputStream();
        return in;
    }

    public OutputStream openOutputStream() throws IOException {
        // Delegate to openDataOutputStream
        return openDataOutputStream();
    }

    public String getRequestMethod() {
        return method;
    }

    public void setRequestMethod(String method) throws IOException {
        if (connected) {
            throw new IOException("already connected");
        }

        if (method == null) {
            throw new NullPointerException();
        }

        if (!method.equals(HEAD) && !method.equals(GET) && !method.equals(POST)) {

            throw new IOException("unsupported method: " + method);
        }

        this.method = new String(method);
    }

    public String getRequestProperty(String key) {
        return (String) reqProperties.get(key);
    }

    public void setRequestProperty(String key, String value) throws IOException {

        if (connected) {
            throw new IOException("already connected");
        }
        reqProperties.put(key, value);
    }

    /**
     * PrivateInputStream to handle chunking for HTTP/1.1.
     */
    class http_PrivateInputStream extends InputStream {

        int bytesleft; // Number of bytes left in current chunk
        int bytesread; // Number of bytes read since the stream was opened
        boolean chunked; // True if Transfer-Encoding: chunked
        boolean eof; // True if EOF seen

        http_PrivateInputStream() throws IOException {
            bytesleft = 0;
            bytesread = 0;
            chunked = false;
            eof = false;

            // Determine if this is a chunked datatransfer and setup
            String te = (String) headerFields.get("transfer-encoding");
            if (te != null && te.equals("chunked")) {
                chunked = true;
                bytesleft = readChunkSize();
                eof = (bytesleft == 0);
            }
        }

        /**
         * Returns the number of bytes that can be read (or skipped over)
         * from this input stream without blocking by the next caller of
         * a method for this input stream.
         * This method simply returns the number of bytes left from a
         * chunked response from an HTTP 1.1 server.
         */
        public int available() throws IOException {

            if (connected) {
                return bytesleft;
            } else {
                throw new IOException("connection is not open");
            }
        }

        /**
         * Reads the next byte of data from the input stream.
         * The value byte is returned as an <code>int</code> in
         * the range <code>0</code> to <code>255</code>.
         * If no byte is available because the end of the stream
         * has been reached, the value <code>-1</code> is returned.
         * This method blocks until input data is available, the
         * end of the stream is detected, or an exception is thrown.
         * <p>
         * A subclass must provide an implementation of this method.
         *
         * @return the next byte of data, or <code>-1</code> if the end of the stream is reached.
         * @exception IOException if an I/O error occurs.
         */
        public int read() throws IOException {

            // Be consistent about returning EOF once encountered.
            if (eof) {
                return -1;
            }

            /*
             * If all the current chunk has been read and this
             * is a chunked transfer then read the next chunk length.
             */
            if (bytesleft <= 0 && chunked) {
                readCRLF(); // Skip trailing \n

                bytesleft = readChunkSize();
                if (bytesleft == 0) {
                    eof = true;
                    return -1;
                }
            }

            int ch = streamInput.read();
            eof = (ch == -1);
            bytesleft--;
            bytesread++;
            return ch;
        }

        /**
         * Reads some number of bytes from the input stream and
         * stores them into the buffer array <code>b</code>.
         * The number of bytes actually read is returned as an integer.
         * This method blocks until input data is available, end of
         * file is detected, or an exception is thrown.
         * (For HTTP requests where the content length has been
         * specified in the response headers, the size of the read
         * may be reduced if there are fewer bytes left than the
         * size of the supplied input buffer.)
         *
         * @param b the buffer into which the data is read.
         * @return the total number of bytes read into the buffer,
         * or <code>-1</code> is there is no more data
         * because the end of the stream has been reached.
         * @exception IOException if an I/O error occurs.
         * @see java.io.InputStream#read(byte[])
         */
        public int read(byte[] b) throws IOException {
            long len = getContentLength();

            if (len != -1) {
                // More bytes are expected
                len -= bytesread;
            } else {
                // Buffered reading in chunks
                len = b.length;
            }

            if (len == 0) {
                eof = true;
                // All expected bytes have been read
                return -1;
            }

            return read(b, 0, (int) (len < b.length ? len : b.length));
        }

        /*
         * Read the chunk size from the input.
         * It is a hex length followed by optional headers (ignored)
         * and terminated with <cr><lf>.
         * Also read any optional trailer that follows the last chunk
         * size (zero) and <cr><lf>.
         */
        private int readChunkSize() throws IOException {
            int size = -1;
            try {
                String chunk = readLine(streamInput);
                if (chunk == null) {
                    throw new IOException("No Chunk Size");
                }

                int i;
                for (i = 0; i < chunk.length(); i++) {
                    char ch = chunk.charAt(i);
                    if (Character.digit(ch, 16) == -1)
                        break;
                }

                /* look at extensions?.... */
                size = Integer.parseInt(chunk.substring(0, i), 16);

            } catch (NumberFormatException e) {
                throw new IOException("Bogus chunk size");
            }

            if (size == 0) {
                /*
                 * REFERENCE: HTTP1.1 document
                 * SECTION: 3.6.1 Chunked Transfer Coding
                 * in some cases there may be an OPTIONAL trailer
                 * containing entity-header fields.
                 */
                readHeaders(streamInput);
            }

            return size;
        }

        /*
         * Read <cr><lf> from the InputStream.
         * @exception IOException is thrown if either <CR> or <LF>
         * is missing.
         */
        private void readCRLF() throws IOException {
            if (streamInput.read() != '\r' || streamInput.read() != '\n') {
                throw new IOException("missing CRLF");
            }
        }

        public void close() throws IOException {
            if (--opens == 0 && connected)
                DestroyHttpConnection();
        }

    } // End of class PrivateInputStream

    /**
     * Private OutputStream to allow the buffering of output
     * so the "Content-Length" header can be supplied.
     */
    class http_PrivateOutputStream extends OutputStream {

        private ByteArrayOutputStream output;

        public http_PrivateOutputStream() {
            output = new ByteArrayOutputStream();
        }

        public void write(int b) throws IOException {
            output.write(b);
        }

        public byte[] toByteArray() {
            return output.toByteArray();
        }

        public int size() {
            return output.size();
        }

        public void close() throws IOException {
            flush();
            if (--opens == 0 && connected)
                DestroyHttpConnection();
        }

    } // End of class PrivateOutputStream

    public String getHost() {
        return (host.length() == 0 ? null : host);
    }

    public String getFile() {
        return (file.length() == 0 ? null : file);
    }

    public String getRef() {
        return (ref.length() == 0 ? null : ref);
    }

    public String getQuery() {
        return (query.length() == 0 ? null : query);
    }

    public int getPort() {
        return port;
    }

    private void readResponseMessage(InputStream in) throws IOException {
        String line = readLine(in);
        int httpEnd, codeEnd;
        String errMessage = null;

        responseCode = -1;
        responseMsg = null;

        malformed: {
            if (line == null) {
                errMessage = "line is null";
                break malformed;
            }

            httpEnd = line.indexOf(' ');

            if (httpEnd < 0) {
                errMessage = "httpEnd less than 0";
                break malformed;
            }

            String httpVer = line.substring(0, httpEnd);

            if (!httpVer.startsWith("HTTP")) {
                errMessage = "httpVer not starts with HTTP";
                break malformed;
            }

            if (line.length() <= httpEnd) {
                errMessage = "line length is less than or equal to httpEnd";
                break malformed;
            }

            codeEnd = line.substring(httpEnd + 1).indexOf(' ');

            if (codeEnd < 0) {
                errMessage = "codeEnd is less than 0";
                break malformed;
            }

            codeEnd += (httpEnd + 1);

            if (line.length() <= codeEnd) {
                errMessage = "line length is less than codeEnd";
                break malformed;
            }

            try {
                responseCode = Integer.parseInt(line.substring(httpEnd + 1, codeEnd));
            } catch (NumberFormatException nfe) {
                errMessage = "NumberFormatException" + nfe;
                break malformed;
            }

            responseMsg = line.substring(codeEnd + 1);
            return;
        }

        throw new IOException("malformed response message:" + errMessage);
    }

    /*
     * Read HTTP header fields terminated by <cr><lf>.
     * Accessed by inner classes.
     */
    void readHeaders(InputStream in) throws IOException {
        String line, key, value;
        int index;

        for (;;) {
            line = readLine(in);

            if (line == null || line.length() == 0)
                return;

            index = line.indexOf(':');

            if (index < 0)
                throw new IOException("malformed header field");

            key = line.substring(0, index);

            if (key.length() == 0)
                throw new IOException("malformed header field");

            if (line.length() <= index + 2) {
                value = "";
            } else {
                value = line.substring(index + 2);
            }

            headerFields.put(key.toLowerCase(), value);
        }
    }

    /*
     * Uses the shared stringbuffer to read a line
     * terminated by <cr><lf> and return it as string.
     * Accessed by inner classes
     */
    String readLine(InputStream in) {
        int c;
        stringbuffer.setLength(0);

        for (;;) {

            try {
                c = in.read();

                if (c < 0) {
                    return null;
                }

                if (c == '\r') {
                    continue;
                }

            } catch (IOException ioe) {
                return null;
            }

            if (c == '\n') {
                break;
            }

            stringbuffer.append((char) c);
        }

        return stringbuffer.toString();
    }

    private String parseHostname() throws IOException {
        String buf = url.substring(index);

        if (buf.startsWith("//")) {
            buf = buf.substring(2);
            index += 2;
        }

        int n = buf.indexOf(':');

        if (n < 0)
            n = buf.indexOf('/');

        if (n < 0)
            n = buf.length();

        String token = buf.substring(0, n);
        index += n;
        return token;
    }

    private int parsePort() throws IOException {
        int p = 80;
        String buf = url.substring(index);

        if (!buf.startsWith(":"))
            return p;

        buf = buf.substring(1);
        index++;

        int n = buf.indexOf('/');
        if (n < 0)
            n = buf.length();

        try {
            p = Integer.parseInt(buf.substring(0, n));

            if (p <= 0) {
                throw new NumberFormatException();
            }
        } catch (NumberFormatException nfe) {
            throw new IOException("invalid port");
        }

        index += n;
        return p;
    }

    private String parseFile() throws IOException {
        String token = "";
        String buf = url.substring(index);

        if (buf.length() == 0)
            return token;

        if (!buf.startsWith("/")) {
            throw new IOException("invalid path");
        }

        int n = buf.indexOf('#');
        int m = buf.indexOf('?');

        if (n < 0 && m < 0) {
            n = buf.length(); // URL does not contain any query or frag id.
        } else if (n < 0 || (m > 0 && m < n)) {
            n = m; // Use query loc if no frag id is present
                   // or if query comes before frag id.
                   // otherwise just strip the frag id.
        }

        token = buf.substring(0, n);
        index += n;
        return token;
    }

    private String parseRef() throws IOException {
        String buf = url.substring(index);

        if (buf.length() == 0 || buf.charAt(0) == '?')
            return "";

        if (!buf.startsWith("#")) {
            throw new IOException("invalid ref");
        }

        int n = buf.indexOf('?');

        if (n < 0)
            n = buf.length();

        index += n;
        return buf.substring(1, n);
    }

    private String parseQuery() throws IOException {
        String buf = url.substring(index);

        if (buf.length() == 0)
            return "";

        if (buf.startsWith("?")) {
            String token = buf.substring(1);
            int n = buf.indexOf('#');

            if (n > 0) {
                token = buf.substring(1, n);
                index += n;
            }
            return token;
        }
        return "";
    }

    protected synchronized void parseURL() throws IOException {
        index = 0;
        host = parseHostname();
        port = parsePort();
        file = parseFile();
        query = parseQuery();
        ref = parseRef();
    }
}
