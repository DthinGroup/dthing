import java.io.IOException;
import java.io.OutputStream;
//import java.net.ServerSocket;
import java.net.Socket;
import java.net.UnknownHostException;


import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;


public class SocketTest extends Applet {
    private static final String IP = "42.121.18.62";
    private static final int PORT = 9018;
    private static Socket handle = null;

    public SocketTest() {
        // TODO Auto-generated constructor stub
    }

    public void cleanup() {
        // TODO Auto-generated method stub

    }

    public void processEvent(Event arg0) {
        // TODO Auto-generated method stub

    }

    public void startup() {
        /*new Thread() {
            public void run() {
                try {
                    ServerSocket ss = new ServerSocket(PORT);
                    System.out.println("Start server with port " + PORT);
                    ss.accept();
                    System.out.println("client connected");
                } catch (IOException e) {
                    System.out.println(e.getMessage());
                }
            }
        }.start();*/

        new Thread() {
            public void run() {
                connect();
                if (handle != null) {
                    send("Hello Socket");
                }
                disconnect();
            }
        }.start();
    }

    public static void disconnect() {
        System.out.println("disconnect");
        if (handle != null) {
            try {
                handle.close();
                handle = null;
            } catch (IOException e) {
                System.out.println("IOException:" + e);
            }
        }
    }

    public static void connect() {
        try {
            handle = new Socket(IP, PORT);
            handle.setKeepAlive(true);
            handle.setReuseAddress(true);
            System.out.println("connect to " + IP + ":" + PORT);
        } catch (UnknownHostException e) {
            handle = null;
            System.out.println(e.getMessage());
        } catch (IOException e) {
            handle = null;
            System.out.println(e.getMessage());
        }
    }

    public static boolean send(byte[] data) {
        boolean result = true;
        try {
            OutputStream os = handle.getOutputStream();
            os.write(data);
            os.write("\\r\\n".getBytes());
            System.out.println("send data:" + new String(data));
            os.flush();
        } catch (IOException e) {
            System.out.println("send:" + e);
            result = false;
        }
        return result;
    }

    public static boolean send(String data) {
        return send(data.getBytes());
    }
}
