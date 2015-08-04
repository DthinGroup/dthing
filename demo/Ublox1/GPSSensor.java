import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;

import iot.oem.gpio.Gpio;
import iot.oem.comm.CommConnectionImpl;

public class GPSSensor extends Applet
{
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";
    private static boolean allowLogPrint = true;
    private static boolean allowRunning = true;

    public void cleanup() {
        allowRunning = false;
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        new Thread() {
            public void run() {
                try {
                    Gpio ldo = new Gpio(7);
                    ldo.setCurrentMode(Gpio.WRITE_MODE);
                    ldo.write(true);
                    reportTestInfo("GPSCOM", "pull GPIO 7 to high");
                } catch (IllegalArgumentException e1) {
                    System.out.println("IllegalArgumentException:" + e1);
                } catch (IOException e1) {
                    System.out.println("IOException:" + e1);
                }

                CommConnectionImpl gpsComm = CommConnectionImpl.getComInstance(1, 9600);
                try {
                    byte[] buf = new byte[128];

                    InputStream is = gpsComm.openInputStream();
                    int readSize;
                    do {
                        try {
                            Thread.sleep(10000L);
                        } catch (InterruptedException e) {
                            System.out.println("InterruptedException:" + e);
                        }

                        readSize = is.read(buf, 0, 128);

                        if (readSize < 0)
                        {
                            reportTestInfo("GPSCOM", "exit when readSize is less than 0");
                            break;
                        }

                        String readString = new String(buf);

                        reportTestInfo("GPSCOM", "read:" + convertEscapedChar(readString));
                    } while (allowRunning);
                    gpsComm.close();
                    notifyDestroyed();
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

            private void reportTestInfo(String name, String msg) throws IOException {
                String content = name + ":" + msg.replace(' ', '.');
                String reportInfo = REPORT_SERVER_FORMAT + content;

                if (allowLogPrint)
                {
                  System.out.println("[" + name + "]" + content);
                  //return;
                }

                URL url = new URL(reportInfo);
                HttpURLConnection httpConn = (HttpURLConnection)url.openConnection();
                httpConn.setRequestMethod(HttpURLConnection.POST);
                InputStream dis = httpConn.getInputStream();
                dis.close();
                httpConn.disconnect();
            }
        }.start();
    }
}