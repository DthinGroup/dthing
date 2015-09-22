import java.io.IOException;

import iot.oem.lcd.LCD;
import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;


public class LCDDriver extends Applet {
    private static LCD lcdObj = null;
    public LCDDriver() {
        // TODO Auto-generated constructor stub
    }

    public void cleanup() {
      System.out.println("[lcd] cleanup");
        try {
            lcdObj.setBackgroundStatus(false);
        } catch (IOException e) {
            System.out.println(e.getMessage());
        }
    }

    public void processEvent(Event arg0) {
        // TODO Auto-generated method stub

    }

    public void startup() {
        lcdObj = LCD.getInstance();
        System.out.println("[lcd] get instance " + lcdObj);
        try {
            lcdObj.setBackgroundStatus(true);
        } catch (IOException e) {
            System.out.println(e.getMessage());
        }
        System.out.println("[lcd] end of startup");
    }

}

