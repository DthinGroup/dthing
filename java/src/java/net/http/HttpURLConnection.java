package java.net.http;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.ProtocolException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.StringTokenizer;
import java.util.Map.Entry;

import com.yarlungsoft.util.Log;
import com.yarlungsoft.util.TextUtils;

public class HttpURLConnection extends URLConnection {
    private static final String TAG = "HttpURLConnection";

    public static final String GET = "GET";
    public static final String POST = "POST";
    public static final String HEAD = "HEAD";
    public static final String OPTIONS = "OPTIONS";
    public static final String PUT = "PUT";
    public static final String DELETE = "DELETE";
    public static final String TRACE = "TRACE";

    // 2XX: generally "OK"
    // 3XX: relocation/redirect
    // 4XX: client error
    // 5XX: server error
    /**
     * Numeric status code, 202: Accepted
     */
    public static final int HTTP_ACCEPTED = 202;

    /**
     * Numeric status code, 502: Bad Gateway
     */
    public static final int HTTP_BAD_GATEWAY = 502;

    /**
     * Numeric status code, 405: Bad Method
     */
    public static final int HTTP_BAD_METHOD = 405;

    /**
     * Numeric status code, 400: Bad Request
     */
    public static final int HTTP_BAD_REQUEST = 400;

    /**
     * Numeric status code, 408: Client Timeout
     */
    public static final int HTTP_CLIENT_TIMEOUT = 408;

    /**
     * Numeric status code, 409: Conflict
     */
    public static final int HTTP_CONFLICT = 409;

    /**
     * Numeric status code, 201: Created
     */
    public static final int HTTP_CREATED = 201;

    /**
     * Numeric status code, 413: Entity too large
     */
    public static final int HTTP_ENTITY_TOO_LARGE = 413;

    /**
     * Numeric status code, 403: Forbidden
     */
    public static final int HTTP_FORBIDDEN = 403;

    /**
     * Numeric status code, 504: Gateway timeout
     */
    public static final int HTTP_GATEWAY_TIMEOUT = 504;

    /**
     * Numeric status code, 410: Gone
     */
    public static final int HTTP_GONE = 410;

    /**
     * Numeric status code, 500: Internal error
     */
    public static final int HTTP_INTERNAL_ERROR = 500;

    /**
     * Numeric status code, 411: Length required
     */
    public static final int HTTP_LENGTH_REQUIRED = 411;

    /**
     * Numeric status code, 301 Moved permanently
     */
    public static final int HTTP_MOVED_PERM = 301;

    /**
     * Numeric status code, 302: Moved temporarily
     */
    public static final int HTTP_MOVED_TEMP = 302;

    /**
     * Numeric status code, 300: Multiple choices
     */
    public static final int HTTP_MULT_CHOICE = 300;

    /**
     * Numeric status code, 204: No content
     */
    public static final int HTTP_NO_CONTENT = 204;

    /**
     * Numeric status code, 406: Not acceptable
     */
    public static final int HTTP_NOT_ACCEPTABLE = 406;

    /**
     * Numeric status code, 203: Not authoritative
     */
    public static final int HTTP_NOT_AUTHORITATIVE = 203;

    /**
     * Numeric status code, 404: Not found
     */
    public static final int HTTP_NOT_FOUND = 404;

    /**
     * Numeric status code, 501: Not implemented
     */
    public static final int HTTP_NOT_IMPLEMENTED = 501;

    /**
     * Numeric status code, 304: Not modified
     */
    public static final int HTTP_NOT_MODIFIED = 304;

    /**
     * Numeric status code, 200: OK
     */
    public static final int HTTP_OK = 200;

    /**
     * Numeric status code, 206: Partial
     */
    public static final int HTTP_PARTIAL = 206;

    /**
     * Numeric status code, 402: Payment required
     */
    public static final int HTTP_PAYMENT_REQUIRED = 402;

    /**
     * Numeric status code, 412: Precondition failed
     */
    public static final int HTTP_PRECON_FAILED = 412;

    /**
     * Numeric status code, 407: Proxy authentication required
     */
    public static final int HTTP_PROXY_AUTH = 407;

    /**
     * Numeric status code, 414: Request too long
     */
    public static final int HTTP_REQ_TOO_LONG = 414;

    /**
     * Numeric status code, 205: Reset
     */
    public static final int HTTP_RESET = 205;

    /**
     * Numeric status code, 303: See other
     */
    public static final int HTTP_SEE_OTHER = 303;

    /**
     * Numeric status code, 305: Use proxy
     */
    public static final int HTTP_USE_PROXY = 305;

    /**
     * Numeric status code, 401: Unauthorized
     */
    public static final int HTTP_UNAUTHORIZED = 401;

    /**
     * Numeric status code, 415: Unsupported type
     */
    public static final int HTTP_UNSUPPORTED_TYPE = 415;

    /**
     * Numeric status code, 503: Unavailable
     */
    public static final int HTTP_UNAVAILABLE = 503;

    /**
     * Numeric status code, 505: Version not supported
     */
    public static final int HTTP_VERSION = 505;

    private static final String SP = " ";
    private static final String CRLF = "\r\n";
    private static final String HTTP = "HTTP/1.0";
    private static final String KV_SEGM = ": ";
    private static final String[] methods = {GET, POST, HEAD, OPTIONS, PUT, DELETE, TRACE};

    // connected is in URLConnection

    private String method = GET;

    private volatile boolean isRequested = false;

    private int responseCode = -1;
    private String responseHttp;
    private String responseMessage;

    private ByteArrayOutputStream userOutput;
    private Socket socket;
    private InputStream socketIn;
    private OutputStream socketOut;
    private InetSocketAddress socketAddress = null;

    protected HttpURLConnection(URL url) {
        super(url);
        socketIn = null;
        socketOut = null;
        connected = false;
        isRequested = false;
        userOutput = new ByteArrayOutputStream();
    }

    // @Override
    public void connect() throws IOException {
        Log.netLog(TAG, "connect() begin");
        if (connected) {
            Log.netLog(TAG, "connect() already connected, return");
            return;
        }
        Log.netLog(TAG, "connect() start connect");
        int port = getPort();
        Log.netLog(TAG, "connect() port=" + port);
        try {
            Log.netLog(TAG, "connect() set socketAddress");
            socketAddress = new InetSocketAddress(url.getHost(), port);
            Log.netLog(TAG, "connect() socketAddress:" + socketAddress);
        } catch (Exception e) {
            Log.netLog(TAG, "connect() set socketAddress failed:" + e);
            throw new IOException("Create socket fail!");
        }
        Log.netLog(TAG, "connect() set socket");
        socket = new Socket();
        Log.netLog(TAG, "connect() socket:" + socket);
        // socket.setSoTimeout(connectTimeout);
        Log.netLog(TAG, "connect() before socket.connect(socketAddress)");
        socket.connect(socketAddress);
        Log.netLog(TAG, "connect() after socket.connect(socketAddress)");
        socketIn = socket.getInputStream();
        Log.netLog(TAG, "connect() socketIn:" + socketIn);
        socketOut = socket.getOutputStream();
        Log.netLog(TAG, "connect() socketOut:" + socketOut);
        connected = true;
        Log.netLog(TAG, "connect() end");
    }

    // @Override
    public InputStream getInputStream() throws IOException {
        if (!doInput) {
            throw new IOException("Can't get inPut stream!");
        }
        connect();
        doRequest();
        return socketIn;
    }

    // @Override
    public OutputStream getOutputStream() throws IOException {
        if (!doOutput) {
            throw new IOException("Can't get outputStream if doOutput=false - call setDoOutput(true)");
        }
        if (isRequested) {
            throw new IOException("Cannot write output after reading input");
        }

        return userOutput;
    }

    // @Override
    public int getDefaultPort() {
        return 80;
    }

    private int getPort() {
        if (url.getPort() >= 0) {
            return url.getPort();
        }
        return getDefaultPort();
    }

    private void doRequest() throws IOException {
        if (!connected) {
            throw new IOException("Socket not connected!");
        }

        if (isRequested) {
            return;
        }

        // HTTP 1/1 requests require the Host header to
        // distinguish virtual host locations.
        if (!requestProperties.containsKey("Host")) {
            requestProperties.put("Host", url.getHost() + ":" + getPort());
        }
        if (!requestProperties.containsKey("User-Agent")) {
            requestProperties.put("User-Agent", "DthingClient/0.1");
        }

        int userDataLen = 0;
        byte[] userData = null;
        if (method.equals(PUT) || method.equals(POST)) {
            userData = userOutput.toByteArray();
            if (userData != null && userData.length > 0) {
                userDataLen = userData.length;
            }
            requestProperties.put("Content-Length", String.valueOf(userDataLen));
        }

        String requestHeader = getRequestHeaderStr();
        socketOut.write(requestHeader.getBytes());
        if (userDataLen > 0 && userData != null) {
            socketOut.write(userData);
            socketOut.write(CRLF.getBytes());
        }
        socketOut.write(CRLF.getBytes());
        socketOut.flush();

        // read HTTP response
        String responseHeader = readHeader();
        if (TextUtils.isEmpty(responseHeader)) {
            throw new IOException("Request header fail");
        }
        parseHeader(responseHeader);

        Log.netLog(TAG, "doRequest() end");
        isRequested = true;
    }

    private void getRequestStatusLine(StringBuffer buf) {
        buf.append(method).append(SP);

        if (TextUtils.isEmpty(url.getPath())) {
            buf.append('/');
        } else {
            buf.append(url.getPath());
        }
        if (url.getRef() != null) {
            buf.append('#').append(url.getRef());
        }
        if (url.getQuery() != null) {
            buf.append('?').append(url.getQuery());
        }
        buf.append(SP).append(HTTP).append(CRLF);
        Log.netLog(TAG, "Http Status line:" + buf);
    }

    private String getRequestHeaderStr() {
        StringBuffer header = new StringBuffer();
        getRequestStatusLine(header);
        for (Entry<String, String> entry : requestProperties.entrySet()) {
            String key = entry.getKey();
            String value = entry.getValue();
            header.append(key).append(KV_SEGM).append(value).append(CRLF);
        }
        header.append(CRLF);

        Log.netLog(TAG, "http header:\n" + header.toString());
        return header.toString();
    }

    private String readHeader() throws IOException {
        StringBuffer buf = new StringBuffer();
        int l = 0;
        int c = -1;
        while ((c = socketIn.read()) != -1) {
            buf.append((char) c);

            l = buf.length();
            if (l > 3 && buf.charAt(l - 4) == '\r' && buf.charAt(l - 3) == '\n' && buf.charAt(l - 2) == '\r'
                && buf.charAt(l - 1) == '\n') {
                // reach HTTP response header end
                break;
            }
        }
        Log.netLog(TAG, "readHeader() header:" + buf);
        return buf.toString();
    }

    private void parseHeader(String header) throws IOException {
        Log.netLog(TAG, "parseHeader() header=" + header);

        if (TextUtils.isEmpty(header)) {
            throw new IOException("parse empty header");
        }

        StringTokenizer st = new StringTokenizer(header, CRLF);
        int tkCnt = st.countTokens();
        if (tkCnt <= 0) {
            throw new IOException("parse header fail!");
        }

        int i = 0;
        String head[] = new String[tkCnt];
        while (st.hasMoreTokens()) {
            head[i++] = st.nextToken();
        }

        responseMessage = "";
        int pos = head[0].indexOf(SP);
        if (pos == -1) {
            Log.netLog(TAG, "parseHeader() no space in response status line");
            responseHttp = head[0].trim();
        } else {
            Log.netLog(TAG, "parseHeader() first space in response status line:" + pos);
            responseHttp = head[0].substring(0, pos).trim();
            int p = pos + SP.length();
            pos = head[0].indexOf(SP, p);
            Log.netLog(TAG, "parseHeader() second space in response status line:" + pos);
            if (pos != -1) {
                try {
                    responseCode = Integer.parseInt(head[0].substring(p, pos).trim());
                } catch (NumberFormatException e) {
                    Log.netLog(TAG, "parseHeader() fail to parse responseCode:" + e);
                }

                p = pos + SP.length();
                if (p < head[0].length()) {
                    responseMessage = head[0].substring(p).trim();
                }
            }
        }

        Log.netLog(TAG, "Response Http:" + responseHttp);
        Log.netLog(TAG, "Response Code:" + responseCode);
        Log.netLog(TAG, "Response Message:" + responseMessage);

        String key, val;
        pos = -1;
        for (i = 1; i < head.length; i++) {
            pos = head[i].indexOf(KV_SEGM);
            if (pos == -1) {
                Log.netLog(TAG, "illegal response header [" + i + "]:" + head[i]);
                key = head[i].trim();
                val = "";
            } else {
                Log.netLog(TAG, "response header [" + i + "]:" + head[i]);
                key = head[i].substring(0, pos).trim();
                val = head[i].substring(pos + KV_SEGM.length()).trim();
            }
            responseProperties.put(key, val);
            Log.netLog(TAG, "response key:" + key + "  val:" + val);
        }
    }

    // support public
    public void disconnect() {
        try {
            if (socketIn != null) {
                socketIn.close();
                socketIn = null;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        try {
            if (socketOut != null) {
                socketOut.close();
                socketOut = null;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        try {
            if (socket != null) {
                socket.close();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        connected = false;
        isRequested = false;
        requestProperties.clear();
        responseProperties.clear();
    }

    public InputStream getErrorStream() {
        return null;
    }

    public static boolean getFollowRedirects() {
        return false;
    }

    public static void setFollowRedirects(boolean set) {
    }

    // @Override
    public String getHeaderField(int idx) {
        if (idx < 0 || idx >= responseProperties.size()) {
            return null;
        }

        int i = 0;
        for (Entry<String, String> entry : responseProperties.entrySet()) {
            if (i == idx) {
                return entry.getValue();
            }
            i++;
        }
        return null;
    }

    // @Override
    public String getHeaderField(String name) {
        return responseProperties.get(name);
    }

    public long getHeaderFieldDate(String name, long Default) {
        Log.netLog(TAG, "getHeaderFieldDate() not supported yet!");
        return Default;
    }

    public String getHeaderFieldKey(int n) {
        Log.netLog(TAG, "getHeaderFieldKey() not supported yet!");
        return null;
    }

    public boolean getInstanceFollowRedirects() {
        return false;
    }

    public void setInstanceFollowRedirects() {
    }

    public int getResponseCode() throws IOException {
        if (isRequested) {
            return responseCode;
        }
        getInputStream();
        return responseCode;
    }

    public String getResponseMessage() throws IOException {
        if (isRequested) {
            return responseMessage;
        }
        getInputStream();
        return responseMessage;
    }

    public void setChunkedStreamingMode(int chunklen) {
        throw new IllegalStateException("not support setChunkedStreamingMode for now!");
    }

    public void setFixedLengthStreamingMode(int contentlen) {
        throw new IllegalStateException("not support setFixedLengthStreamingMode for now!");
    }

    public void setRequestMethod(String method) throws ProtocolException {
        if (connected) {
            throw new ProtocolException("Can't reset method: already connected");
        }

        for (int i = 0; i < methods.length; i++) {
            if (methods[i].equals(method)) {
                this.method = method;
                return;
            }
        }
        throw new ProtocolException("Invalid HTTP method: " + method);
    }

    public String getRequestMethod() {
        return method;
    }

    // not support
    // public Permission getPermission(){}
    // public abstract boolean usingProxy(){}
}
