import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;

import iot.oem.gpio.Gpio;

public class GpioTest extends Applet
{
    private int vibraId = 0;
    private static boolean allowRunning = true;

    public GpioTest()
    {
        vibraId = 0;
    }

    public void cleanup() {
        allowRunning = false;
    }

    public void processEvent(Event paramEvent) {
    }


    public void startup() {
        boolean test = false;
        int gpioid = 0;
        try
        {
            while ((gpioid < 61) && allowRunning)
            {
                Gpio gpio = new Gpio(gpioid);
                for (int i = 0; i < 5; i++)
                {
                    Thread.sleep(1000);
                    gpio.setCurrentMode(0);
                    if (test)
                    {
                        log("read true write false:gpio="+gpioid);
                        gpio.write(false);
                        test = false;
                    } else
                    {
                        log("read false write true:gpio="+gpioid);
                        gpio.write(true);
                        test = true;
                    }
                }
                //TODO: destroy gpio instance
                gpioid++;
            }
            notifyDestroyed();
        }
        catch (IllegalArgumentException illegalargumentexception)
        {
            illegalargumentexception.printStackTrace();
        }
        catch (IOException ioexception)
        {
            ioexception.printStackTrace();
        }
        catch (InterruptedException interruptedexception)
        {
            interruptedexception.printStackTrace();
        }
    }

    private void log(String s)
    {
        System.out.println("[Gpio Test]" + s);
    }
}

