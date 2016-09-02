package com.yarlungsoft.demo.mqtt;

import com.yarlungsoft.iot.mqttv3.MqttException;
import com.yarlungsoft.iot.mqttv3.simple.ISimpleMqttClient;
import com.yarlungsoft.iot.mqttv3.simple.SimpleMqttClient;
import com.yarlungsoft.util.Log;

import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;


public class Mqtt extends Applet {
	private SimpleMqttClient mqtt;
	public Mqtt() {
		// TODO Auto-generated constructor stub
		Log.log("Mqtt test", "call con");
	}

	public void cleanup() {
		// TODO Auto-generated method stub

	}

	public void processEvent(Event arg0) {
		// TODO Auto-generated method stub

	}

	public void startup() {
		Log.log("Mqtt test", "startup");
		mqtt = SimpleMqttClient.getInstance();
		Log.log("Mqtt test", "thread startup");
		new Thread(new Runnable() {		
			
			public void run() {
				try {
					String url = "tcp://182.61.25.208:1883";					
					Log.log("Mqtt test", "ready to connect - " + url);
					
					int ret = mqtt.connect("tcp", "182.61.25.208", 1883);
					Log.log("Mqtt test", "connect result - " + ret);
					
					if(ret == ISimpleMqttClient.SUCCESS){
						Log.log("Mqtt test", "ready to subscribe");
						ret = mqtt.subscribe("fuck years", null);
						Log.log("Mqtt test", "subscribe ret - " + ret);
					}
					
				} catch (MqttException e) {
					e.printStackTrace();
				} catch (Exception e){
					
				}
			}
		}).start();
	
	}

}
