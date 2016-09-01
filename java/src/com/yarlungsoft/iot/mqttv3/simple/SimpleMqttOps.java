package com.yarlungsoft.iot.mqttv3.simple;

import java.util.Hashtable;

import com.yarlungsoft.iot.mqttv3.IMqttMessageListener;
import com.yarlungsoft.iot.mqttv3.MqttConnectOptions;

public class SimpleMqttOps {

	private Hashtable<String, IMqttMessageListener> mHashListener;
	
	
	public SimpleMqttOps(){
		mHashListener = new Hashtable<String, IMqttMessageListener>();
	}
	
	/*
	 * mqttVersion: 3/4	 
	 * will: if null, set willFlag = 0
	 * return : -1 - failure  0 -success
	 */
	public native int connect0(String clientId, String name, String pwd, String will, int mqttVersion, int keepAliveInterval, boolean cleanSession); 
		
	public native int subscribe0(String topic, int qos);
	
	public int connect(String host, int port, MqttConnectOptions opt){
		
		boolean cleanSession = opt.isCleanSession();		
		int mqttVer = opt.getMqttVersion();
		int keepAliveInterval = opt.getKeepAliveInterval();
		String clientId = opt.getClientId();
		String name = opt.getUserName();
		String passwd = opt.getPassword();
		String willMsg = null;
		
		return connect0(clientId, name, passwd, willMsg, mqttVer, keepAliveInterval, cleanSession);		
	}
	
	public int subscribe(String topicFilter, int qos){		
		return subscribe0(topicFilter, qos);
	}	
	
	public void addMqttMessageListener(String topic, IMqttMessageListener listener){
		removeMqttMessageListener(topic);
		mHashListener.put(topic, listener);
	}
	
	public void removeMqttMessageListener(String topic){
		Object obj;
		if((obj = mHashListener.get(topic)) != null)
			mHashListener.remove(obj);			
	}
}
