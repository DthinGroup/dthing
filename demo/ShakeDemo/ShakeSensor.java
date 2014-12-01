import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;

import iot.oem.gpio.Gpio;
import iot.oem.gpio.GpioInterruptListener;

class MyListener implements GpioInterruptListener {

    public void onInterrupt(Gpio arg0, int arg1, boolean arg2) {
        // TODO Auto-generated method stub
        System.out.println("Response to interrupt with Gpio:" + arg0 + " arg1:" + arg1 + " arg2:" + arg2);
    }
}

public class ShakeSensor extends Applet {
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=jsd&parmInfo=";
    private int vibraId = 8;
    private static boolean allowLogPrint = true;

    public ShakeSensor() {
      // TODO Auto-generated constructor stub
    }

    public void cleanup() {
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        new Thread(){
            public void run()
            {
                Gpio gpio = null;
                MyListener listener = null;
                String oldValue = null;
                String newValue = null;
                int count = 10000;

                try {
                    log("open vibra gpio " + vibraId);
                    gpio = new Gpio(vibraId);
                    Thread.sleep(2000);
                    listener = new MyListener();
                    startVibraTest(gpio, listener);
                    Thread.sleep(2000);
                } catch (IllegalArgumentException e) {
                    log("IllegalArgumentException:" + e);
                } catch (IOException e) {
                    log("IOException:" + e);
                } catch (InterruptedException e) {
                    log("InterruptedException:" + e);
                }

                while(count > 0)
                {
                    newValue = System.getProperty("yarlung.vibra");

                    if ((newValue != null) && (newValue != oldValue))
                    {
                        oldValue = newValue;
                        log("shake value is changed to " + newValue);
                        try {
                            reportShakeInfo(newValue);
                        } catch (IOException e) {
                            log("reportShakeInfo IOException:" + e);
                        }
                        count--;
                    }
                }

                stopVibraTest(gpio, listener);
            }

        }.start();
    }

    private void log(String s) {
        System.out.println("[Gpio Test]" + s);
    }

    private void startVibraTest(Gpio gpio, GpioInterruptListener listener)
    {
        try {
            log("set vibra gpio to wirte mode");
            gpio.setCurrentMode(Gpio.READ_MODE);
            gpio.registerInterruptListener(listener, Gpio.INTERRUPT_TYPE_LOW_TO_HIGH);
            log("unRegisterInterruptListener");
        } catch (IllegalArgumentException e) {
            log("IllegalArgumentException:" + e);
        } catch (IOException e) {
            log("IOException:" + e);
        }
    }

    private void stopVibraTest(Gpio gpio, GpioInterruptListener listener)
    {
        try {
            log("unRegisterInterruptListener");
            gpio.unRegisterInterruptListener(listener);
            log("destroy vibra gpio");
            gpio.destroy();
        } catch (IllegalArgumentException e) {
            log("IllegalArgumentException:" + e);
        } catch (IOException e) {
            log("IOException:" + e);
        }
    }

    private void reportShakeInfo(String value) throws IOException
    {
        String content = "Shake%20value%20changed%20to%20" + value;
        String reportInfo = REPORT_SERVER_FORMAT + content;

        if (allowLogPrint)
        {
            System.out.println("[ShakeSensor]" + content);
        }

        URL url = new URL(reportInfo);
        HttpURLConnection httpConn = (HttpURLConnection)url.openConnection();
        httpConn.setRequestMethod(HttpURLConnection.POST);
        InputStream dis = httpConn.getInputStream();
        dis.close();
        httpConn.disconnect();
    }
}
