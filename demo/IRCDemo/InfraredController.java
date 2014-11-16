import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.OutputStream;

import javax.microedition.io.Connector;
import javax.microedition.io.HttpConnection;
import jp.co.aplix.io.CommConnectionImpl;

public class InfraredController extends Applet {
    private static final String OPEN_CODE = "1B00A394090A20500220A8000000D0A394090A2070022000000000D0";
    private static final String CHANGE_CODE = "1B00A394090E20500220A800000010A394090E207002200000000010";
    private static final String CLOSE_CODE = "1B00A394010020500220A8000000B0A39401002070022000000000B0";
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";
    private static boolean allowLogPrint = true;

    public InfraredController() {
      // TODO Auto-generated constructor stub
    }

    public void cleanup() {
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        new Thread() {
            public void run() {
                CommConnectionImpl comm = CommConnectionImpl.getComInstance(1, 9600);
                try {
                    System.out.println("[IRC]Current baudrate is " + comm.getBaudRate());

                    OutputStream is = comm.openOutputStream();
                    is.write(hex2Bytes(OPEN_CODE));

                    try {
                        Thread.sleep(5000L);
                    } catch (InterruptedException e) {
                        System.out.println("InterruptedException:" + e);
                    }

                    reportTestInfo("IRC", "write:" + OPEN_CODE);

                    is.write(hex2Bytes(CLOSE_CODE));

                    try {
                        Thread.sleep(5000L);
                    } catch (InterruptedException e) {
                        System.out.println("InterruptedException:" + e);
                    }

                    reportTestInfo("IRC", "write:" + CLOSE_CODE);

                    comm.close();
                } catch (IOException e) {
                    System.out.println("IOException:" + e);
                }
            }

            private byte[] hex2Bytes(String src){
                byte[] res = new byte[src.length()/2];
                char[] chs = src.toCharArray();
                for(int i = 0,c = 0; i < chs.length; i+=2, c++){
                    res[c] = (byte) (Integer.parseInt(new String(chs, i, 2), 16));
                }
                return res;
            }

            private void reportTestInfo(String name, String msg) throws IOException {
                String content = name + ":" + msg.replace(' ', '.');
                String reportInfo = REPORT_SERVER_FORMAT + content;
                if (allowLogPrint)
                {
                  System.out.println("[" + name + "]" + content);
                }

                HttpConnection httpConn = (HttpConnection)Connector.open(reportInfo);
                httpConn.setRequestMethod("POST");
                DataInputStream dis = httpConn.openDataInputStream();
                dis.close();
                httpConn.close();
            }
        }.start();
    }
}
