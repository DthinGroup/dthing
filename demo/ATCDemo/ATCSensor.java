import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;
import java.io.IOException;

import jp.co.cmcc.atcommand.*;

public class ATCSensor extends Applet {
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";
    private static boolean allowLogPrint = true;

    public ATCSensor() {
      // TODO Auto-generated constructor stub
    }

    public void cleanup() {
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        //STEP-1: Create ATCommandConnection
        ATCommandConnection conn = ATCommandConnection.getInstance();
            //STEP-2: Send AT comand: AT<CR> //Check whether AT command works
        String atCommand = new String("AT");
        String dialCommand = new String("ATD10086");
        String response = new String("Failed");
        String dialResponse = new String("Failed");

        try {
          response = conn.send(atCommand);

          //STEP-3: Received: OK
          int result = response.compareTo(new String("OK"));
          System.out.println("AT test result is " + result);

          dialResponse = conn.send(dialCommand);
          System.out.println("ATD response is " + dialResponse);

          //STEP-4: Release ATCommandConnection
          conn.close();
        } catch (NullPointerException e) {
          // TODO Auto-generated catch block
          e.printStackTrace();
        } catch (IOException e) {
          // TODO Auto-generated catch block
          e.printStackTrace();
        }

        try {
          reportI2CInfo(atCommand, response);
          reportI2CInfo(dialCommand, dialResponse);
        } catch (IOException e) {
          // TODO Auto-generated catch block
          e.printStackTrace();
        }
    }

    private void reportI2CInfo(String cmd, String response) throws IOException
    {
        String content = "CMD:" + cmd + ";RET:" + response;
        String reportInfo = REPORT_SERVER_FORMAT + content;

        if (allowLogPrint)
        {
            System.out.println("[ATDSensor]" + content);
        }
    }
}
