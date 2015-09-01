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

    private static long totalMemory = 0;
    private static long gcMemory = 0;

    private static final int DefaultGPSPort = 0;
    private static final int DefaultBaudrate = 9600;
    private static final int DefaultGPSBuffer = 32;
    private static final int DefaultGCPercentage = 50;
    private static int totalReadLength = 0;
    private static CommConnectionImpl gpsComm = null;

    public void cleanup() {
        allowRunning = false;
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {

        new Thread() {
            public void run() {
                byte[] buf = null;
                int readSize = 0;
                InputStream is = null;
                totalMemory = Runtime.getRuntime().totalMemory();
                gcMemory = totalMemory * DefaultGCPercentage/ 100;

                while(allowRunning) {
                    try {
                        if (gpsComm == null) {
                            gpsComm = CommConnectionImpl.getComInstance(DefaultGPSPort, DefaultBaudrate);

                            if (gpsComm != null) {
                                Gpio ldo = new Gpio(60); //60 for board, 7 for shoe
                                ldo.setCurrentMode(Gpio.WRITE_MODE);
                                ldo.write(true);
								int status = ldo.read();
                                reportTestInfo("GPSCOM", "pull GPIO 60 to high:" + status);
                                buf = new byte[DefaultGPSBuffer];
                            } else {
                                continue;
                            };
                        }

                        if (is == null) {
                            is = gpsComm.openInputStream();
                            if (is != null) {
                                Thread.sleep(10000L);
                            } else {
                                continue;
                            }
                        }

                        readSize = is.read(buf, 0, DefaultGPSBuffer);

                        if (readSize < 0)
                        {
                            reportTestInfo("GPSCOM", "exit when readSize is less than 0");
                            break;
                        }

                        String readString = new String(buf);

                        reportTestInfo("GPSCOM", "read["+readSize+"]:" + convertEscapedChar(readString));
                        MemoryCheck();
                    } catch (IllegalArgumentException e) {
                        System.out.println("IllegalArgumentException:" + e);
                    } catch (InterruptedException e) {
                        System.out.println("InterruptedException:" + e);
                    } catch (IOException e) {
                        System.out.println("IOException:" + e);
                    }
                }

                try {
                   Gpio ldo = new Gpio(60); //60 for board, 7 for shoe
                   ldo.setCurrentMode(Gpio.WRITE_MODE);
                   ldo.write(false);
                    gpsComm.close();
                } catch (IOException e1) {
                    System.out.println("IOException:" + e1);
                }
                notifyDestroyed();
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

            public static void MemoryCheck() {
                long free =  Runtime.getRuntime().freeMemory();

                if (free < gcMemory) {
                    Runtime.getRuntime().gc();
                }
            }
        }.start();
    }
}