/******************************************************************************
 * Copyright (c)2016 Yarlung Soft
 * We don't support i18n now, just return English version message
 ******************************************************************************/
package jp.co.mqttv3;

public class MqttExceptionMessage {
	/*
	 * 1=Invalid protocol version
	 * 2=Invalid client ID
	 * 3=Broker unavailable
	 * 4=Bad user name or password
	 * 5=Not authorized to connect
	 * 6=Unexpected error
	 * 32000=Timed out waiting for a response from the server
	 * 32001=Internal error, caused by no new message IDs being available
	 * 32002=Timed out while waiting to write messages to the server
	 * 32100=Client is connected
	 * 32101=Client is disconnected
	 * 32102=Client is currently disconnecting
	 * 32103=Unable to connect to server
	 * 32104=Client is not connected
	 * 32105=The specified SocketFactory type does not match the broker URI
	 * 32106=SSL configuration error
	 * 32107=Disconnecting is not allowed from a callback method
	 * 32108=Unrecognized packet
	 * 32109=Connection lost
	 * 32110=Connect already in progress
	 * 32111=Client is closed
	 * 32200=Persistence already in use
	 * 32201=Token already in use
	 * 32202=Too many publishes in progress
	 */
	public static final String getExceptionMessage(int reason){
		String message ;
		switch(reason){
		case MqttException.REASON_CODE_INVALID_PROTOCOL_VERSION:  //0x01
			message = "Invalid protocol version";
			break;
		case MqttException.REASON_CODE_INVALID_CLIENT_ID:  //0x02
			message = "Invalid client ID";
			break;
		case MqttException.REASON_CODE_BROKER_UNAVAILABLE:  //0x03
			message = "Broker unavailable";
			break;
		case MqttException.REASON_CODE_FAILED_AUTHENTICATION:  //0x04
			message = "Bad user name or password";
			break;
		case MqttException.REASON_CODE_NOT_AUTHORIZED:  //0x05
			message = "Not authorized to connect";
			break;
		case MqttException.REASON_CODE_UNEXPECTED_ERROR:  //0x06
			message = "Unexpected error";
			break;
		case MqttException.REASON_CODE_CLIENT_TIMEOUT:  //32000
			message = "Timed out waiting for a response from the server";
			break;
		case MqttException.REASON_CODE_NO_MESSAGE_IDS_AVAILABLE:  //32001
			message = "Internal error, caused by no new message IDs being available";
			break;
		case MqttException.REASON_CODE_WRITE_TIMEOUT:  //32002
			message = "Timed out while waiting to write messages to the server";
			break;
		case MqttException.REASON_CODE_CLIENT_CONNECTED:  //32100
			message = "Client is connected";
			break;
		case MqttException.REASON_CODE_CLIENT_ALREADY_DISCONNECTED:  //32101
			message = "Client is disconnected";
			break;
		case MqttException.REASON_CODE_CLIENT_DISCONNECTING:  //32102
			message = "Client is currently disconnecting";
			break;
		case MqttException.REASON_CODE_SERVER_CONNECT_ERROR:  //32103
			message = "Unable to connect to server";
			break;
		case MqttException.REASON_CODE_CLIENT_NOT_CONNECTED:  //32104
			message = "Client is not connected";
			break;
		case MqttException.REASON_CODE_SOCKET_FACTORY_MISMATCH:  //32105
			message = "The specified SocketFactory type does not match the broker URI";
			break;
		case MqttException.REASON_CODE_SSL_CONFIG_ERROR:  //32106
			message = "SSL configuration error";
			break;
		case MqttException.REASON_CODE_CLIENT_DISCONNECT_PROHIBITED:  //32107
			message = "Disconnecting is not allowed from a callback method";
			break;
		case MqttException.REASON_CODE_INVALID_MESSAGE:  //32108
			message = "Unrecognized packet";
			break;
		case MqttException.REASON_CODE_CONNECTION_LOST:  //32109
			message = "Connection lost";
			break;
		case MqttException.REASON_CODE_CONNECT_IN_PROGRESS:  //32110
			message = "Connect already in progress";
			break;			
		case MqttException.REASON_CODE_CLIENT_CLOSED:  //32111
			message = "Client is closed";
			break;
		case MqttException.REASON_CODE_PERSISTENCE_IN_USE:  //32200
			message = "Persistence already in use";
			break;
		case MqttException.REASON_CODE_TOKEN_INUSE:  //32201
			message = "Token already in use";
			break;
		case MqttException.REASON_CODE_MAX_INFLIGHT:  //32202
			message = "Too many publishes in progress";
			break;
		default: message = "Reason: " + reason; break;
		}	
		return message;
	}
}
