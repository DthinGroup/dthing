
import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;

import jp.co.cmcc.message.sms.*;

public class SmsTest extends Applet
{
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";
    private static boolean allowLogPrint = true;
    private static boolean allowRunning = true;
    
    public SmsTest() {
    }

    public void cleanup() {
        allowRunning = false;
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() { 
        new Thread() {
            public void run() {
            	System.out.println("[SmsTest]Start SMS test.");

                try {
                    SMSConnection conn = SMSConnection.getInstance();
                    String textData = "test data";

                    //send the message
                    MessageSender sender = conn.getSender();
                    SMSMessage sendMessage = new SMSMessage(textData);
                    //sendMessage.setMessageAddress("1064899990000");  
                    //System.out.println("[SmsTest]send the message to 1064899990000 server. textData = " + textData);
					//String rtNumber = "13764016422";
					sendMessage.setMessageAddress(rtNumber);  
                    System.out.println("[SmsTest]send the message to" + rtNumber + ". textData = " + textData);
                    sender.send(sendMessage);

                    //receive the message
                    MessageReceiver receiver = conn.getReceiver();
                    Message message = receiver.receive();
                    if(message instanceof SMSMessage) {
                        SMSMessage smsMessage = (SMSMessage) message;
                      	netlog("received data");
                        if(smsMessage.getMode() == SMSMessage.TEXT_MODE)
                        {
                        	  String recData = new String(smsMessage.getData());
                        	  System.out.println("[SmsTest]receive data is :" + recData);
                        	  if (recData.equals(textData)) {
                        	      System.out.println("[SmsTest] the receive data is equal as send data.");
                        	  } else {
                        	      System.out.println("[SmsTest] the receive data is not equal as send data.");
                        	  }
                        }
                    }
                
                    conn.close();
                }catch(Exception e) {
                    //e.printStackTrace();
                    System.out.println("[SmsTest]exception happened on startApp.");
                }               
            }
        }.start();       
    }

    private void netlog(String msg)
    {
        String content = "SmsTest:" + msg.replace(' ', '.');
        String reportInfo = REPORT_SERVER_FORMAT + content;

        try {
            System.out.println(content);
            URL url = new URL(reportInfo);
            HttpURLConnection httpConn = (HttpURLConnection)url.openConnection();
            httpConn.setRequestMethod(HttpURLConnection.POST);
            InputStream dis = httpConn.getInputStream();
            dis.close();
            httpConn.disconnect();
        } catch (IOException e) {
            System.out.println("IOException:" + e);
        }
    }
}
