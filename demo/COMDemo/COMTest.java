import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;

import jp.co.aplix.io.CommConnectionImpl;

public class COMTest extends Applet
{
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";
    private static boolean allowLogPrint = true;

    public void cleanup() {
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        new Thread() {
            public void run() {
                CommConnectionImpl comm = CommConnectionImpl.getComInstance(1);
                try {
                    byte[] buf = new byte[128];

                    InputStream is = comm.openInputStream();
                    int readSize;
                    do {
                        try {
                            Thread.sleep(10000L);
                        } catch (InterruptedException e) {
                            System.out.println("InterruptedException:" + e);
                        }

                        readSize = is.read(buf, 0, 28);
                        String readString = new String(buf);

                        reportTestInfo("COM", "read:" + convertEscapedChar(readString));
                    }
                    while (readSize >= 0);
                    comm.close();
                } catch (IOException e) {
                    System.out.println("IOException:" + e);
                }
            }

            private String convertEscapedChar(String original)
            {
                String escaped = "";
                char[] ctest = original.toCharArray();
                for (int i = 0; i < ctest.length; i++)
                {
                    if ((ctest[i] != '\r') && (ctest[i] != '\n'))
                    {
                        escaped = escaped + ctest[i];
                    }
                    else
                    {
                        escaped = escaped + '.';
                    }
                }
                return escaped;
            }

            private void reportTestInfo(String name, String msg) throws IOException {
                String content = name + ":" + msg.replace(' ', '.');
                String reportInfo = REPORT_SERVER_FORMAT + content;
                if (allowLogPrint)
                {
                  System.out.println("[" + name + "]" + content);
                }
            }
        }.start();
    }
}