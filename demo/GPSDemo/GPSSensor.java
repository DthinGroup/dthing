import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import jp.co.aplix.io.CommConnectionImpl;

public class GPSSensor extends Applet {
   private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=gps&parmInfo=";
    private static final String UPLOAD_SERVER_DEFAULT_ADDRESS = "www.clouddatasrv.com";
    private static final String UPLOAD_SERVER_DEFAULT_PORT = "8008";
    private static final String UPLOAD_PROTOCOL_UDP = "datagram://";
    private static final int UPLOAD_MESSAGE_TYPE_GPS = 1;
    private static final int UPLOAD_MESSAGE_TYPE_SMS = 2;
    private static final int UPLOAD_MESSAGE_TYPE_BS = 3;
    private static final String UPLOAD_MESSAGE_SCRIPT_VERSION = "2.0";
    private static final String UPLOAD_MESSAGE_HEAD = "B";
    private static final String UPLOAD_MESSAGE_END = "E";
    private static boolean isConnected = false;
    private static boolean allowLogPrint = true;
    private String imei;
    private String longitude;
    private String latitude;
    private String direction;
    private String speed;
    private String shake;
    private String mileage;
    private String power;
    private String acc;
    private String extension1;
    private String extension2;
    private String extension3;
    private String mcc;
    private String mnc;
    private String lac;
    private String mci;
    private String mciss;
    private String nci1;
    private String nciss1;
    private String nci2;
    private String nciss2;
    private String nci3;
    private String nciss3;
    private String nci4;
    private String nciss4;
    private String nci5;
    private String nciss5;
    private String nci6;
    private String nciss6;
    private int messageId;
    private boolean shakeFlag;

    public GPSSensor()
    {
        imei = "860078020016973";
        longitude = "0";
        latitude = "0";
        direction = "0";
        speed = "0";
        shake = "0";
        mileage = "0";
        power = "0";
        acc = "0";
        extension1 = "0";
        extension2 = "0";
        extension3 = "0";
        mcc = "0";
        mnc = "0";
        lac = "0";
        mci = "0";
        mciss = "0";
        nci1 = "0";
        nciss1 = "0";
        nci2 = "0";
        nciss2 = "0";
        nci3 = "0";
        nciss3 = "0";
        nci4 = "0";
        nciss4 = "0";
        nci5 = "0";
        nciss5 = "0";
        nci6 = "0";
        nciss6 = "0";
        messageId = 0;
        shakeFlag = false;
    }

    public void cleanup() {
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        (new Thread() {
            private CommConnectionImpl comm = null;

            public void run()
            {
                do
                {
                    //Open GPS port
                    if (GPSSensor.isConnected == false)
                    {
                        log("try to open gps .....");
                        comm = CommConnectionImpl.getCom1Instance();
                        try {
                            Thread.sleep(20000L);
                        } catch (InterruptedException e2) {
                            log("startApp run() sleep exception...");
                        }
                    }

                    //Test: Init only time
                    GPSSensor.isConnected = true;

                    log("read gps start.....");
                    String myGpsData = readGpsData();

                    if (GPSSensor.isConnected == true)
                    {
                        log("read gps end..... myGpsData=" + myGpsData);

                        try {
                            reportGPSInfo();
                        } catch (IOException e) {
                            log("failed to report GPS info with exception " + e);
                        }
                    }
                    else
                    {
                        log("gps is not ready ...");
                    }

                    try
                    {
                        Thread.sleep(5000L);
                    }
                    catch (InterruptedException e)
                    {
                        log("startApp run() sleep exception...");

                        try {
                            comm.close();
                            log("gps port is closed due to interruption");
                            break;
                        } catch (IOException e1) {
                            log("failed to close gps port");
                        }
                    }
                } while (true);
            }
        }).start();
    }

    private String readGpsData()
    {
        log("readGpsData called");
        updateGPSInfo();
        updateDeviceInfo();
        String gpsData = composeGPSMessage();
        return gpsData;
    }

    private boolean readShakeFlag()
    {
        boolean retFlag = shakeFlag;
        log("read shake flag " + retFlag);
        shakeFlag = !shakeFlag;
        return shakeFlag;
    }

    private void updateGPSInfo()
    {
        log("updateGPSInfo called");
        longitude = System.getProperty("gps.longtitude");
        if (longitude == null || longitude.equalsIgnoreCase("0000.0000"))
        {
            longitude = "116.345442";
        }
        else
        {
            longitude = formatlongitude(longitude);
            GPSSensor.isConnected = true;
        }
        latitude = System.getProperty("gps.latitude");
        if (latitude == null || latitude.equalsIgnoreCase("0000.0000"))
            latitude = "39.896217";
        else
            latitude = formatlatitude(latitude);
        log("updateGPSInfo longitude " + longitude + " latitude " + latitude);
    }

    private void updateDeviceInfo()
    {
        log("updateDeviceInfo called");
        String vibra = System.getProperty("yarlung.vibra");

        if (vibra != null)
        {
            shake = vibra.equals("true")? "1" : "0";
        }
        else
        {
          shake = "0";
        }

        log("shake flag is " + shake);
    }

    private void updateBSInfo()
    {
    }

    private String composeBSMessage()
    {
        StringBuffer bsData = new StringBuffer();
        bsData.append(imei);
        bsData.append(",");
        bsData.append(UPLOAD_MESSAGE_TYPE_BS);
        bsData.append(",");
        bsData.append(longitude);
        bsData.append(",");
        bsData.append(latitude);
        bsData.append(",");
        bsData.append(shake);
        bsData.append(",");
        bsData.append(power);
        bsData.append(",");
        bsData.append(mcc);
        bsData.append(",");
        bsData.append(mnc);
        bsData.append(",");
        bsData.append(lac);
        bsData.append(",");
        bsData.append(mci);
        bsData.append(",");
        bsData.append(mciss);
        bsData.append(",");
        bsData.append(nci1);
        bsData.append(",");
        bsData.append(nciss1);
        bsData.append(",");
        bsData.append(nci2);
        bsData.append(",");
        bsData.append(nciss2);
        bsData.append(",");
        bsData.append(nci3);
        bsData.append(",");
        bsData.append(nciss3);
        bsData.append(",");
        bsData.append(nci4);
        bsData.append(",");
        bsData.append(nciss4);
        bsData.append(",");
        bsData.append(nci5);
        bsData.append(",");
        bsData.append(nciss5);
        bsData.append(",");
        bsData.append(nci6);
        bsData.append(",");
        bsData.append(nciss6);
        String bsMessage = composeUploadMessage(bsData.toString());
        log("Send bs message: " + bsMessage);
        return bsMessage;
    }

    private String composeGPSMessage()
    {
        log("composeGPSMessage called");
        String gpsMessage;
        try
        {
            log("composeGPSMessage new buffer start...");
            StringBuffer gpsData = new StringBuffer();
            log("composeGPSMessage new buffer end...");
            gpsData.append(imei);
            gpsData.append(",");
            gpsData.append(UPLOAD_MESSAGE_TYPE_GPS);
            gpsData.append(",");
            gpsData.append(UPLOAD_MESSAGE_SCRIPT_VERSION);
            gpsData.append(",");
            gpsData.append(longitude);
            gpsData.append(",");
            gpsData.append(latitude);
            gpsData.append(",");
            gpsData.append(direction);
            gpsData.append(",");
            gpsData.append(speed);
            gpsData.append(",");
            gpsData.append(shake);
            gpsData.append(",");
            gpsData.append(mileage);
            gpsData.append(",");
            gpsData.append(power);
            gpsData.append(",");
            gpsData.append(acc);
            gpsData.append(",");
            gpsData.append(extension1);
            gpsData.append(",");
            gpsData.append(extension2);
            gpsData.append(",");
            gpsData.append(extension3);
            log("composeUploadMessage start");
            gpsMessage = composeUploadMessage(gpsData.toString());
            log("composeUploadMessage end");
        }
        catch (Exception e)
        {
            log("composeGPSMessage exception e=: " + e);
            gpsMessage = "B0060065860078020016973,1,2.0,116.210153,39.534581,0,0,0,0,0,0,0,0,0E";
        }
        catch (Error er)
        {
            log("composeGPSMessage error er=: " + er);
            gpsMessage = "B0060065860078020016973,1,2.0,116.210153,39.534581,0,0,0,0,0,0,0,0,0E";
        }
        log("composeGPSMessage return: " + gpsMessage);
        return gpsMessage;
    }

    private String composeSMSMessage(String message, String phoneNumber)
    {
        String smsData = imei + "," + UPLOAD_MESSAGE_TYPE_SMS + "," + phoneNumber + "," + message;
        String sms = composeUploadMessage(smsData);
        log("Send sms: " + sms);
        return sms;
    }

    private String composeUploadMessage(String data)
    {
        log("composeUploadMessage called");
        String uploadMessage = UPLOAD_MESSAGE_HEAD + getLengthFormat(data) + generateMessageId() + data + UPLOAD_MESSAGE_END;
        return uploadMessage;
    }

    private String formatlatitude(String latitude)
    {
        String deg = latitude.substring(0, 2);
        String min = latitude.substring(2);
        double ddeg = Double.valueOf(deg).doubleValue();
        double dmin = Double.valueOf(min).doubleValue() / 60D;
        log("formatlatitude =" + ddeg + "|" + dmin + "|" + (ddeg + dmin));
        log("formatlatitude =" + deg + "|" + min);
        String ret = (new Double(ddeg + dmin)).toString();
        log("formatlatitude ret=" + ret);
        if (ret.length() > 9)
          ret = ret.substring(0, 9);
        return ret;
    }

    private String formatlongitude(String longitude)
    {
        String deg = longitude.substring(0, 3);
        String min = longitude.substring(3);
        double ddeg = Double.valueOf(deg).doubleValue();
        double dmin = Double.valueOf(min).doubleValue() / 60D;
        log("formatlongitude =" + ddeg + "|" + dmin + "|" + (ddeg + dmin));
        log("formatlongitude =" + deg + "|" + min);
        String ret = (new Double(ddeg + dmin)).toString();
        log("formatlongitude ret=" + ret);
        if (ret.length() > 10)
          ret = ret.substring(0, 10);
        return ret;
    }

    private void reportGPSInfo() throws IOException
    {
        String longitudeType = "E";
        String latitudeType = "N";

        if (longitude.startsWith("-"))
        {
          longitude = longitude.substring(1);
          longitudeType = "W";
        }

        if (latitude.startsWith("-"))
        {
          latitude = latitude.substring(1);
          latitudeType = "S";
        }

        String content = longitudeType + ":" + longitude + ";" + latitudeType + ":" + latitude;
        String reportInfo = REPORT_SERVER_FORMAT + content;

        if (allowLogPrint)
        {
            System.out.println("[GPSSensor]" + content);
        }

        URL url = new URL(reportInfo);
        HttpURLConnection httpConn = (HttpURLConnection)url.openConnection();
        httpConn.setRequestMethod(HttpURLConnection.POST);
        InputStream dis = httpConn.getInputStream();
        dis.close();
        httpConn.disconnect();
    }

    private String generateMessageId()
    {
        log("generateMessageId called");
        String mid = String.valueOf(messageId / 100) + (messageId % 100) / 10 + messageId % 10;
        messageId = messageId > 999 ? 0 : messageId + 1;
        log("Generate message id " + mid);
        return mid;
    }

    private String getLengthFormat(String data)
    {
        log("getLengthFormat called");
        String length = String.valueOf(data.length() / 1000) + (data.length() % 1000) / 100 + (data.length() % 100) / 10 + data.length() % 10;
        log("format length is " + length);
        return length;
    }

    private void log(String log)
    {
        System.out.println("==GPS==" + log);
    }

    //Use uart com to access GPS data
}
