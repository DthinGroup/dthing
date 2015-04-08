
import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;

import iot.oem.gpio.Gpio;

public class LEDDemo {
    private int vibraId = 20;

    public LEDDemo()
    {
        vibraId = 20;
    }

    public void startTest() {
        boolean test = false;
        try
        {
            Gpio gpio = new Gpio(vibraId);
            log("open led gpio " + vibraId);
            for (int i = 0; i < 16; i++)
            {
                Thread.sleep(1000);
                gpio.setCurrentMode(0);
                log("i = " + i);

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

            gpio.destroy();
            log("close led gpio " + vibraId);
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
        System.out.println("[LEDDemo]" + s);
    }
}

