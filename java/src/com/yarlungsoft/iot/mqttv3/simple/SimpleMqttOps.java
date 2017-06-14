package com.yarlungsoft.iot.mqttv3.simple;

import java.util.Hashtable;

import com.yarlungsoft.iot.mqttv3.IMqttMessageListener;
import com.yarlungsoft.iot.mqttv3.MqttConnectOptions;
import com.yarlungsoft.iot.mqttv3.MqttMessage;
import com.yarlungsoft.util.Log;

public class SimpleMqttOps {

	private final static String TAG = "SimpleMqttOps";
	private Hashtable<String, IMqttMessageListener> mHashListener;
	
	
	public SimpleMqttOps(){
		mHashListener = new Hashtable<String, IMqttMessageListener>();
	}
	
	/*
	 * mqttVersion: 3/4	 
	 * will: if null, set willFlag = 0
	 * return : -1 - failure  0 -success
	 */
	public native int connect0(String host, int port, String clientId, String name, String pwd, String will, int mqttVersion, int keepAliveInterval, boolean cleanSession); 
		
	public native int subscribe0(String topic, int qos);
	
	public native int unsubscribe0(String topic);
	
	public native int publish0(String topic, String payload, int msgId, int qos, boolean dup, boolean retain);
	
	public native int disconnect0();
	
	public native void close0();
	
	//return 0 - message exists, -1: no message
	public native int recv_message0(String topic, String message);
	
	public int connect(String host, int port, MqttConnectOptions opt){
		
		boolean cleanSession = opt.isCleanSession();		
		int mqttVer = opt.getMqttVersion();
		int keepAliveInterval = opt.getKeepAliveInterval();
		String clientId = opt.getClientId();
		String name = opt.getUserName();
		String passwd = opt.getPassword();
		String willMsg = null;
		
		int ret = connect0(host, port, clientId, name, passwd, willMsg, mqttVer, keepAliveInterval, cleanSession);
		if(ret == ISimpleMqttClient.SUCCESS){
			//TODO : need to fix
			//  new Thread(new MqttReceiveTask()).start();
		}
		Log.log("FUCK", "connect0 ret:" + ret);
		return ret;
	}
	
	public int subscribe(String topicFilter, int qos){		
		return subscribe0(topicFilter, qos);
	}	
	
	public int unsubscribe(String topicFilter){
		int ret ;
		if((ret = unsubscribe0(topicFilter)) == ISimpleMqttClient.SUCCESS){
			removeMqttMessageListener(topicFilter);
		}
		return ret;
	}
	
	public int publish(String topic, MqttMessage message){
		return publish0(topic, new String(message.getPayload()), message.getId(), message.getQos(), message.isDuplicate(), message.isRetained());
	}
	
	public int disconnect(){
		int ret = disconnect0();
		if(ret == ISimpleMqttClient.SUCCESS)
			stopRecvTask();
		
		return ret;
	}
	
	public void close(){
		close0();
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
	
	private volatile boolean isStop = false;
	private void stopRecvTask(){
		isStop = true;
	}
	private class MqttReceiveTask implements Runnable{

		@Override
		public void run() {
			// TODO Auto-generated method stub
			int ret ;
			String topic = new String();
			String message = new String(); 
			while(!isStop){
				Log.log(TAG, "MqttReceiveTask sleep!");
				//Thread.sleep(10);					
				Log.log(TAG, "MqttReceiveTask to recv");
				ret = recv_message0(topic, message);
				Log.log(TAG, "MqttReceiveTask recv:" + ret);
			}
			topic = null;
			message = null;
		}
		
	}
}
