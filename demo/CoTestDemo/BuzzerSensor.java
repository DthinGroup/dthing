
import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;

import iot.oem.gpio.Gpio;

public class BuzzerSensor {
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";
    private int gpioId = 19;
    private static boolean allowLogPrint = true;

    public BuzzerSensor() {
      // TODO Auto-generated constructor stub
    }

    public void startTest() {
        Gpio gpio = null;
        int count = 16;

        try {
            reportBuzzerInfo("open buzzer gpio " + gpioId);
            gpio = new Gpio(gpioId);

            while(count > 0)
            {
                Thread.sleep(1000);
                gpio.setCurrentMode(0);
                System.out.println("count = " + count);

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
    }

    private void log(String s) {
        System.out.println("[BuzzerSensor] " + s);
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

