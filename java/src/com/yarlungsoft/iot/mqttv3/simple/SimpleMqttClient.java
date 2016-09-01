package com.yarlungsoft.iot.mqttv3.simple;

import com.yarlungsoft.iot.mqttv3.IMqttMessageListener;
import com.yarlungsoft.iot.mqttv3.MqttCallback;
import com.yarlungsoft.iot.mqttv3.MqttConnectOptions;
import com.yarlungsoft.iot.mqttv3.MqttException;
import com.yarlungsoft.iot.mqttv3.MqttMessage;
import com.yarlungsoft.iot.mqttv3.MqttPersistenceException;
import com.yarlungsoft.iot.mqttv3.MqttSecurityException;
import com.yarlungsoft.iot.mqttv3.MqttTopic;

public class SimpleMqttClient implements ISimpleMqttClient {

	private static final byte CONNECTED	= 0;
	private static final byte CONNECTING	= 1;
	private static final byte DISCONNECTING	= 2;
	private static final byte DISCONNECTED	= 3;
	private static final byte CLOSED	= 4;
	
	private static final int DEFAULT_PORT = 1883;
	
	private static SimpleMqttClient mInstance = null;
	
	private volatile byte conState = DISCONNECTED;
	private Object	conLock = new Object();  	// Used to synchronize connection state
	
	private SimpleMqttOps mCommonOps ;
	MqttConnectOptions mConnectOptions;
	
	private String serverURI;
	private String scheme;
	private String host;
	private int port;
	
	public static synchronized SimpleMqttClient getInstance(){
	
		if(mInstance == null)
			mInstance = new SimpleMqttClient();
		
		return mInstance;
	}
	
	private SimpleMqttClient(){
		conState = DISCONNECTED;		
		mCommonOps = new SimpleMqttOps();
	}
	
	public void reset(){
		conState = DISCONNECTED;
		mConnectOptions = null;
	}
	
	@Override
	public int connect(String serverURI) throws MqttException, Exception {
		// TODO Auto-generated method stub
		
		return this.connect(serverURI, new MqttConnectOptions());
	}
	
	private void parseURI(String URI) throws Exception{
		if(URI == null){
			throw new NullPointerException("Server URI cannot be null");
		}
		
		scheme = parseScheme(URI);
		host = getHostName(URI);
		port = getPort(URI, DEFAULT_PORT);
		
		if(host == null || host.isEmpty()){
			throw new Exception("cannot parse URI");
		}
	}

	@Override
	public int connect(String serverURI, MqttConnectOptions options) throws MqttException, Exception {
		// TODO Auto-generated method stub
		
		
		if (isConnected()) {
			throw new MqttException(MqttException.REASON_CODE_CLIENT_CONNECTED);
		}
		if (isConnecting()) {
			throw new MqttException(MqttException.REASON_CODE_CONNECT_IN_PROGRESS);
		}
		if (isDisconnecting()) {
			throw new MqttException(MqttException.REASON_CODE_CLIENT_DISCONNECTING);
		}
		if (isClosed()) {
			throw new MqttException(MqttException.REASON_CODE_CLIENT_CLOSED);
		}
		setConnectState(DISCONNECTED);		
		mConnectOptions = options;
		parseURI(serverURI);
		
		int ret = mCommonOps.connect(host, port, options);
		if(ret == ISimpleMqttClient.SUCCESS){
			this.setConnectState(CONNECTED);
		}
		return ret;
	}
	
	private void setConnectState(byte state){
		synchronized (conLock) {
			conState = state;
		}
	}
	
	public boolean isConnected() {
		synchronized (conLock) {
			return conState == CONNECTED;
		}
	}

	public boolean isConnecting() {
		synchronized (conLock) {
			return conState == CONNECTING;
		}
	}

	public boolean isDisconnected() {
		synchronized (conLock) {
			return conState == DISCONNECTED;
		}
	}

	public boolean isDisconnecting() {
		synchronized (conLock) {
			return conState == DISCONNECTING;
		}
	}

	public boolean isClosed() {
		synchronized (conLock) {
			return conState == CLOSED;
		}
	}

	
	private int getPort(String uri, int defaultPort) {
		int port;
		int portIndex = uri.lastIndexOf(':');
		if (portIndex == -1) {
			port = defaultPort;
		}
		else {
			int slashIndex = uri.indexOf('/');
			if (slashIndex == -1) {
				slashIndex = uri.length();
			}
		    port = Integer.parseInt(uri.substring(portIndex + 1, slashIndex));
		}
		return port;
	}

	private String getHostName(String uri) {
		int portIndex = uri.indexOf(':');
		if (portIndex == -1) {
			portIndex = uri.indexOf('/');
		}
		if (portIndex == -1) {
			portIndex = uri.length();
		}
		return uri.substring(0, portIndex);
	}
	
	/**
	 * Validate a URI
	 * @param srvURI
	 * @return the URI type
	 */
	
	// scheme:
	public static String parseScheme(String uri){		
		int colon = findFirstOf(uri, ":", 0, uri.length());
		if (colon < findFirstOf(uri, "/?#", 0, uri.length())) {
			String scheme;
			try {
				scheme = validateScheme(uri, colon);
			} catch (Exception e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
				return "tcp";
			}		
			return scheme;
		}
		
		return "tcp";
	}
	

	private static String validateScheme(String uri, int end) throws Exception {
        if (end == 0) {
            throw new Exception(uri + "Scheme expected 0");
        }

        for (int i = 0; i < end; i++) {
            if (!isValidSchemeChar(i, uri.charAt(i))) {
                throw new Exception(uri + "Illegal character in scheme 0");
            }
        }

        return uri.substring(0, end);
    }

	private static boolean isValidSchemeChar(int index, char c) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
            return true;
        }
        if (index > 0 && ((c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.')) {
            return true;
        }
        return false;
    }
	
	private static int findFirstOf(String string, String chars, int start, int end) {
		for (int i = start; i < end; i++) {
			char c = string.charAt(i);
			if (chars.indexOf(c) != -1) {
				return i;
			}
		}
		return end;
	}


	@Override
	public void unsubscribe(String topicFilter) throws MqttException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void unsubscribe(String[] topicFilters) throws MqttException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void publish(String topic, byte[] payload, int qos, boolean retained)
			throws MqttException, MqttPersistenceException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void publish(String topic, MqttMessage message) throws MqttException, MqttPersistenceException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void setCallback(MqttCallback callback) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public MqttTopic getTopic(String topic) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String getClientId() {
		// TODO Auto-generated method stub
		return mConnectOptions.getClientId();
	}

	@Override
	public void close() throws MqttException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public int subscribe(String topicFilter, IMqttMessageListener messageListener)
			throws MqttException, MqttSecurityException {
		// TODO Auto-generated method stub
		return subscribe(topicFilter, 1, messageListener);
	}

	@Override
	public int subscribe(String topicFilter, int qos, IMqttMessageListener messageListener) throws MqttException {				
		
		if(!isConnected()){
			throw new MqttException(MqttException.REASON_CODE_CLIENT_NOT_CONNECTED);
		}
		
		//TODO: check topicFilter first		
		
		MqttMessage.validateQos(qos);		
		mCommonOps.addMqttMessageListener(topicFilter, messageListener == null ? new DefaultMqttMessageListener() : messageListener);
		
		return mCommonOps.subscribe(topicFilter, qos);
	}
	
	private class DefaultMqttMessageListener implements IMqttMessageListener{

		@Override
		public void messageArrived(String topic, MqttMessage message) throws Exception {
			System.out.println("Handle Default Mqtt Message: Topic - " + topic + ", Message - " + message);
		}
		
	}
}
