import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;
import java.io.DataInputStream;
import java.io.IOException;
import java.net.ClientSocket;
import java.net.UnknownHostException;

import java.net.DatagramSocket;
import java.net.DatagramPacket;

public class UDPTest extends Applet {
    private static String host = "127.0.0.1";
    private static int port = 5050;
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";
    private static boolean allowLogPrint = true;

    public UDPTest() {
      // TODO Auto-generated constructor stub
    }

    public void cleanup() {
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        //Server Thread
        new Thread() {
            public void run() {
                int count = 10;
                try {
					DatagramSocket udpserver = new DatagramSocket(port);
                    while(count > 0)
                    {
                        byte[] buff = new byte[128];
 						DatagramPacket dgbuff = new DatagramPacket(buff, 128);
                        count--;
                        reportTestInfo("UDPS", "Waiting on port " + port + "...");
                        udpserver.receive(dgbuff);
                        String received = new String(buff);
                        reportTestInfo("UDPS", "Received message:" + received);
                    }
                    udpserver.close();
                    reportTestInfo("UDPS", "Success to exit server thread...");
                } catch (IOException e) {
                    try {
                        reportTestInfo("UDPS", "IOException:" + e);
                    } catch (IOException e1) {
                        System.out.println("[UDPS]IOException:" + e1);
                    }
                }
            }

            private void reportTestInfo(String name, String msg) throws IOException
            {
                String content = name + ":" + msg.replace(' ', '.');
                String reportInfo = REPORT_SERVER_FORMAT + content;

                if (allowLogPrint)
                {
                    System.out.println("[" + name + "]" + content);
                }
            }
        }.start();
        //Client Thread
        new Thread() {
            public void run() {
                int count = 10;
                String testData = "test";

                try {
                    ClientSocket udpclient = new ClientSocket(host, port);
                    while(count > 0)
                    {
                        count--;
                        testData = testData + count;
                        udpclient.send(testData.getBytes());
                        reportTestInfo("UDPC", "Send data: " + new String(testData.getBytes()));
                        sleep(6000);
                    }
                    udpclient.close();
                    reportTestInfo("UDPC", "Success to exit client thread...");
                } catch (UnknownHostException e) {
                    try {
                        reportTestInfo("UDPC", "UnknownHostException:" + e);
                    } catch (IOException e1) {
                        System.out.println("[UDPC]IOException:" + e1);
                    }
                } catch (IllegalArgumentException e) {
                    try {
                        reportTestInfo("UDPC", "IllegalArgumentException:" + e);
                    } catch (IOException e1) {
                        System.out.println("[UDPC]IOException:" + e1);
                    }
                } catch (IOException e) {
                    try {
                        reportTestInfo("UDPC", "IOException:" + e);
                    } catch (IOException e1) {
                        System.out.println("[UDPC]IOException:" + e1);
                    }
                } catch (InterruptedException e) {
                    try {
                        reportTestInfo("UDPC", "InterruptedException:" + e);
                    } catch (IOException e1) {
                        System.out.println("[UDPC]IOException:" + e1);
                    }
                }
            }

            private void reportTestInfo(String name, String msg) throws IOException
            {
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