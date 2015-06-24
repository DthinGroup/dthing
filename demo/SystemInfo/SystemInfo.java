import java.io.IOException;
import java.io.InputStream;
import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.util.Random;

import javax.microedition.io.Connector;
import javax.microedition.io.file.FileConnection;

import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;


public class SystemInfo extends Applet {
    private static boolean debug = true;
    private final static String SERVER_PREFIX =
    "http://42.121.18.62:8080/dthing" +
    "/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";

    public SystemInfo() {
      // TODO Auto-generated constructor stub
    }

    public void cleanup() {
      // TODO Auto-generated method stub

    }

    public void processEvent(Event arg0) {
      // TODO Auto-generated method stub

    }

    public void startup() {
      // TODO Auto-generated method stub
      getSystemInfo();
    }

    private void postMessageToServer(String msg) {
        try {
            if (debug) {
                System.out.println("postMessageToServer - msg = \n\t" + msg);
            }

            URL url = new URL(SERVER_PREFIX + msg);
            HttpURLConnection httpConn = (HttpURLConnection)url.openConnection();
            httpConn.setRequestMethod(HttpURLConnection.POST);
            InputStream dis = httpConn.getInputStream();
            dis.close();
            httpConn.disconnect();
        } catch (IOException e) {
            if (debug) {
                System.out.println("postMessageToServer IOException");
            }
            e.printStackTrace();
        }
    }

    void getSystemInfo() {
        long free = 0;
        long total = 0;
        long used = 0;
        String msg = null;

        /* memory usage */
        total = Runtime.getRuntime().totalMemory();
        free = Runtime.getRuntime().freeMemory();
        used = total - free;
        if (debug) {
            System.out.println("Memory used(" + (used >> 10) + "K), " +
                "free(" + (free >> 10) + "K)");
        }
        msg = "Memory%20used(" + (used >> 10) + "K),%20" +
            "free(" + (free >> 10) + "K)";
        postMessageToServer(msg);

        /* file system(device storage) usage */
        FileConnection fconn;
        try {
            fconn = (FileConnection) Connector.open("file:///D:/");
            used = fconn.usedSize();
            free = fconn.availableSize();
        } catch (IOException e) {
            if (debug) {
              System.out.println("File Connetion open failure!");
            }
            e.printStackTrace();
        }
        if (debug) {
            System.out.println("FS Flash used(" + (used >> 10) + "K), " +
                "free(" + (free >> 10) + "K)");
        }
        msg = "FS%20Flash%20used(" + (used >> 10) + "K),%20" +
            "free(" + (free >> 10) + "K)";
        postMessageToServer(msg);

        /* count used time while cycle 100 times of add operation. */
        long startpoint = 0; //unit is ms
        long endpoint = 0; //unit is ms
        Random random = new Random(0xCAFE);

        total = 0;
        startpoint = System.currentTimeMillis();
        for (int i = 0; i < 100; i++) {
            total += random.nextInt(0xFFFFF);
        }
        endpoint = System.currentTimeMillis();
        if (debug) {
            System.out.println("Cycle 100 times takes " + (startpoint-endpoint) + "ms");
        }
        msg = "Cycle%20100%20times%20takes%20" + (startpoint - endpoint) + "ms";
        postMessageToServer(msg);
    }
}