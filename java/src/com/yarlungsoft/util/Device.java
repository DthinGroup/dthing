package com.yarlungsoft.util;

public class Device {
	private static native String getImei0();
	
	public static native void restartDevice0();
	
	public 	static String getDeviceIMEI(){
		String imei = getImei0();
		return imei;
	}
	
	public static void restartDevice(){
		restartDevice0();
	}
}
