import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;

import iot.oem.adc.ADCManager;

public class ADCSensor extends Applet {
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=qt&parmInfo=";
    private static boolean allowRunning = true;
    private static int port = 0;

    public ADCSensor() {
      // TODO Auto-generated constructor stub
    }

    public void cleanup() {
        allowRunning = false;
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        new Thread() {
            private ADCManager manager = null;
            private int result = -1;
            private int adcValue = 0;
            private boolean allowLogPrint = false;

            public void run() {
                System.out.println("[ADCDemo]Start ADC sensor test");

                do {
                    manager = ADCManager.getInstance();

                    if (manager == null) break;

                    try {
                        result = manager.read(port);
                        adcValue = convert(result);
                        System.out.println("[ADCDemo] read channelID[" + port + "] with result[" + adcValue + "]");
                    } catch (IllegalArgumentException e) {
                        System.out.println("ADCDemo IllegalArgumentException:" + e);
                    } catch (IOException e) {
                        System.out.println("ADCDemo IOException:" + e);
                    }

                    try {
                        reportADCInfo(adcValue, port);
                    } catch (IOException e2) {
                        System.out.println("IOException: " + e2);
                    }

                    try {
                        Thread.sleep(5000L);
                    } catch (InterruptedException e) {
                        System.out.println("InterruptedException: " + e);
                    }
                }while(allowRunning);

                try {
                    manager.destroy();
                } catch (IOException e1) {
                    System.out.println("IOException: " + e1);
                }
                System.out.println("[ADCDemo] End ADC sensor test");
                notifyDestroyed();
            }

            private int convert(int value)
            {
                int zero_g = 512000;
                int scale = 1023;
                int result = 0;
                result = (value * 1000 - zero_g) / scale;
                return result;
            }

            private void reportADCInfo(int value, int cid) throws IOException
            {
                String content = "ADC:%20" + value + "%20Channel:%20" + cid;
                String reportInfo = REPORT_SERVER_FORMAT + content;

                if (allowLogPrint)
                {
                    System.out.println("[ADCSensor]" + content);
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