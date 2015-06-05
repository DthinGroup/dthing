package com.yarlungsoft.ZigBeeDemo;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;

import javax.microedition.io.Connector;
import javax.microedition.io.HttpConnection;
import javax.microedition.midlet.MIDlet;
import javax.microedition.midlet.MIDletStateChangeException;

import jp.co.aplix.io.CommConnectionImpl;

public class ZigBeeDemo extends MIDlet implements Runnable
{
    private final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=pm25&parmInfo=";
    private boolean allowLogPrint = true;
    private GetServerCommand mGetServerCommand;
    private final int READ_BUFFER_LENGTH = 100;
    private final String REGEX = ",";

    protected void destroyApp(boolean arg0) throws MIDletStateChangeException
    {
    	if (mGetServerCommand != null) {
    		mGetServerCommand.destroyApp(true);
    	}
    }

    protected void pauseApp()
    {
    	if (mGetServerCommand != null) {
    		mGetServerCommand.pauseApp();
    	}
    }

    protected void startApp() throws MIDletStateChangeException
    {
    	System.out.println("ZigBeeDemo startApp");
        new Thread(this).start();
        mGetServerCommand = new GetServerCommand();
        mGetServerCommand.startApp();
    }
    
	public void run() {
		// TODO Auto-generated method stub
        try {
            byte[] buf = new byte[READ_BUFFER_LENGTH];

            int readSize;
            do {
                CommConnectionImpl comm = CommConnectionImpl.getComInstance(1);
                InputStream is = comm.openInputStream();
                try {
                    Thread.sleep(1000L);
                } catch (InterruptedException e) {
                     System.out.println("InterruptedException:" + e);
                }

                readSize = is.read(buf, 0, READ_BUFFER_LENGTH);
                String readString = new String(buf);
                readString = readString.trim();
                //re-organize string
                readString = convertEscapedChar(readString);
                readString = join(split(readString, REGEX), REGEX);

                if (readString.length() > 0) {
                	reportTestInfo("COM", "\"" + readString + "\"");
                }
                comm.close();
            } while (readSize >= 0);
        } catch (IOException e) {
            System.out.println("IOException:" + e);
        }		
	}
    
    private String convertEscapedChar(String original)
    {
        String escaped = "";
        char[] ctest = original.toCharArray();
        for (int i = 0; i < ctest.length; i++)
        {
            if ((ctest[i] != '\r') && (ctest[i] != '\n'))
            {
                escaped = escaped + ctest[i];
            }
            else
            {
                escaped = escaped + '.';
            }
        }
        return escaped;
    }
    
    private String[] split(String string, String regex) {
    	String[] arrays = new String[string.length() / 2];
    	String[] result = null;
    	int count = 0;
    	int index = 0;
    	for (int i = 0; i < string.length(); i++) {
    		if (string.charAt(i) == regex.charAt(0)) {
    			String temp = string.substring(index, i);
    			//if string not contain ":", ignore it
    			if (temp.indexOf(":") == -1) {
    				continue;
    			}
    			arrays[count] = temp;
    			count++;
    			index = i + 1;
    		}
    	}
    	
    	if (count < 3) {
    		result = new String[count];
    		for (int i = 0; i < count; i++) {
    			result[i] = arrays[i];
    		}
    	} else {
	    	//remove the first and the last
	    	result = new String[count - 2];
	    	for (int i = 0; i < count - 2; i++) {
	    		result[i] = arrays[i + 1];
	    	}
    	}
    	return result;
    }
    
    private String join(String[] arrays, String regex) {
    	String result = "";
    	if (arrays.length == 1) {
    		result = arrays[0];
    	} else if (arrays.length > 1) {
			for (int i = 0; i < arrays.length; i++) {
				result += arrays[i] + regex;
			}
		}
    	
    	/*delete the last ,*/
    	if (result.length() > 1) {
    		StringBuilder sb = new StringBuilder(result);
    		sb.deleteCharAt(result.length() - 1);
    		result = sb.toString();
    	}
    	
    	return result;
    }

    private void reportTestInfo(String name, String msg) throws IOException {
        String content = name + ":" + msg.replace(' ', '.');
        String reportInfo = REPORT_SERVER_FORMAT + msg.replace(' ', '.');
        if (allowLogPrint)
        {
          System.out.println("[" + name + "]" + content);
        }

        HttpConnection httpConn = (HttpConnection)Connector.open(reportInfo);
        httpConn.setRequestMethod("POST");
        DataInputStream dis = httpConn.openDataInputStream();
        dis.close();
        httpConn.close();
    }
}