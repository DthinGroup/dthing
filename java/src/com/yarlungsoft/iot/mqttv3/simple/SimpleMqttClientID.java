package com.yarlungsoft.iot.mqttv3.simple;

public class SimpleMqttClientID {
	public static final int CLIENT_ID_DEFAULT = 0x00;
	public static final int CLIENT_ID_EDIAN   = 0x01;
	public static final int CLIENT_ID_SMARTWATER = 0x02;
	public static final int CLIENT_ID_CARWINDOW  =0x03;
	
	private final static String CLIENT_ID_DEFAULT_PREFEX = "dtu-client-";	
	private final static String CLIENT_ID_EDIAN_PREFEX = "dtu-edian-client-";
	private final static String CLIENT_ID_SMARTWATER_PREFEX = "dtu-smartwater-client-";
	private final static String CLIENT_ID_CARWINDOW_PREFEX = "dtu-carwindow-client-";	
	private static String client_id_prefex = CLIENT_ID_DEFAULT_PREFEX;
	
	public static void setClientPrefex(int id){
		System.out.print("SimpleMqttClientID setClientPrefex:" + id);
		switch(id){
		case CLIENT_ID_EDIAN:
			client_id_prefex = CLIENT_ID_EDIAN_PREFEX;
			break;
			
		case CLIENT_ID_SMARTWATER:
			client_id_prefex = CLIENT_ID_SMARTWATER_PREFEX;
			break;
			
		case CLIENT_ID_CARWINDOW:
			client_id_prefex = CLIENT_ID_CARWINDOW_PREFEX;
			break;
		
		case CLIENT_ID_DEFAULT:
		default:
			client_id_prefex = CLIENT_ID_DEFAULT_PREFEX;
			break;
		}
	}
	
	public static String getClientPrefix(){
		return client_id_prefex;
	}
}
