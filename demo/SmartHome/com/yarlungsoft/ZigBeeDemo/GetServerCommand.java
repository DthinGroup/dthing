package com.yarlungsoft.ZigBeeDemo;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.util.Timer;
import java.util.TimerTask;

import javax.microedition.midlet.MIDletStateChangeException;

import jp.co.aplix.io.CommConnectionImpl;

public class GetServerCommand implements Runnable {
	private Timer mTimer;
	private CommConnectionImpl mSerialPort;
	private final String REMOTE_SERVER_URL = "42.121.18.62";
	//private final String REMOTE_SERVER_URL = "192.168.5.106";
	private final int REMOTE_SERVER_PORT = 7777;
	private final int CMD_NULL = 0;
	private final int CMD_AIR_CONDITIONER = 1;
	private final int CMD_FAN = 2;
	private final int CMD_COUNT = 3;
	private final String NAME_CMD_NULL = "null";
	private final String NAME_CMD_AIR_CONDITIONER = "air-conditioner";
	private final String NAME_CMD_FAN = "fan";
	private final String[] CMD_NAMES = {
			NAME_CMD_NULL,
			NAME_CMD_AIR_CONDITIONER,
			NAME_CMD_FAN
	};
	
	private final int STATUS_REFRIGERATION = 0;
	private final int STATUS_HEATING = 1;
	private final int STATUS_AERATION = 2;
	private final int STATUS_AREFACTION = 3;
	private final int STATUS_WIND_SWING = 4;
	private final int STATUS_AUTO = 5;
	private final int STATUS_POWER = 6;
	private final int STATUS_TEMPERATURE = 7;
	private final int STATUS_WIND_FORCE = 8;
	private final int STATUS_COUNT = 9;
	private int mOldStatus[] = new int[STATUS_COUNT];
	
	private final String NAME_REFRIGERATION = "Refrigeration";
	private final String NAME_HEATING = "Heating";
	private final String NAME_AERATION = "Aeration";
	private final String NAME_AREFACTION = "Arefaction";
	private final String NAME_WIND_SWING = "WindSwing";
	private final String NAME_AUTO = "Auto";
	private final String NAME_POWER = "Power";
	private final String NAME_TEMPERATURE = "Temperature";
	private final String NAME_WIND_FORCE = "WindForce";
	private final String[] STATUS_NAMES = {
			NAME_REFRIGERATION, 
			NAME_HEATING, 
			NAME_AERATION, 
			NAME_AREFACTION, 
			NAME_WIND_SWING, 
			NAME_AUTO, 
			NAME_POWER, 
			NAME_TEMPERATURE, 
			NAME_WIND_FORCE, 
	};
	
	private final char COMMAND_POWER_OFF = '1';
	private final char COMMAND_REFRIDGERATION = '9';
	private final char COMMAND_HEATING = 'C';
	
	private final int BUTTON_WIND_FORCE = 0;
	private final int BUTTON_POWER = 1;
	private final int BUTTON_WIND_SWING = 2;
	private final int BUTTON_SCHEDULE = 3;
	private final int BUTTON_COUNT = 4;
	private final String NAME_SCHEDULE = "Schedule";
	private final String[] BUTTON_NAMES = {
			NAME_WIND_FORCE, 
			NAME_POWER, 
			NAME_WIND_SWING, 
			NAME_SCHEDULE, 
	};

	/**
	 * the data structure is like this:
	 *  |0000|0000|0000|xxxxx....
	 *  |header length(4 bytes)|command(4 bytes)|content length(4 bytes)|content...
	 *  the first two parts are header
	 */
	/*default byte size*/
	private final int DEFAULT_SIZE_LENGTH = 4;
	/*command length*/
	private final int DEFAULT_COMMAND_LENGTH = 4;
	/*the header length = command + command length, */
	private final int DEFAULT_HEADER_LENGTH = DEFAULT_SIZE_LENGTH + DEFAULT_COMMAND_LENGTH;
	/*the content start position = header length + content length*/
	private final int DEFAULT_CONTENT_START_POSITION = DEFAULT_HEADER_LENGTH + DEFAULT_SIZE_LENGTH;
	private final int CONTENT_MIN_LENGTH = 2;
	
	private final int DEFAULT_TIME_INTERVAL = 1000;
	private final int COM_PORT = 1;
	private final int COM_BAUD = 115200;
	private final int AIR_CONDITIONER_MIN_TEMPERATURE = 16;
	private final int AIR_CONDITIONER_MIN_WIND_FORCE = 1;
	
	/** 
	 * Infrared code
	 * now only support temperature control
	private final String AIR_CONDITIONER_POWER_OFF = "1B00A394010020500220A8000000B0A39401002070022000000000B0";
	private final String AIR_CONDITIONER_REFRIGERATION_16 = "1B00A394090020500220A800000030A3940900207002200000000030";
	private final String AIR_CONDITIONER_REFRIGERATION_17 = "1B00A394090120500220A800000040A3940901207002200000000040";
	private final String AIR_CONDITIONER_REFRIGERATION_18 = "1B00A394090220500220A800000050A3940902207002200000000050";
	private final String AIR_CONDITIONER_REFRIGERATION_19 = "1B00A394090320500220A800000060A3940903207002200000000060";
	private final String AIR_CONDITIONER_REFRIGERATION_20 = "1B00A394090420500220A800000070A3940904207002200000000070";
	private final String AIR_CONDITIONER_REFRIGERATION_21 = "1B00A394090520500220A800000080A3940905207002200000000080";
	private final String AIR_CONDITIONER_REFRIGERATION_22 = "1B00A394090620500220A800000090A3940906207002200000000090";
	private final String AIR_CONDITIONER_REFRIGERATION_23 = "1B00A394090720500220A8000000A0A39409072070022000000000A0";
	private final String AIR_CONDITIONER_REFRIGERATION_24 = "1B00A394090820500220A8000000B0A39409082070022000000000B0";
	private final String AIR_CONDITIONER_REFRIGERATION_25 = "1B00A394090920500220A8000000C0A39409092070022000000000C0";
	private final String AIR_CONDITIONER_REFRIGERATION_26 = "1B00A394090A20500220A8000000D0A394090A2070022000000000D0";
	private final String AIR_CONDITIONER_REFRIGERATION_27 = "1B00A394090B20500220A8000000E0A394090B2070022000000000E0";
	private final String AIR_CONDITIONER_REFRIGERATION_28 = "1B00A394090C20500220A8000000F0A394090C2070022000000000F0";
	private final String AIR_CONDITIONER_REFRIGERATION_29 = "1B00A394090D20500220A800000001A394090D207002200000000001";
	private final String AIR_CONDITIONER_REFRIGERATION_30 = "1B00A394090E20500220A800000010A394090E207002200000000010";
	private final String AIR_CONDITIONER_HEATING_16 = "1B00A3940C0020500220A800000060A3940C00207002200000000060";
	private final String AIR_CONDITIONER_HEATING_17 = "1B00A3940C0120500220A800000070A3940C01207002200000000070";
	private final String AIR_CONDITIONER_HEATING_18 = "1B00A3940C0220500220A800000080A3940C02207002200000000080";
	private final String AIR_CONDITIONER_HEATING_19 = "1B00A3940C0320500220A800000090A3940C03207002200000000090";
	private final String AIR_CONDITIONER_HEATING_20 = "1B00A3940C0420500220A8000000A0A3940C042070022000000000A0";
	private final String AIR_CONDITIONER_HEATING_21 = "1B00A3940C0520500220A8000000B0A3940C052070022000000000B0";
	private final String AIR_CONDITIONER_HEATING_22 = "1B00A3940C0620500220A8000000C0A3940C062070022000000000C0";
	private final String AIR_CONDITIONER_HEATING_23 = "1B00A3940C0720500220A8000000D0A3940C072070022000000000D0";
	private final String AIR_CONDITIONER_HEATING_24 = "1B00A3940C0820500220A8000000E0A3940C082070022000000000E0";
	private final String AIR_CONDITIONER_HEATING_25 = "1B00A3940C0920500220A8000000F0A3940C092070022000000000F0";
	private final String AIR_CONDITIONER_HEATING_26 = "1B00A3940C0A20500220A800000001A3940C0A207002200000000001";
	private final String AIR_CONDITIONER_HEATING_27 = "1B00A3940C0B20500220A800000010A3940C0B207002200000000010";
	private final String AIR_CONDITIONER_HEATING_28 = "1B00A3940C0C20500220A800000020A3940C0C207002200000000020";
	private final String AIR_CONDITIONER_HEATING_29 = "1B00A3940C0D20500220A800000030A3940C0D207002200000000030";
	private final String AIR_CONDITIONER_HEATING_30 = "1B00A3940C0E20500220A800000040A3940C0E207002200000000040";
	private final String[] AIR_CONDITIONER_REFRIGERATION_CMDS = {
			AIR_CONDITIONER_REFRIGERATION_16, 
			AIR_CONDITIONER_REFRIGERATION_17, 
			AIR_CONDITIONER_REFRIGERATION_18, 
			AIR_CONDITIONER_REFRIGERATION_19, 
			AIR_CONDITIONER_REFRIGERATION_20, 
			AIR_CONDITIONER_REFRIGERATION_21, 
			AIR_CONDITIONER_REFRIGERATION_22, 
			AIR_CONDITIONER_REFRIGERATION_23, 
			AIR_CONDITIONER_REFRIGERATION_24, 
			AIR_CONDITIONER_REFRIGERATION_25, 
			AIR_CONDITIONER_REFRIGERATION_26, 
			AIR_CONDITIONER_REFRIGERATION_27, 
			AIR_CONDITIONER_REFRIGERATION_28, 
			AIR_CONDITIONER_REFRIGERATION_29, 
			AIR_CONDITIONER_REFRIGERATION_30
	};
	private final String[] AIR_CONDITIONER_HEATING_CMDS = {
			AIR_CONDITIONER_HEATING_16, 
			AIR_CONDITIONER_HEATING_17, 
			AIR_CONDITIONER_HEATING_18, 
			AIR_CONDITIONER_HEATING_19, 
			AIR_CONDITIONER_HEATING_20, 
			AIR_CONDITIONER_HEATING_21, 
			AIR_CONDITIONER_HEATING_22, 
			AIR_CONDITIONER_HEATING_23, 
			AIR_CONDITIONER_HEATING_24, 
			AIR_CONDITIONER_HEATING_25, 
			AIR_CONDITIONER_HEATING_26, 
			AIR_CONDITIONER_HEATING_27, 
			AIR_CONDITIONER_HEATING_28, 
			AIR_CONDITIONER_HEATING_29, 
			AIR_CONDITIONER_HEATING_30
	};
	*/

	public void destroyApp(boolean arg0) throws MIDletStateChangeException {
		// TODO Auto-generated method stub
		mTimer.cancel();
		if (mSerialPort != null) {
			try {
				mSerialPort.close();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}

	public void pauseApp() {
		// TODO Auto-generated method stub
		mTimer.cancel();
	}

	protected void startApp() {
		// TODO Auto-generated method stub
		System.out.println("GetServerCommand startApp");
		mTimer = new Timer();
		mSerialPort = CommConnectionImpl.getComInstance(COM_PORT, COM_BAUD);
		getCommand();
	}

	public void getCommand() {
		new Thread(this).start();
	}

	private byte[] intToByteArray(int val)
	{
	    byte[] buff = new byte[4];

	    buff[0] = (byte) ((val >> 24 ) & 0xff);
	    buff[1] = (byte) ((val >> 16 ) & 0xff);
	    buff[2] = (byte) ((val >>  8 ) & 0xff);
	    buff[3] = (byte) ((val >>  0 ) & 0xff);

	    return buff;
	}
	
	private int byteArrayToInt(byte[] buff, int offset)
	{
		int part1 = (buff[0 + offset] << 24) & 0xff000000;
		int part2 = (buff[1 + offset] << 16) & 0xff0000;
		int part3 = (buff[2 + offset] << 8) & 0xff00;
		int part4 = buff[3 + offset] & 0xff;
	    int val = part1 + part2 + part3 + part4;

	    return val;
	}
	
	private byte[] hexStringToByteArray(String s) {
	    int len = s.length();
	    byte[] data = new byte[len / 2];
	    for (int i = 0; i < len; i += 2) {
	        data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4) + Character.digit(s.charAt(i+1), 16));
	    }
	    return data;
	}
	
	private char decToHex(int dec) {
		final char[] hexDigits = { 
			    '0', '1', '2', '3', '4', '5', '6', '7', 
			    '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
			  };
	    return hexDigits[dec]; 
	  }

	private boolean arraysEqual(int[] array1, int[] array2) {
		if (array1.length != array2.length) {
			return false;
		}
		for (int i = 0; i < array1.length; i++) {
			if (array1[i] != array2[i]) {
				return false;
			}
		}
		return true;
	}
	
	private char getTypeCommand(int type) {
		switch (type) {
		case STATUS_REFRIGERATION:
			return COMMAND_REFRIDGERATION;
		case STATUS_HEATING:
			return COMMAND_HEATING;
		case STATUS_POWER:
			return COMMAND_POWER_OFF;
		default:
			throw new IllegalArgumentException(STATUS_NAMES[type] + " not support now");
		}
	}
	
	private char[] getTemperatureCommandCode(int type, int temperature) {
		char[] temperCodes = new char[2];
		char firstCode = 0;
		char secondCode = 0;
		int temperStart = 0;
		int temperDiff = Math.abs(temperature - AIR_CONDITIONER_MIN_TEMPERATURE);
		
		/**
		 *  Refrigeration start from 3
		 *  Heating start from 6
		 * */
		switch (type) {
		case STATUS_REFRIGERATION:
			temperStart = 3;
			break;
		case STATUS_HEATING:
			temperStart = 6;
			break;
		case STATUS_POWER:
			temperStart = 0xB;
			break;
		default:
			throw new IllegalArgumentException(STATUS_NAMES[type] + " not support now");
		}
		
		/**
		 * keep the value in the range 0x0-0xF
		 * */
		firstCode = decToHex((temperStart + temperDiff) % 0x10);
		if (firstCode == '0') {
			secondCode = '1';
		} else {
			secondCode = '0';
		}
		
		temperCodes[0] = firstCode;
		temperCodes[1] = secondCode;
		return temperCodes;
	}
	
	private String getRemoteControllerCommand(int type, int temperature) {
		System.out.println(STATUS_NAMES[type] + ": temper = " + temperature);
		final String COMMAND_BODY = "1B00A3940t0i20500220A8000000wuA3940t0i2070022000000000wu";
		/**
		 *  there is no String.format() or String.replace(String, String) in J2ME, so what can 
		 *  do is only replace the destination char by char, it looks a little stupid, but still 
		 *  not find a better way to solve this problem yet.
		 *  t ==> command type
		 *  i ==> temperature index
		 *  w ==> temperature first code
		 *  u ==> temperature second code
		 * */
		String command = COMMAND_BODY.replace('t', getTypeCommand(type));
		command = command.replace('i', decToHex(temperature - AIR_CONDITIONER_MIN_TEMPERATURE));
		char[] temperCodes = getTemperatureCommandCode(type, temperature);
		command = command.replace('w', temperCodes[0]);
		command = command.replace('u', temperCodes[1]);
		System.out.println("command = " + command);
		return command;
	}
	
	private byte[] getSendContent(int type) throws IOException {
		ByteArrayOutputStream outputStream =  new ByteArrayOutputStream();
		outputStream.write(intToByteArray(DEFAULT_HEADER_LENGTH));
		outputStream.write(intToByteArray(type));
		return outputStream.toByteArray();
	}
	
	private void sendConditionerCommandToPort(int[] status) {
		if (arraysEqual(mOldStatus, status)) {
			System.out.println("status not change");
			/*if status not change, do not send command*/
			return;
		}
		/*record last status*/
		System.arraycopy(status, 0, mOldStatus, 0, mOldStatus.length);
		try {
			int type = STATUS_COUNT;
			if (status[STATUS_POWER] == 0) {
				type = STATUS_POWER;
			} else if (status[STATUS_REFRIGERATION] == 1) {
				type = STATUS_REFRIGERATION;
			} else if (status[STATUS_HEATING] == 1) {
				type = STATUS_HEATING;
			}
			if (type < STATUS_COUNT) {
				OutputStream outputStream = mSerialPort.openOutputStream();
				System.out.println("openOutputStream done");
				String command = getRemoteControllerCommand(type, status[STATUS_TEMPERATURE]);
				outputStream.write(hexStringToByteArray(command));
				outputStream.flush();
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			System.out.println("IOException:" + e.getMessage());
			e.printStackTrace();
		}
	}
	
	private void parseConditionerCommand(byte[] content, int length) {
		if (length < CONTENT_MIN_LENGTH) {
			throw new IllegalArgumentException("command length(" + length + ") is less than min length" + CONTENT_MIN_LENGTH);
		}
		int[] status = new int[STATUS_COUNT];
		status[STATUS_TEMPERATURE] = AIR_CONDITIONER_MIN_TEMPERATURE;
		status[STATUS_WIND_FORCE] = AIR_CONDITIONER_MIN_WIND_FORCE;
		/**
		 * one function data occupy two bytes, the first byte is function type, the second is function status,
		 * for all status 1 means enabled, 0 means disabled except temperature and wind force. 
		 * the full content should like this:
		 * content[0] = 0  .......... refrigeration
			content[1] = 1  .......... refrigeration status
			content[2] = 1  .......... heating
			content[3] = 0  .......... heating status
			content[4] = 2  .......... aeration
			content[5] = 0  .......... aeration status
			content[6] = 3  .......... arefaction
			content[7] = 0  .......... arefaction status
			content[8] = 4  .......... wind swing
			content[9] = 1  .......... wind swing status
			content[10] = 5  .......... auto
			content[11] = 0  .......... auto status
			content[12] = 6  .......... power
			content[13] = 1  .......... power status
			content[14] = 7  .......... temperature
			content[15] = 26  .......... temperature value
			content[16] = 8  .......... wind force
			content[17] = 1  .......... wind force value
		 * but the server will not send all of the data, the data only contains function enabled
		 * */
		for (int i = DEFAULT_CONTENT_START_POSITION, count = 0; count < length; i++, count+=2) {
			switch (content[i])
			{
			case STATUS_REFRIGERATION:
				status[STATUS_REFRIGERATION] = content[++i];
				break;
			case STATUS_HEATING:
				status[STATUS_HEATING] = content[++i];
				break;
			case STATUS_AERATION:
				status[STATUS_AERATION] = content[++i];
				break;
			case STATUS_AREFACTION:
				status[STATUS_AREFACTION] = content[++i];
				break;
			case STATUS_WIND_SWING:
				status[STATUS_WIND_SWING] = content[++i];
				break;
			case STATUS_AUTO:
				status[STATUS_AUTO] = content[++i];
				break;
			case STATUS_POWER:
				status[STATUS_POWER] = content[++i];
				break;
			case STATUS_TEMPERATURE:
				status[STATUS_TEMPERATURE] = content[++i];
				break;
			case STATUS_WIND_FORCE:
				status[STATUS_WIND_FORCE] = content[++i];
				break;
			default:
				throw new IllegalArgumentException("Unknown command:" + content[i]);
			}
		}
		for (int i = 0; i < STATUS_COUNT; i++) {
			System.out.println(STATUS_NAMES[i] + " is " + status[i]);
		}
		sendConditionerCommandToPort(status);
	}
	
	private void sendFanCommandToPort(int button) {
		try {
			String command = "";
			switch (button) {
			case BUTTON_WIND_FORCE:
				command = "09030C871B020C001B02";
				break;
			case BUTTON_POWER:
				command = "09030C871B080C001B08";
				break;
			case BUTTON_WIND_SWING:
				command = "09030C879B000C009B00";
				break;
			case BUTTON_SCHEDULE:
				command = "09030C871B010C001B01";
				break;

			default:
				break;
			}
			System.out.println(BUTTON_NAMES[button] + " cmd is " + command);
			if (!command.equals("")) {
				OutputStream outputStream = mSerialPort.openOutputStream();
				outputStream.write(hexStringToByteArray(command));
				outputStream.flush();
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	private void parseFanCommand(byte[] content, int length) {
		if (length != 1) {
			System.out.println("Command length illegal, it should be 1 but now is " + length);
			throw new IllegalArgumentException("Command length illegal, it should be 1 but now is " + length);
		}
		int pressedButton = content[DEFAULT_CONTENT_START_POSITION];
		if (pressedButton >= BUTTON_WIND_FORCE && pressedButton < BUTTON_COUNT) {
			System.out.println(BUTTON_NAMES[pressedButton] + " is pressed");
			sendFanCommandToPort(pressedButton);
		} else {
			System.out.println("no button is pressed");
		}
	}
	
	private void parseReceivedContent(byte[] content) {
		int cmd = byteArrayToInt(content, DEFAULT_SIZE_LENGTH);
		int cmdLength = byteArrayToInt(content, DEFAULT_HEADER_LENGTH);
		System.out.println("cmd = " + CMD_NAMES[cmd] + ", cmdlen = " + cmdLength);
		switch (cmd)
		{
		case CMD_NULL:
			//not handle
			break;
		case CMD_AIR_CONDITIONER:
			parseConditionerCommand(content, cmdLength);
			break;
		case CMD_FAN:
			parseFanCommand(content, cmdLength);
			break;
		default:
			break;
		}
	}

	public void run() {
		// TODO Auto-generated method stub
		System.out.println("start run");
		Socket socket = null;
		try {
			byte[] receivedContent = new byte[100];
			socket = new Socket(REMOTE_SERVER_URL, REMOTE_SERVER_PORT);
			for (int i = CMD_AIR_CONDITIONER; i < CMD_COUNT; i++) {
				byte[] sendContent = getSendContent(i);
				System.out.println("get " + CMD_NAMES[i] + " content");
				OutputStream socketOutputStream = socket.getOutputStream();
				socketOutputStream.write(sendContent);
				socketOutputStream.flush();
				InputStream socketInputStream = socket.getInputStream();
				socketInputStream.read(receivedContent);
				parseReceivedContent(receivedContent);
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			System.out.println("socket IOException:" + e.getMessage());
			e.printStackTrace();
		} finally {
			try {
				if (socket != null) {
				socket.close();
				socket = null;
				}
			} catch (IOException e) {
				// TODO Auto-generated catch block
				System.out.println("finally socket IOException");
				e.printStackTrace();
			}
		}
		
		//set timer to get command continuously
		mTimer.cancel();
		mTimer = new Timer();
		mTimer.schedule(new TimerTask() {
			public void run() {
				// TODO Auto-generated method stub
				getCommand();
			}
		}, DEFAULT_TIME_INTERVAL);
	}	
}