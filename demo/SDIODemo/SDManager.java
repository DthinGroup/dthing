import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;

import javax.microedition.io.Connector;
import javax.microedition.io.HttpConnection;
import javax.microedition.io.file.FileConnection;

import iot.oem.sdio.*;

public class SDManager extends Applet {
	  private static final String testFilePath = "D:/Java/tmp.txt";
	  private static final String initString = "File";
	  private static final String testString = "Yarlung-Soft";
	  private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";

	  public SDManager() {
	  }

	  private void createTestFile() {
        try {
          FileConnection fc = (FileConnection)Connector.open(testFilePath);
          if(!fc.exists())
          {
              fc.create();
              System.out.println("create file");
          }

          if (fc.canWrite())
          {
              DataOutputStream dos = fc.openDataOutputStream();
              dos.writeUTF(initString);
              System.out.println("write to file");
          }

          if (fc.canRead())
          {
            DataInputStream dis = fc.openDataInputStream();
            String fcstr = dis.readUTF();
            System.out.println("read:" + fcstr);
            if (fcstr.compareTo(initString) == 0)
            {
              System.out.println("setup successed");
            }
          }

          fc.close();
        } catch (IOException e1) {
          System.out.println("IOException:" + e1);
        }
	  }

    public void cleanup() {
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        new Thread() {
            private SDIOImplement sdObj = null;
            private boolean allowLogPrint = true;

            public void run() {
                createTestFile();
                System.out.println("[SDDemo]Start SDIO test");

                try {
                  sdObj = new SDIOImplement(testFilePath);
                  DataOutputStream dos = new DataOutputStream(sdObj.getOutputStream());
                  dos.writeUTF(testString);
                  DataInputStream dis = new DataInputStream(sdObj.getInputStream());
                  String expectedString = dis.readUTF();
                  if (expectedString.compareTo(testString) == 0)
                  {
                    reportTestInfo("SDIO%20Success", testString, expectedString);
                  }
                  else
                  {
                    reportTestInfo("SDIO%20Failure", testString, expectedString);
                  }

                  sdObj.close();
                } catch (IOException e) {
                  System.out.println("IOException:" + e);
                }

                System.out.println("[SDDemo] End SDIO test");
            }

            private void reportTestInfo(String name, String writeIn, String readOut) throws IOException
            {
                String content = name + ":writein(" + writeIn + ")|readout(" + readOut + ")";
                String reportInfo = REPORT_SERVER_FORMAT + content;

                if (allowLogPrint)
                {
                    System.out.println("[" + name + "]" + content);
                }

                HttpConnection httpConn = (HttpConnection)Connector.open(reportInfo);
                httpConn.setRequestMethod(HttpConnection.POST);
                DataInputStream dis = httpConn.openDataInputStream();
                dis.close();
                httpConn.close();
            }
        }.start();
    }
}
