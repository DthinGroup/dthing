import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;

import iot.oem.gpio.Gpio;

public class BuzzerSensor extends Applet {
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";
    private int gpioId = 19;
    private static boolean allowLogPrint = true;
    private static boolean allowRunning = true;

    public BuzzerSensor() {
      // TODO Auto-generated constructor stub
    }

    public void cleanup() {
        allowRunning = false;
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        new Thread(){
            public void run()
            {
                Gpio gpio = null;
                int count = 50;

                try {
                    reportBuzzerInfo("open buzzer gpio " + gpioId);
                    gpio = new Gpio(gpioId);

                    while ((count > 0) && allowRunning)
                    {
                        Thread.sleep(1000);
                        gpio.setCurrentMode(0);

                        if ((count % 2) == 0)
                        {
                            gpio.write(false);
                            reportBuzzerInfo("false");
                        }
                        else
                        {
                            gpio.write(true);
                            reportBuzzerInfo("true");
                        }
                        count--;
                    }

                    gpio.destroy();
                    reportBuzzerInfo("close buzzer gpio " + gpioId);
                } catch (IllegalArgumentException e) {
                    log("IllegalArgumentException:" + e);
                } catch (IOException e) {
                    log("IOException:" + e);
                } catch (InterruptedException e) {
                    log("InterruptedException:" + e);
                }
                notifyDestroyed();
            }
        }.start();
    }

    private void log(String s) {
        System.out.println("[Gpio Test]" + s);
    }

    private void reportBuzzerInfo(String value) throws IOException
    {
        String content = "Buzzer%20value%20changed%20to%20" + value;
        String reportInfo = REPORT_SERVER_FORMAT + content;

        if (allowLogPrint)
        {
            System.out.println("[BuzzerSensor]" + content);
        }

        URL url = new URL(reportInfo);
        HttpURLConnection httpConn = (HttpURLConnection)url.openConnection();
        httpConn.setRequestMethod(HttpURLConnection.POST);
        InputStream dis = httpConn.getInputStream();
        dis.close();
        httpConn.disconnect();
    }
}

