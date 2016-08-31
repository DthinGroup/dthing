package com.yarlungsoft.iot.mqttv3;


/**
 * Handles client communications with the server.  Sends and receives MQTT V3
 * messages.
 */
public class ClientComms {
	public static String 		VERSION = "${project.version}";
	public static String 		BUILD_LEVEL = "L${build.level}";
	private static final String CLASS_NAME = ClientComms.class.getName();	

	private static final byte CONNECTED	= 0;
	private static final byte CONNECTING	= 1;
	private static final byte DISCONNECTING	= 2;
	private static final byte DISCONNECTED	= 3;
	private static final byte CLOSED	= 4;

	private IMqttClient 		client;
	private int 					networkModuleIndex;	
	private MqttConnectOptions		conOptions;
	private MqttClientPersistence	persistence;	
	private boolean 				stoppingComms = false;

	private byte	conState = DISCONNECTED;
	private Object	conLock = new Object();  	// Used to synchronize connection state
	private boolean	closePending = false;
	private boolean resting = false;
	
	public ClientComms(IMqttClient client, MqttClientPersistence persistence) throws MqttException {
		this.conState = DISCONNECTED;
		this.client 	= client;
		this.persistence = persistence;		
		
	}
	
	
	
}
