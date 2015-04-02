// Decompiled by Jad v1.5.8e2. Copyright 2001 Pavel Kouznetsov.
// Jad home page: http://kpdus.tripod.com/jad.html
// Decompiler options: packimports(3) fieldsfirst ansi space
// Source File Name:   BuzzerDemo.java

import iot.oem.gpio.Gpio;

import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;

public class BuzzerDemo {
    private int vibraId;

    public BuzzerDemo()
    {
        vibraId = 19;
    }

    public void startTest()
    {
        boolean test = false;
        try
        {
            Gpio gpio = new Gpio(vibraId);
            for (int i = 0; i < 16; i++)
            {
                Thread.sleep(1000);
                log("i = "+i);
                gpio.setCurrentMode(0);
                if (test)
                {
                    log("read true write false");
                    gpio.write(false);
                    test = false;
                } else
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
        System.out.println("[Gpio Test] " + s);
    }
}
