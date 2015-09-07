import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;

import iot.oem.gpio.Gpio;
import iot.oem.i2c.I2CManager;
import iot.oem.comm.CommConnectionImpl;


public class SmartShoe extends Applet {
    private static final String NETLOG_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?uploadData";

    private static String imei = "135444411112222";
    private static String imsi = "460014411112222";
    private static String password = "11222233";
    private static String longitude = "116357331";
    private static String latitude = "39902663";
    private static String altitude = "210";
    private static String speed = "50";
    private static String deviation = "100";
    private static int deviation_int = 0;
    private static String time= "20150828150101";
    private static String cellid = "0";
    private static String lac = "0";
    private static String batterylevel = "0";


    private static boolean allowLogPrint = true;
    private static boolean allowRunning = true;

    protected static String gpstime = "0";
    protected static String gpsdate = "0";
    protected static int stepcount = 0;
    private static I2CManager manager = null;
    private static int busId = 2;
    private static int slaveAddress = 0x30;
    private static int regAddressNumber = 1;
    private static int subAccAddress = 0x02; //sub address for accelerator data read
    private static int accRange = 0;
    private static boolean isUpdated = false;
    private static int sendResultLoop = 0;

    private static CommConnectionImpl gpsComm = null;
    private static GPSParser parser = null;
    private static InputStream gis = null;
    private static StepCounter counter = null;
    private static byte[] accBuf = null;
    private static byte[] gpsBuf = null;
    private static Gpio ldo = null;
    private static StringBuffer gpsStrBuf = null;
    private Thread gsensorThread = null;
    private Thread gpsThread = null;

    private static final boolean DEBUG = false;
    private static long totalMemory = 0;
    private static long gcMemory = 0;

    private static final int DefaultGPSPort = 0;
    private static final int DefaultBaudrate = 9600;
    private static final int DefaultGPSBuffer = 128;
    private static final int DefaultGCPercentage = 50;
    private static int totalReadLength = 0;

    public void cleanup() {
        allowRunning = false;
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        netlog("startup with COM["+DefaultGPSPort+"] Baudrate["+DefaultBaudrate+"] GC["+DefaultGCPercentage+"] GPSBuffer["+DefaultGPSBuffer+"]");
        totalMemory = Runtime.getRuntime().totalMemory();
        gcMemory = totalMemory * DefaultGCPercentage/ 100;
        gsensorThread = new Thread() {
            public void run() {
            debug("check - openGSensorModule -");
            openGSensorModule();
            debug("check - openGSensorModule done -");
            while(allowRunning) {
                try {
                    debug("check - readGSensorModule -");
                    for (int i = 0; i < 50; i++) {
                        readGSensorModule();
                        }
                    debug("check - readGSensorModule done -");

                    if (isUpdated) {
                        isUpdated = false;
                        lac = "" + totalReadLength; //FIXME: show total read length of gps

                        String info = "&imei=" + imei + "&imsi=" + imsi + "&password=" + password + "&longitude="
                            + longitude + "&latitude=" +  latitude + "&altitude=" + altitude + "&speed=" + speed
                            + "&deviation=" + stepcount + "&time=" + time + "&cellid=" + cellid + "&lac=" + lac
                            + "&batterylevel=" + batterylevel;
                        reportTestInfo(info);
                    }
                    MemoryCheck();
                    Thread.sleep(1);
                } catch (InterruptedException e) {
                    log("InterruptedException:" + e);
                }
            }
            debug("check - closeGSensorModule -");
            closeGSensorModule();
            notifyDestroyed();
        }
    };

    gpsThread = new Thread() {
        public void run() {
        while(allowRunning) {
            debug("check - openGPSModule -");
            openGPSModule();
            debug("check - openGPSModule done -");
            try {
                Thread.sleep(5000);
                MemoryCheck();
                debug("check - readGPSModule -");
                readGPSModule();
                debug("check - readGPSModule done -");
                MemoryCheck();
            } catch (InterruptedException e) {
                log("InterruptedException:" + e);
            }
        }
        debug("check - closeGPSModule -");
        closeGPSModule();
        notifyDestroyed();
        }
    };

    gsensorThread.start();
    gpsThread.start();

    }

    public void openGPSModule() {
        try {
            if (gpsComm == null) {
                ldo = new Gpio(60); //60 for board, 7 for shoe
                ldo.setCurrentMode(Gpio.WRITE_MODE);
                ldo.write(true);
                debug("check - 0.1 -");
                gpsComm = CommConnectionImpl.getComInstance(DefaultGPSPort, DefaultBaudrate);
                if (gpsComm == null) {
                    return;
                }
                debug("check - 0.2 -");
                parser = new GPSParser();
                gis = gpsComm.openInputStream();
                debug("check - 0.3 -");
                gpsBuf = new byte[DefaultGPSBuffer];
                //gpsStrBuf = new StringBuffer(300);
                Thread.sleep(10000);
            }
        } catch (IllegalArgumentException e1) {
            log("Gpio IllegalArgumentException:" + e1);
        } catch (IOException e1) {
            log("Gpio IOException:" + e1);
        } catch (InterruptedException e) {
          log("Gpio InterruptedException:" + e);
        }
    }

    public void readGPSModule() {
        try {
            if (gis != null) {
                debug("check - 3.1 -");
                int readSize = gis.read(gpsBuf, 0, DefaultGPSBuffer);

                debug("check - 3.2 - readSize:" + readSize);
                if (readSize < 0)
                {
                    log("exit when readSize is less than 0");
                    return;
                }

                String readString = new String(gpsBuf).trim();
                String gpsData = convertEscapedChar(readString);
                totalReadLength += gpsData.length();
                log("read:" + gpsData);
                totalReadLength++;
                if (parser != null) {
                    totalReadLength++;
                    parser.save(gpsData);
                    totalReadLength++;
                    longitude = parser.getLongtiInfo();
                    totalReadLength++;
                    latitude = parser.getLatiInfo();
                    totalReadLength++;
                    gpstime = parser.getTimeInfo();
                    totalReadLength++;
                    gpsdate = parser.getDateInfo();
                    totalReadLength++;
                }
            }
        } catch (IOException e) {
            log("GPS IOException:" + e);
            totalReadLength += 20;
        }
    }

    public void closeGPSModule() {
        totalReadLength = 0;
        try {
            if (gpsComm == null) {
                gpsComm.close();
                gpsComm = null;
            }
            ldo.setCurrentMode(Gpio.WRITE_MODE);
            ldo.write(false);
            gis = null;
            parser = null;
            gpsBuf = null;
        } catch (IOException e) {
            log("GPS IOException:" + e);
        }
    }

    public void openGSensorModule() {
        try {
            manager = new I2CManager(busId, I2CManager.DATA_RATE_FAST, slaveAddress, regAddressNumber);
            debug("check - 1.1 -");
            initGSensor();
            debug("check - 1.2 -");
            initAccelerator();
            debug("check - 1.3 -");
            enableAccelerator();
            debug("check - 1.4 -");
            accBuf = new byte[6];
            counter = new StepCounter();
        } catch (IOException e) {
            log("GSensor IOException:" + e);
        }
    }

    public void readGSensorModule() {
        int xAc = 0;
        int yAc = 0;
        int zAc = 0;
        try {
            manager.receive(slaveAddress, I2CManager.ADDRESS_TYPE_7BIT, subAccAddress, accBuf);
            xAc = getAccIntValue(accBuf[0], accBuf[1]);
            yAc = getAccIntValue(accBuf[2], accBuf[3]);
            zAc = getAccIntValue(accBuf[4], accBuf[5]);
            log("15 save value:" + xAc + ":" + yAc + ":" + zAc);

            counter.saveAccValue(xAc, yAc, zAc);
            if (counter.available()) {
                stepcount += counter.fetchStepCount() * 4;
                isUpdated = true;
                log("update stepcount to " + stepcount);
            }

        } catch (IllegalArgumentException e) {
            log("GSensor IllegalArgumentException:" + e);
        } catch (NullPointerException e) {
            log("GSensor NullPointerException:" + e);
        } catch (IOException e) {
            log("GSensor IOException" + e);
        }
    }

    public void closeGSensorModule() {
        try {
            manager.destroy();
            manager = null;
            counter = null;
            accBuf = null;
        } catch (IOException e) {
            log("GSensor IOException:" + e);
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

    private void initGSensor() throws IOException
    {
        byte[] reg = new byte[1];
        byte[] buff = new byte[1];

        //Disable all interrupt (See 5.11 Interrupt settings)
        reg[0] = 0x16;
        buff[0] = 0x00;
        manager.send(slaveAddress, I2CManager.ADDRESS_TYPE_7BIT, reg[0], buff);

        //contains the definition of the theta blocking angle for the orientation interrupt.
        reg[0] = 0x2D;
        buff[0] = 0x01;
        manager.send(slaveAddress, I2CManager.ADDRESS_TYPE_7BIT, reg[0],buff);

        //contains the interrupt reset bit and the interrupt mode selection.
        reg[0] = 0x21;
        buff[0] = 0x03; //0x03 means temporary, 1s
        manager.send(slaveAddress, I2CManager.ADDRESS_TYPE_7BIT, reg[0], buff);

        //maps all interrupts to INT1 pin
        reg[0] = 0x19;
        buff[0] = (byte)0xF7;
        manager.send(slaveAddress, I2CManager.ADDRESS_TYPE_7BIT, reg[0], buff);

        //contains the behavioural configuration
        reg[0] = 0x20;
        buff[0] = 0x01;
        manager.send(slaveAddress, I2CManager.ADDRESS_TYPE_7BIT, reg[0], buff);

        //contains the threshold definition for the slope interrupt.
        reg[0] = 0x28;
        buff[0] = 0x08;
        manager.send(slaveAddress, I2CManager.ADDRESS_TYPE_7BIT, reg[0],buff);

        //contains the definition of the number of samples to be evaluated for the slope interrupt (any-motion detection).
        reg[0] = 0x27;
        buff[0] = 0x00;
        manager.send(slaveAddress, I2CManager.ADDRESS_TYPE_7BIT, reg[0],buff);

        //enables slope interrupt for x-axis, y-axis, z-axis and flat interrupt (See 5.11 Interrupt settings)
        reg[0] = 0x16;
        buff[0] = 0x47;
        manager.send(slaveAddress, I2CManager.ADDRESS_TYPE_7BIT, reg[0], buff);
    }

    private void initAccelerator() throws IOException
    {
        byte[] reg = new byte[1];
        byte[] buff = new byte[1];

        //set acceleration range to +-2g (See 5.7 g-range selection)
        reg[0] = 0x0f;
        buff[0] = 0x03;
        accRange = 2;
        manager.send(slaveAddress, I2CManager.ADDRESS_TYPE_7BIT, reg[0], buff);

        //set data collection and output, MSB data depends on LSB data (See 5.10 Special control settings)
        reg[0] = 0x13;
        buff[0] =0x0;
        manager.send(slaveAddress, I2CManager.ADDRESS_TYPE_7BIT, reg[0], buff);

        //set filter data bandwidth as buff[0] value, to update time(ms) (See 5.8 Bandwidths)
        //buff[0]: 0x08->7.81Hz(64ms), 0x09->15.63Hz(32ms), 0x0a->31.25Hz(16ms), 0x0b->62.5Hz(8ms)
        //         0x0c->125Hz(4ms), 0x0d->250Hz(2ms), 0x0e->500Hz(1ms) ,0x0f->1000Hz(0.5ms)
        reg[0] = 0x10;
        buff[0] = 0x0b;
        manager.send(slaveAddress, I2CManager.ADDRESS_TYPE_7BIT, reg[0], buff);

        //set power modes, no delay, lower volumn mode and 50ms to sleep (See 5.9 Power modes)
        reg[0] = 0x11;
        buff[0] = 0x58;
        manager.send(slaveAddress, I2CManager.ADDRESS_TYPE_7BIT, reg[0], buff);
    }

    private void enableAccelerator() throws IOException
    {
        byte[] reg = new byte[1];
        byte[] buff = new byte[1];

        //According to WriteG_Reg implementation, write [0x30, 0x02, 0x31] will start to read accelerate information
        reg[0] = 0x02;
        buff[0] = 0x31;
        manager.send(slaveAddress, I2CManager.ADDRESS_TYPE_7BIT, reg[0], buff);
    }

    //accStringValue = sign + valueHead + "." + valueTail
    //accValue = sign * (valueHead * 100 + valueTail/10)
    //sign = "-" or ""
    public int getAccIntValue(int lsb, int msb)
    {
        int accValue = 0;
        int sign = (msb & 0x80) >> 7;
        int value = ((msb & 0x7f) << 2) + ((lsb & 0xc0) >> 6);
        int valueHead = value * accRange / 512;
        int valueTail = (value * accRange % 512) * 1000 / 512;
        accValue = valueHead * 100 + valueTail/10;

        if (sign > 0) {
            accValue = -accValue;
        }
        return accValue;
    }

    private void reportTestInfo(String msg) {

    String reportInfo = REPORT_SERVER_FORMAT + msg.replace(' ', '.');

    log(msg);

    try {
      URL url = new URL(reportInfo);
      HttpURLConnection httpConn = (HttpURLConnection)url.openConnection();
      httpConn = (HttpURLConnection)url.openConnection();
      httpConn.setRequestMethod(HttpURLConnection.POST);
      InputStream dis = httpConn.getInputStream();
      dis.close();
      httpConn.disconnect();
    } catch (IOException e) {
      System.out.println("IOException:" + e);
    }
    }

    private void netlog(String msg)
    {
        String content = "SmartShoe:" + msg.replace(' ', '.');
        String reportInfo = NETLOG_SERVER_FORMAT + content;

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

  public static void MemoryCheck() {
      long free =  Runtime.getRuntime().freeMemory();
        //System.out.println("free:" + free + "/" + total);

        //Force VM to gc when memory is less than 30%
        if (free < gcMemory) {
            Runtime.getRuntime().gc();
        }

        //free =  Runtime.getRuntime().freeMemory();
        //System.out.println("free:" + free + "/" + total);
    }

    private void log(String msg)
    {
        System.out.println("SmartShoe:" + msg);
    }

    private void debug(String msg)
    {
      if (DEBUG) {
            System.out.println("SmartShoe:" + msg);
      }
    }
}




