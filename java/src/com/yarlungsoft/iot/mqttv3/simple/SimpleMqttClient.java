package com.yarlungsoft.iot.mqttv3.simple;

import com.yarlungsoft.iot.mqttv3.IMqttClient;
import com.yarlungsoft.iot.mqttv3.IMqttMessageListener;
import com.yarlungsoft.iot.mqttv3.MqttCallback;
import com.yarlungsoft.iot.mqttv3.MqttConnectOptions;
import com.yarlungsoft.iot.mqttv3.MqttException;
import com.yarlungsoft.iot.mqttv3.MqttMessage;
import com.yarlungsoft.iot.mqttv3.MqttPersistenceException;
import com.yarlungsoft.iot.mqttv3.MqttSecurityException;
import com.yarlungsoft.iot.mqttv3.MqttTopic;

public class SimpleMqttClient implements IMqttClient {

	@Override
	public void connect() throws MqttSecurityException, MqttException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void connect(MqttConnectOptions options) throws MqttSecurityException, MqttException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void disconnect() throws MqttException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void disconnect(long quiesceTimeout) throws MqttException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void disconnectForcibly() throws MqttException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void disconnectForcibly(long disconnectTimeout) throws MqttException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void disconnectForcibly(long quiesceTimeout, long disconnectTimeout) throws MqttException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void subscribe(String topicFilter) throws MqttException, MqttSecurityException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void subscribe(String[] topicFilters) throws MqttException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void subscribe(String topicFilter, int qos) throws MqttException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void subscribe(String[] topicFilters, int[] qos) throws MqttException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void subscribe(String topicFilter, IMqttMessageListener messageListener)
			throws MqttException, MqttSecurityException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void subscribe(String[] topicFilters, IMqttMessageListener[] messageListeners) throws MqttException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void subscribe(String topicFilter, int qos, IMqttMessageListener messageListener) throws MqttException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void subscribe(String[] topicFilters, int[] qos, IMqttMessageListener[] messageListeners)
			throws MqttException {
		// TODO Auto-generated method stub
		
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
	public boolean isConnected() {
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public String getClientId() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String getServerURI() {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void setManualAcks(boolean manualAcks) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void messageArrivedComplete(int messageId, int qos) throws MqttException {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void close() throws MqttException {
		// TODO Auto-generated method stub
		
	}

}
