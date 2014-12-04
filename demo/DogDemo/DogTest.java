import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;

import jp.co.cmcc.watchdog.WatchDogManager;

public class DogTest extends Applet {
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";
    private boolean allowLogPrint = true;

    public DogTest() {
      // TODO Auto-generated constructor stub
    }

    public void cleanup() {
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        // TODO Auto-generated method stub
        WatchDogManager dog = WatchDogManager.getInstance();
        try {
            reportTestInfo("Dog", "begin to kick");
            for (int i = 0; i < 1000000; i++)
            {
                if (i % 1000 == 0)
                {
                    reportTestInfo("Dog", "kick at " + i);
                    dog.kick(1000);
                }
            }
            reportTestInfo("Dog", "end of kick");
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    private void reportTestInfo(String name, String msg) throws IOException
    {
        String content = name + ":" + msg;
        String reportInfo = REPORT_SERVER_FORMAT + content;

        if (allowLogPrint)
        {
            System.out.println("[" + name + "]" + content);
        }

        URL url = new URL(reportInfo);
        HttpURLConnection httpConn = (HttpURLConnection)url.openConnection();
        httpConn.setRequestMethod(HttpURLConnection.POST);
        InputStream dis = httpConn.getInputStream();
        dis.close();
        httpConn.disconnect();
    }
}
