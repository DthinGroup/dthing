import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;

import iot.oem.kpd.RawKeyPdDriver;
import iot.oem.kpd.RawKeyPdListener;
import iot.oem.kpd.NanoDriverException;

class MyKpdListener implements RawKeyPdListener {
    private static boolean allowLogPrint = true;
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";

    public void keyStateChanged(long arg0) {
        try {
            reportTestInfo("Key " + arg0 + "detected");
        } catch (IOException e) {
            // TODO Auto-generated catch block
            log("IOException:" + e);
        }
    }

    private void reportTestInfo(String msg) throws IOException {
        String reportInfo = REPORT_SERVER_FORMAT + msg.replace(' ', '.');

        if (allowLogPrint)
        {
            log(msg);
        }

        URL url = new URL(reportInfo);
        HttpURLConnection httpConn = (HttpURLConnection)url.openConnection();
        httpConn.setRequestMethod(HttpURLConnection.POST);
        InputStream dis = httpConn.getInputStream();
        dis.close();
        httpConn.disconnect();
    }

    private void log(String msg)
    {
        System.out.println("[Keypad]" + msg);
    }
}

public class Keypad extends Applet {
    private static boolean allowLogPrint = true;
    private static boolean allowWaiting = true;
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";
    MyKpdListener listener = new MyKpdListener();

    public Keypad() {
      // TODO Auto-generated constructor stub
    }

    public void cleanup() {
        try {
            allowWaiting = false;
            RawKeyPdDriver.close();
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        new Thread() {
            public void run() {
                try {
                    RawKeyPdDriver.setListener(listener);
                    RawKeyPdDriver.initialize();
                    reportTestInfo("Waiting for key event...");
                    while(allowWaiting)
                    {
                        //Waiting for key event
                    }
                } catch (IOException e) {
                    log("IOException:" + e);
                } catch (NanoDriverException e) {
                    log("NanoDriverException:" + e);
                }
            }

            private void reportTestInfo(String msg) throws IOException {
                String reportInfo = REPORT_SERVER_FORMAT + msg.replace(' ', '.');

                if (allowLogPrint)
                {
                    log(msg);
                    return;
                }

                URL url = new URL(reportInfo);
                HttpURLConnection httpConn = (HttpURLConnection)url.openConnection();
                httpConn.setRequestMethod(HttpURLConnection.POST);
                InputStream dis = httpConn.getInputStream();
                dis.close();
                httpConn.disconnect();
            }

            private void log(String msg)
            {
                System.out.println("[Keypad]" + msg);
            }
        }.start();
    }
}
