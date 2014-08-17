package java.net.http;



import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.io.ProtocolException;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.StringTokenizer;
import java.util.Map.Entry;



public class HttpURLConnection extends URLConnection{
	private static boolean DEBUG = true; 
	
	public final static String GET 	= "GET";
	public final static String POST = "POST";
	public final static String HEAD = "HEAD";
	public final static String OPTIONS 	= "OPTIONS";
	public final static String PUT 		= "PUT";
	public final static String DELETE 	= "DELETE";
	public final static String TRACE 	= "TRACE";
	
	// 2XX: generally "OK"
    // 3XX: relocation/redirect
    // 4XX: client error
    // 5XX: server error
    /**
     * Numeric status code, 202: Accepted
     */
    public final static int HTTP_ACCEPTED = 202;

    /**
     * Numeric status code, 502: Bad Gateway
     */
    public final static int HTTP_BAD_GATEWAY = 502;

    /**
     * Numeric status code, 405: Bad Method
     */
    public final static int HTTP_BAD_METHOD = 405;

    /**
     * Numeric status code, 400: Bad Request
     */
    public final static int HTTP_BAD_REQUEST = 400;

    /**
     * Numeric status code, 408: Client Timeout
     */
    public final static int HTTP_CLIENT_TIMEOUT = 408;

    /**
     * Numeric status code, 409: Conflict
     */
    public final static int HTTP_CONFLICT = 409;

    /**
     * Numeric status code, 201: Created
     */
    public final static int HTTP_CREATED = 201;

    /**
     * Numeric status code, 413: Entity too large
     */
    public final static int HTTP_ENTITY_TOO_LARGE = 413;

    /**
     * Numeric status code, 403: Forbidden
     */
    public final static int HTTP_FORBIDDEN = 403;

    /**
     * Numeric status code, 504: Gateway timeout
     */
    public final static int HTTP_GATEWAY_TIMEOUT = 504;

    /**
     * Numeric status code, 410: Gone
     */
    public final static int HTTP_GONE = 410;

    /**
     * Numeric status code, 500: Internal error
     */
    public final static int HTTP_INTERNAL_ERROR = 500;

    /**
     * Numeric status code, 411: Length required
     */
    public final static int HTTP_LENGTH_REQUIRED = 411;

    /**
     * Numeric status code, 301 Moved permanently
     */
    public final static int HTTP_MOVED_PERM = 301;

    /**
     * Numeric status code, 302: Moved temporarily
     */
    public final static int HTTP_MOVED_TEMP = 302;

    /**
     * Numeric status code, 300: Multiple choices
     */
    public final static int HTTP_MULT_CHOICE = 300;

    /**
     * Numeric status code, 204: No content
     */
    public final static int HTTP_NO_CONTENT = 204;

    /**
     * Numeric status code, 406: Not acceptable
     */
    public final static int HTTP_NOT_ACCEPTABLE = 406;

    /**
     * Numeric status code, 203: Not authoritative
     */
    public final static int HTTP_NOT_AUTHORITATIVE = 203;

    /**
     * Numeric status code, 404: Not found
     */
    public final static int HTTP_NOT_FOUND = 404;

    /**
     * Numeric status code, 501: Not implemented
     */
    public final static int HTTP_NOT_IMPLEMENTED = 501;

    /**
     * Numeric status code, 304: Not modified
     */
    public final static int HTTP_NOT_MODIFIED = 304;

    /**
     * Numeric status code, 200: OK
     */
    public final static int HTTP_OK = 200;

    /**
     * Numeric status code, 206: Partial
     */
    public final static int HTTP_PARTIAL = 206;

    /**
     * Numeric status code, 402: Payment required
     */
    public final static int HTTP_PAYMENT_REQUIRED = 402;

    /**
     * Numeric status code, 412: Precondition failed
     */
    public final static int HTTP_PRECON_FAILED = 412;

    /**
     * Numeric status code, 407: Proxy authentication required
     */
    public final static int HTTP_PROXY_AUTH = 407;

    /**
     * Numeric status code, 414: Request too long
     */
    public final static int HTTP_REQ_TOO_LONG = 414;

    /**
     * Numeric status code, 205: Reset
     */
    public final static int HTTP_RESET = 205;

    /**
     * Numeric status code, 303: See other
     */
    public final static int HTTP_SEE_OTHER = 303;

    /**
     * Numeric status code, 305: Use proxy
     */
    public final static int HTTP_USE_PROXY = 305;

    /**
     * Numeric status code, 401: Unauthorized
     */
    public final static int HTTP_UNAUTHORIZED = 401;

    /**
     * Numeric status code, 415: Unsupported type
     */
    public final static int HTTP_UNSUPPORTED_TYPE = 415;

    /**
     * Numeric status code, 503: Unavailable
     */
    public final static int HTTP_UNAVAILABLE = 503;

    /**
     * Numeric status code, 505: Version not supported
     */
    public final static int HTTP_VERSION = 505;
	
	
	
	private static final String SP =" ";
	private static final String CRLF = "\r\n";
	private static final String HTTP = "HTTP/1.1"; 
	private static final String KV_SEGM = ": ";
	private static final String[] methods = {
	    "GET", "POST", "HEAD", "OPTIONS", "PUT", "DELETE", "TRACE"};
	//connected is in URLConnection
	
	private String method = "GET";
	
	private volatile boolean isRequested = false;
	
	private int responseCode =-1;
	private String responseHttp;	
	private String responseMessage;
	
	private ByteArrayOutputStream userOutput;
	private Socket socket;
	private InputStream socketIn;
	private OutputStream socketOut;
	private InetSocketAddress socketAddress =null;
	
	protected HttpURLConnection(URL url) {
		super(url);
		socketIn = null;
		socketOut =null;
		connected = false;
		isRequested = false;
		userOutput = new ByteArrayOutputStream();
	}

	//@Override
	public void connect() throws IOException {
		System.out.println("test point 1");
		if(connected)
			return;
		System.out.println("test point 2");
		int port = url.getPort();
		System.out.println("test point 3");
		if(port ==-1)
			port = getDefaultPort();
		System.out.println("test point 4,port:" + port);
		try{
			socketAddress = new InetSocketAddress(url.getHost(),port);
			System.out.println("test point 5,socketAddress:" + socketAddress);
		}catch(Exception e){
			System.out.println("test point 6");
			throw new IOException("Create socket fail!");
		}
		System.out.println("test point 7");
		socket = new Socket();
		System.out.println("test point 8");
		//socket.setSoTimeout(connectTimeout);
		System.out.println("test point 9");
		socket.connect(socketAddress);
		System.out.println("test point 10");
		socketIn = socket.getInputStream();
		System.out.println("test point 11");
		socketOut = socket.getOutputStream();
		System.out.println("test point 12");

		connected = true;
	}

	//@Override
	public InputStream getInputStream() throws IOException {
		if(!doInput){
			throw new IOException("Can't get inPut stream!");
		}
		connect();
		doRequest();
		return socketIn;
	}

	//@Override
	public OutputStream getOutputStream() throws IOException {
		if(!doOutput){
			throw new IOException("Can't get outputStream if doOutput=false - call setDoOutput(true)");
		}
		if(isRequested){
			throw new IOException("Cannot write output after reading input");
		}
		
		return userOutput;
	}

	//@Override
	public int getDefaultPort() {
		return 80;
	}
	
	private int getPort(){
		if(url.getPort() >0)
			return url.getPort();
		return getDefaultPort();
	}

	
	
	private void doRequest() throws IOException
	{
		String requestHeader ;
		String responseHeader ;
		if(!connected){
			throw new IOException("Socket not connected!");
		}
		
		if(isRequested){
			return;
		}
		
		// HTTP 1/1 requests require the Host header to
		// distinguish virtual host locations.
		if(!requestProperties.containsKey("Host")){
			List<String> arlist = (List<String>) new ArrayList<String>();
			arlist.add((url.getHost() + ':') + getPort());
			requestProperties.put("Host", arlist);
		}
		if(!requestProperties.containsKey("User-Agent")){
			List<String> arlist = (List<String>) new ArrayList<String>();
			arlist.add("DthingClient/0.1");
			requestProperties.put("User-Agent", arlist);
		}

		byte[] userarr= userOutput.toByteArray();
		int len = 0;
		if(userarr != null && userarr.length >0){
			len = userarr.length;			
		}
		List<String> arlist = (List<String>) new ArrayList<String>();
		arlist.add(""+len);
		requestProperties.put("Content-Length", arlist);
		
		requestHeader = getRequestHeaderStr();		
		socketOut.write(requestHeader.getBytes());
		if(userarr != null && userarr.length >0){
			socketOut.write(userarr);			
			socketOut.write(CRLF.getBytes());
		}
		socketOut.write(CRLF.getBytes());
		

		//write user stream
		responseHeader = ReadHeader(socketIn);
		if(responseHeader.isEmpty()){
			throw new IOException("Request header fail");
		}
		parseRequestheader(responseHeader);

		if(DEBUG)
			System.out.println("Body:");
		isRequested = true;
	}

	private String getRequestStatusLine()
	{
		String line = method;
		line += SP;

		if(url.getPath() == null ||url.getPath().isEmpty()){
			line += "/";
		}else{
			line += url.getPath();
		}
		if(url.getRef() !=null){
			line += "#" + url.getRef();
		}
		if(url.getQuery() !=null){
			line += "?" + url.getQuery();
		}
		line += SP + HTTP + CRLF;
		
		if(DEBUG)
			System.out.println("Http Status line:"+line);
		return  line;
	}
	
	private String getRequestHeaderStr()
	{
		StringBuffer header = new StringBuffer(getRequestStatusLine());
		for (Entry<String, List<String>> next : this.requestProperties.entrySet()) {
			String key = next.getKey();
			String value = next.getValue().get(0);
			header.append(key).append(KV_SEGM).append(value).append(CRLF);
		}
		header.append(CRLF);
		
		if(DEBUG)
			System.out.print("http header:\n" + header.toString()+"\n");
		return header.toString();
	}
	

	private String ReadHeader(InputStream is) throws IOException {
        String ClientMessage = "";
        String line;

        BufferedReader bRead = new BufferedReader(new InputStreamReader(socketIn));
        while(null != (line = bRead.readLine()) && !line.isEmpty()){
        	ClientMessage += line + CRLF;
        }
        if(DEBUG)
        	System.out.println("Response header:" +ClientMessage);
        
        /*
        

        byte[] aread = new byte[32];
        int length =0;
        
        try{
        while(0< (length = socketIn.read(aread))){
        	System.out.print("" + new String(aread));
        	aread = new byte[32];
        }
        }catch(Exception e){
        	throw new IOException("read exception");
        }
		*/
        return ClientMessage;
    }

	private void parseRequestheader(String header) throws IOException
	{
		StringTokenizer st = new StringTokenizer(header,CRLF);
		int i=0;
		if(st.countTokens() <=0){
			throw new IOException("parse header fail!");
		}
		String head[] = new String[st.countTokens()];
		while(st.hasMoreElements()){
			head[i++] = st.nextToken();
		}
		//status line
		st = new StringTokenizer(head[0],SP);
		
		responseHttp = st.nextToken().trim();
		responseCode = Integer.parseInt(st.nextToken().trim());
		responseMessage = "";
		while(st.hasMoreElements()){
			responseMessage += st.nextToken()+SP;
		}
		responseMessage.trim();

		if(DEBUG){
			System.out.println("Response Http:" + responseHttp);
			System.out.println("Response Code:" + responseCode);
			System.out.println("Response Message:" + responseMessage);
		}
		String key,val;
		for(i=1;i<head.length;i++){
			key = head[i].substring(0, head[i].indexOf(KV_SEGM)).trim();
			val = head[i].substring(head[i].indexOf(KV_SEGM) + KV_SEGM.length()).trim();
			List<String> list = (List<String>) new ArrayList<String>();
			list.add(val);
			this.responseProperties.put(key, list);
			if(DEBUG){
				System.out.println("Response key:" + key + "  val:" +val);				
			}
		}
		
		/*
		String head[] = header.split(CRLF);
		String statusLine[] = head[0].split(SP,3); 
		responseHttp = statusLine[0];
		responseCode = Integer.parseInt(statusLine[1]);
		responseMessage = statusLine[2];
		for(i=1;i<head.length;i++){
			String kv[] = head[i].split(KV_SEGM);
			List<String> list = new ArrayList<String>();
			list.add(kv[1]);
			this.responseProperties.put(kv[0], list);
		}
		*/
	}

	//support public
	public void disconnect() {
		
		try {
			if(socketIn!=null){
				socketIn.close();
				socketIn = null;
			}
			if(socketOut!=null){				
				socketOut.close();				
				socketOut = null;
			}
			if(socket!=null){
				socket.close();
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
		connected = false;
		isRequested = false;
		this.requestProperties.clear();
		this.responseProperties.clear();
	}
	
	public InputStream getErrorStream() {
		return null;
	}
	
	public static boolean getFollowRedirects(){
		return false;
	}
	
	public static void setFollowRedirects(boolean set){
		
	}
	
	//@Override
	public String getHeaderField(int n) {
		if(n<0 || n >= responseProperties.size()){
			return null;
		}
		int i=0;
		Iterator<Entry<String, List<String>>> iter = responseProperties.entrySet().iterator();
		while(iter.hasNext()){
			
			if(n==i){
				List<String> lstr = iter.next().getValue();
				return lstr.get(0);
			}
			else{
				iter.next();
			}
			i++;
		}
		return null;
	}
	
	//@Override
	public String getHeaderField(String name) {
    	if(this.responseProperties.containsKey(name)){
    		List<String> lstr = responseProperties.get(name);
    		return lstr.get(0);
    	}
    	return null;
    }
	
	public long getHeaderFieldDate(String name,long Default){
		return Default;
	}
	
	public String getHeaderFieldKey(int n){
		return null;
	}
	
	public boolean getInstanceFollowRedirects(){
		return false;
	}
	
	public void setInstanceFollowRedirects(){
		
	}

	public int getResponseCode() throws IOException{
		if(isRequested){
			return responseCode;
		}
		getInputStream();
		return responseCode;
	}
	
	public String getResponseMessage() throws IOException{
		if(isRequested){
			return responseMessage;
		}
		getInputStream();
		return responseMessage;
	}
	
	public void setChunkedStreamingMode(int chunklen){
		throw new IllegalStateException("not support setChunkedStreamingMode for now!");
	}
	
	public void setFixedLengthStreamingMode(int contentlen){
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
	
	//not support
	//public Permission getPermission(){}
	//public abstract boolean usingProxy(){}
}
