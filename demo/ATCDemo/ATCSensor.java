import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;
import java.io.DataInputStream;
import java.io.IOException;

import javax.microedition.io.Connector;
import javax.microedition.io.HttpConnection;
import jp.co.aplix.atcommand.*;


public class ATCSensor extends Applet {
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";
    private static boolean allowLogOnServer = true;

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

        if (!allowLogOnServer)
        {
            System.out.println("[ATDSensor]" + content);
            return;
        }

        HttpConnection httpConn = (HttpConnection)Connector.open(reportInfo);
        httpConn.setRequestMethod(HttpConnection.POST);
        DataInputStream dis = httpConn.openDataInputStream();
        dis.close();
        httpConn.close();
    }
}
