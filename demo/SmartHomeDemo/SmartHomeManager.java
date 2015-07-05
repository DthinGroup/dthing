import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.http.HttpURLConnection;
import java.net.http.URL;

import iot.oem.comm.CommConnectionImpl;
import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;


public class SmartHomeManager extends Applet {
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";
    private static boolean allowLogPrint = true;
    private static boolean allowRunning = true;
    private static boolean allowIRCRunning = true;
    private static Thread dataCollectThread = null;
    private static Thread irControllerThread = null;
    private static int commRefCount = 0; // Count of threads that is using comm instance
    private static boolean commInuse = false;
    private static CommConnectionImpl comm = null;

    private static final String OPEN_CODE = "1B00A394090A20500220A8000000D0A394090A2070022000000000D0";
    private static final String CLOSE_CODE = "1B00A394010020500220A8000000B0A39401002070022000000000B0";

    public SmartHomeManager() {
      // TODO Auto-generated constructor stub
    }

    public void cleanup() {
        allowRunning = false;
        allowIRCRunning = false;
    }

    public void processEvent(Event arg0) {
      // TODO Auto-generated method stub

    }

    public void startup() {
        startDataCollectThread();
    }

    public void registerCommReference(CommConnectionImpl comm)
    {
        System.out.println("registerCommReference");
        if (comm != null)
        {
            commRefCount += 1;
        }
    }

    public void destroyCommReference(CommConnectionImpl comm) throws IOException
    {
        System.out.println("destroyCommReference");
        if (commRefCount > 0)
        {
            commRefCount--;
        }
        if (commRefCount == 0)
        {
            comm.close();
        }
    }

    public boolean allowToUseComm()
    {
        boolean result = false;

        if (!commInuse)
        {
            commInuse = true;
            result = true;
        }
        return result;
    }

    public void endToUseComm()
    {
        commInuse = false;
    }

    public void reportTestInfo(String name, String msg){
        String content = name + ":" + msg.replace(' ', '.');
        String reportInfo = REPORT_SERVER_FORMAT + content;

        if (allowLogPrint)
        {
            System.out.println("[" + name + "]" + content);
        }

		try{
        	URL url = new URL(reportInfo);
        	HttpURLConnection httpConn = (HttpURLConnection)url.openConnection();
        	httpConn.setRequestMethod(HttpURLConnection.POST);
        	InputStream dis = httpConn.getInputStream();
        	dis.close();
        	httpConn.disconnect();
    	} catch (Exception e){
    		System.out.println("reportTestInfo Exception <not throw>: " + e);
    	}
    }

    public void startIRControllerThread()
    {    	
        irControllerThread = new Thread() {
            public void run() {
                CommConnectionImpl comm = CommConnectionImpl.getComInstance(0);
                registerCommReference(comm);

                try {
                    do {
                        reportTestInfo("IRC", "startIRControllerThread");
                        while(!allowToUseComm())
                        {
                          //Waiting
                        }
                        reportTestInfo("IRC", "God bless IRControllerThread");
                        OutputStream os = comm.openOutputStream();

						byte[] HardCode = {0x33,0x32,0x32,0x35};
                        os.write(HardCode/*hex2Bytes(OPEN_CODE)*/);
                        reportTestInfo("IRC", "write:" + OPEN_CODE);
                        os.close();
                        endToUseComm();
                        Thread.sleep(1000);
                    } while(allowIRCRunning);
                    destroyCommReference(comm);
                    reportTestInfo("IRC", "End of IRC");
                } catch (IOException e) {
                      System.out.println("cc IOException:" + e);
                } catch (InterruptedException e) {
                      System.out.println("cc InterruptedException:" + e);
                }
            }

            private byte[] hex2Bytes(String src){
                byte[] res = new byte[src.length()/2];
                char[] chs = src.toCharArray();
                for(int i = 0,c = 0; i < chs.length; i+=2, c++){
                    res[c] = (byte) (Integer.parseInt(new String(chs, i, 2), 16));
                }
                return res;
            }
        };

        irControllerThread.start();
    }

    public void startDataCollectThread()
    {
        dataCollectThread = new Thread() {
            public void run() {
				int count = 0;
            	allowIRCRunning = false;
                CommConnectionImpl comm = CommConnectionImpl.getComInstance(0);
                registerCommReference(comm);
                
                try {
                    byte[] buf = new byte[128];

                    InputStream is = null;
                    int readSize;
                    do {
                    	  reportTestInfo("COM", "startDataCollectThread");
                        while(!allowToUseComm())
                        {
                            //Waiting
                        }
                        System.out.println("God bless dataCollectThread");
                        reportTestInfo("COM", "God bless dataCollectThread");
                        is = comm.openInputStream();
                        System.out.println("god is:" + is);
                        System.out.println("startDataCollectThread openInputStream, to sleep");
                        Thread.sleep(2000);
                        System.out.println("startDataCollectThread sleep 2s over A");
                        reportTestInfo("COM", "start to read data");
                        System.out.println("Ready to read 1");
                        System.out.println("Ready to read is:" + is);
                        System.out.println("Ready to read 2");                        
                        readSize = is.read(buf, 0, 28);
						System.out.println("startDataCollectThread Read Size:" + readSize);
                        if (readSize < 0)
                        {
                            reportTestInfo("COM", "exit when readSize is less than 0");
                            break;
                        }

                        String readString = new String(buf);
                        System.out.println("startDataCollectThread readString:" + readString);
                        reportTestInfo("COM", "read:" + convertEscapedChar(readString));
                        Thread.sleep(2000L);
                        count = count +1;
                        //is.close();
                        endToUseComm();
                    } while ((allowRunning) && (count < 1));
                    destroyCommReference(comm);
                    reportTestInfo("COM", "End of data collect");
                    startIRControllerThread();
                } catch (IOException e) {
                    System.out.println("aa IOException:" + e);
                } catch (InterruptedException e) {
                    System.out.println("bb InterruptedException:" + e);
                }
            }

            private String convertEscapedChar(String original)
            {
            	System.out.println("convertEscapedChar,original:" + original);
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
        };
        dataCollectThread.start();
    }
}
