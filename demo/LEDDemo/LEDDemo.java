import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;

import iot.oem.gpio.Gpio;

public class LEDDemo extends Applet
{
    private int vibraId = 20;

    public LEDDemo()
    {
        vibraId = 20;
    }

    public void cleanup() {
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        boolean test = false;
        try
        {
            Gpio gpio = new Gpio(vibraId);
            for (int i = 0; i < 1000; i++)
            {
                Thread.sleep(1000);
                gpio.setCurrentMode(0);

                if (test)
                {
                    log("read true write false");
                    gpio.write(false);
                    test = false;
                }
                else
                {
                    log("read false write true");
                    gpio.write(true);
                    test = true;
                }
            }
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
