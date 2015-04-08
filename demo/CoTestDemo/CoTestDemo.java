/**
 * CoTestDemo is used for testing multiple tests in one Demo.
 */
import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;


public class CoTestDemo extends Applet {
  private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";

    public CoTestDemo() {
      // TODO Auto-generated constructor stub
    }

    public void cleanup() {
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        new Thread() {
            public void run() {
                try {
                    log("Start testing multi-demos. \n");

                    printLog(true, "PWMSensor");
                    PWMSensor pwmSensor = new PWMSensor();
                    pwmSensor.startTest();
                    printLog(false, "PWMSensor");

                    printLog(true, "LEDDemo");
                    LEDDemo ledDemo = new LEDDemo();
                    ledDemo.startTest();
                    printLog(false, "LEDDemo");

                    printLog(true, "BuzzerSensor");
                    BuzzerSensor buzSensor = new BuzzerSensor();
                    buzSensor.startTest();
                    printLog(false, "BuzzerSensor");

                    log("End testing multi-demos.");
                } catch (IOException e) {
                    // TODO Auto-generated catch block
                    log("exception happened in line54!");
                    e.printStackTrace();
                }
            }

            private void printLog(boolean isStart, String testName) throws IOException {
                String msg = (isStart?"Start":"End") + " testing " + testName;
                reportCoTestInfo("CoTestDemo", msg);
            }

            private void reportCoTestInfo(String name, String msg) throws IOException  {
                String content = name + ":" + msg;
                String reportInfo = REPORT_SERVER_FORMAT + content;

                if(true)
                {
                    log(content);
                    //return;
                }

                URL url = new URL(reportInfo);
                HttpURLConnection httpConn = (HttpURLConnection)url.openConnection();
                httpConn.setRequestMethod(HttpURLConnection.POST);
                InputStream dis = httpConn.getInputStream();
                dis.close();
                httpConn.disconnect();
            }

            private void log(String s)  {
                System.out.println("[CoTestDemo]" + s);
            }
        }.start();
    }
}
