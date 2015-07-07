import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;

import iot.oem.comm.CommConnectionImpl;


public class ZigbeeDemo extends Applet {
    private final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=pm25&parmInfo=";
    private boolean allowLogPrint = true;
    private GetServerCommand mGetServerCommand;
    private final int READ_BUFFER_LENGTH = 100;
    private final String REGEX = ",";

	public void cleanup() {
    	if (mGetServerCommand != null) {
		    mGetServerCommand.destroyApp(true);
    	}
	}

	public void processEvent(Event arg0) {
		// TODO Auto-generated method stub

	}

	public void startup() {
    	System.out.println("ZigBeeDemo startApp");
        new Thread(){
    		public void run() {
    			// TODO Auto-generated method stub
                CommConnectionImpl comm = CommConnectionImpl.getComInstance(0);
    	        try {
    	            byte[] buf = new byte[READ_BUFFER_LENGTH];
                    InputStream is = comm.openInputStream();
    	            int readSize;
                    
    	            do {
    	                try {
    	                    Thread.sleep(1000L);
    	                } catch (InterruptedException e) {
    	                     System.out.println("InterruptedException:" + e);
    	                }

    	                readSize = is.read(buf, 0, READ_BUFFER_LENGTH);
                        if (readSize < 0)  {
                            reportTestInfo("COM", "exit when readSize is less than 0");
                            break;
                        }
    	                String readString = new String(buf);
    	                readString = readString.trim();
    	                //re-organize string
    	                readString = convertEscapedChar(readString);
    	                readString = join(split(readString, REGEX), REGEX);

    	                if (readString.length() > 0) {
    	                	reportTestInfo("COM", "\"" + readString + "\"");
                            //System.out.println("COM\"" + readString + "\"");
    	                }
                    } while (true);
                    //comm.close();
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

                URL url = new URL(reportInfo);
                HttpURLConnection httpConn = (HttpURLConnection)url.openConnection();
                httpConn.setRequestMethod(HttpURLConnection.POST);
                InputStream dis = httpConn.getInputStream();
                dis.close();
                httpConn.disconnect();
    	    }
        }.start();
        mGetServerCommand = new GetServerCommand();
        mGetServerCommand.startApp();
	}
}
