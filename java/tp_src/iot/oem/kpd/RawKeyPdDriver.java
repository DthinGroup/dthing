/*
 * Copyright 2014 Yarlung Soft. All rights reserved.
 */

package iot.oem.kpd;

import java.lang.AsyncIO;

public class RawKeyPdDriver extends Thread {
    private static boolean isEnabled = false;
    private static RawKeyPdListener kpdListener;
	private static RawKeyPdDriver driver = null;

    private RawKeyPdDriver() {
    }

    public static void initialize() throws NanoDriverException {
        if (driver == null)
        {
            if (init0() < 0)
            {
                throw new NanoDriverException("failed to initialize native keypad drvier");
            }
            driver = new RawKeyPdDriver();
            isEnabled = true;
            driver.start();
        }
    }

    private static native int init0();

    public void run() {
        long detectedKey = 0;

        while(isEnabled)
        {
            while (AsyncIO.loop())
            {
              detectedKey = getKey0();
            }

            if(kpdListener != null)
            {
                kpdListener.keyStateChanged(detectedKey);
            }
        }
    }

    private static native long getKey0();

    public static void setListener(RawKeyPdListener listener) {
        kpdListener = listener;
    }

    public static void close() throws InterruptedException {
        if (close0() < 0)
        {
            throw new InterruptedException("failed to close native keypad driver");
        }
        kpdListener = null;
        isEnabled = false;
		driver = null;
    }

    private static native int close0();
}
