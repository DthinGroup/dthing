import java.io.IOException;

import iot.oem.lcd.LCD;
import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;


public class LCDDriver extends Applet {
    private static boolean allowRunning = true;
    public LCDDriver() {
        // TODO Auto-generated constructor stub
    }

    public void cleanup() {
        // TODO Auto-generated method stub
        allowRunning = false;
    }

    public void processEvent(Event arg0) {
        // TODO Auto-generated method stub

    }

    public void startup() {
        new Thread() {
            public void run() {
                LCD lcdObj = LCD.getInstance();
                try {
                    lcdObj.setBackgroundStatus(true);
                } catch (IOException e) {
                    System.out.println(e.getMessage());
                }
                while(allowRunning) {
                    try {
                        Thread.sleep(2000);
                    } catch (InterruptedException e) {
                        System.out.println(e.getMessage());
                    }
                }
                try {
                    lcdObj.setBackgroundStatus(false);
                } catch (IOException e) {
                    System.out.println(e.getMessage());
                }
                notifyDestroyed();
            }
        }.start();
    }

}
