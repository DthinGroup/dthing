import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;
import java.io.DataInputStream;
import java.io.IOException;

import iot.oem.pwm.PWMManager;

public class PWMSensor extends Applet {
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";

    public PWMSensor() {
      // TODO Auto-generated constructor stub
    }

    public void cleanup() {
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
         new Thread() {
              private PWMManager manager = null;
              private boolean allowLogPrint = true;

              public void run() {
                int i = 0;
                int  hz = 1;
                  System.out.println("[PWMDemo]Start PWM sensor test");
                  manager = PWMManager.getInstance();

                  try {
                      manager.command(0, 0x30, 1);
                      manager.command(0, 0x31, 1);

                      for (i = 0; i < 10; i++)
                      {
                            manager.config(0, hz, i*10);

                            if (hz < 5)
                            {
                              hz++;
                            }
                            else
                            {
                              hz = 1;
                            }
                      }
                      reportPWMInfo("Test success");
                      manager.command(0, 0x30, 0);
                  } catch (IOException e) {
                        try {
                            reportPWMInfo("IOException");
                            manager.command(0, 0x30, 0);
                        } catch (IOException e1) {
                            e1.printStackTrace();
                        }
                        e.printStackTrace();
                  }

                  System.out.println("[PWMDemo] End PWM sensor test");
              }

              private void reportPWMInfo(String msg) throws IOException
              {
                  String content = "PWM:" + msg;
                  String reportInfo = REPORT_SERVER_FORMAT + content;

                  if (allowLogPrint)
                  {
                      System.out.println("[PWMSensor]" + content);
                  }
              }
          }.start();
  }
}
