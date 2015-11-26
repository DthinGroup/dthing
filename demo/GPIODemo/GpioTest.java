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
        int i = 0;
        int[] gpioids = {26,88};
        try
        {
            while ((i < gpioids.length) && allowRunning)
            {
                Gpio gpio = new Gpio(gpioids[i]);
                Thread.sleep(500);
                gpio.setCurrentMode(0);
                log("set gpio:" + gpioids[i] + " to write mode");
                gpio.write(true);
                log("write gpio:" + gpioids[i] + " to true");
                gpio.setCurrentMode(1);
                Thread.sleep(500);
                log("read gpio:" + gpioids[i] + " with value " +gpio.read());
                gpio.setCurrentMode(0);
                Thread.sleep(500);
                gpio.write(false);
                log("write gpio:" + gpioids[i] + " to false");
                gpio.setCurrentMode(1);
                Thread.sleep(500);
                log("read gpio:" + gpioids[i] + " with value " +gpio.read());
                gpio.setCurrentMode(0);
                Thread.sleep(500);
                gpio.destroy();
                log("destroy gpio:" + gpioids[i]);
                Thread.sleep(500);
                i++;
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
        System.out.println("[GPIO]" + s);
    }
}
