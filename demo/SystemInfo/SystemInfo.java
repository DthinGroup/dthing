import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.util.Random;

import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;


public class SystemInfo extends Applet {
    private static boolean debug = true;
    private static boolean allowMemoryInfo = true;
    private static boolean allowFileList = false;
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

    long getDirectorySize(File directory)
    {
        long used = 0;
        File[] fileArray = directory.listFiles();
        for (int i = 0; i < fileArray.length; i++)
        {
            if (fileArray[i].isFile())
            {
                System.out.println("list file:" + fileArray[i].getPath());
                used += fileArray[i].length();
            }
            else
            {
                System.out.println("list Directory: " + fileArray[i].getPath());
                used += getDirectorySize(fileArray[i]);
            }
        }
        return used;
    }

    void getSystemInfo() {
        long free = 0;
        long total = 0;
        long used = 0;
        long totalFS = 85 * 1024; //初始化文件系统大小
        String msg = null;

        /* memory usage */
        if (allowMemoryInfo)
        {
            total = Runtime.getRuntime().totalMemory();
            free = Runtime.getRuntime().freeMemory();
            used = total - free;

            msg = "Memory%20used(" + (used >> 10) + "K),%20" +
                "free(" + (free >> 10) + "K)";
            postMessageToServer(msg);
        }

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

        msg = "Cycle%20100%20times%20takes%20" + (endpoint - startpoint) + "ms";
        postMessageToServer(msg);

        /* count used time while cycle 1000 times of add operation. */
        random = new Random(0xCAFE);

        total = 0;
        startpoint = System.currentTimeMillis();
        for (int i = 0; i < 1000; i++) {
            total += random.nextInt(0xFFFFF);
        }
        endpoint = System.currentTimeMillis();

        msg = "Cycle%201000%20times%20takes%20" + (endpoint - startpoint) + "ms";
        postMessageToServer(msg);

        /* count used time while cycle 10000 times of add operation. */
        random = new Random(0xCAFE);

        total = 0;
        startpoint = System.currentTimeMillis();
        for (int i = 0; i < 10000; i++) {
            total += random.nextInt(0xFFFFF);
        }
        endpoint = System.currentTimeMillis();

        msg = "Cycle%2010000%20times%20takes%20" + (endpoint - startpoint) + "ms";
        postMessageToServer(msg);

        /* file system(device storage) usage */
        try {
            File rootPath = new File("D:/");
            if (allowFileList)
            {
                used = getDirectorySize(rootPath);
                free = totalFS - used;
            }
            else
            {
                free = rootPath.availableSize();
                used = rootPath.totalSize() - free;
            }
        } catch (NullPointerException e) {
            System.out.println("rootPath[D:/]:" + e);
            used = 0;
            free = totalFS;
        }

        msg = "FS%20Flash%20used(" + (used >> 10) + "K),%20" +
                "free(" + (free >> 10) + "K)";
        postMessageToServer(msg);
    }
}