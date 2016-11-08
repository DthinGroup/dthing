package com.yarlungsoft.iot.mqttv3.simple;

public class SimpleMqttClientID {
	public enum CLIENT_ITEM{ID_DEFAULT,ID_EDIAN, ID_SMARTWATER,ID_CARWINDOW};
	
	private final static String CLIENT_ID_DEFAULT_PREFEX = "dtu-client-";	
	private final static String CLIENT_ID_EDIAN_PREFEX = "dtu-edian-client-";
	private final static String CLIENT_ID_SMARTWATER_PREFEX = "dtu-smartwater-client-";
	private final static String CLIENT_ID_CARWINDOW_PREFEX = "dtu-carwindow-client-";	
	private static String client_id_prefex = CLIENT_ID_DEFAULT_PREFEX;
	
	protected static void setClientPrefex(CLIENT_ITEM id){
		switch(id){
		case ID_EDIAN:
			client_id_prefex = CLIENT_ID_EDIAN_PREFEX;
			break;
			
		case ID_SMARTWATER:
			client_id_prefex = CLIENT_ID_SMARTWATER_PREFEX;
			break;
			
		case ID_CARWINDOW:
			client_id_prefex = CLIENT_ID_CARWINDOW_PREFEX;
			break;
		
		case ID_DEFAULT:
		default:
			client_id_prefex = CLIENT_ID_DEFAULT_PREFEX;
			break;
		}
	}
	
	public static String getClientPrefix(){
		return client_id_prefex;
	}
}
