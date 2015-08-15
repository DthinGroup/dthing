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
    //private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";
	private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?uploadData";
	//定义上传到服务器的属性和默认值
    private static String imei = "1234";
    private static String imsi = "1234";
    private static String password = "111";
    private static String longitude = "100999897"; //上报格式: 符号位 + 度(3位) + 分(换算后保留6位)
    private static String latitude = "36010203"; //上报格式：符号位 + 度(2位) + 分(换算后保留6位)
    private static String altitude = "210";
    private static String speed = "50";
    private static String deviation = "100";
    private static int deviation_int = 0;
    private static String time= "20150814130101";
    private static String cellid = "0";
    private static String lac = "0";
    private static String batterylevel = "0";

	
    private static boolean allowLogPrint = true;
    private static boolean allowRunning = true;
    //protected static String longitude = "100.000001";
   // protected static String latitude = "36.000001";
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

    private CommConnectionImpl gpsComm = null;
    private GPSParser parser = null;
    private InputStream gis = null;
    private StepCounter counter = null;
    private byte[] accBuf = null;
    private byte[] gpsBuf = null;
    private Gpio ldo = null;
	private StringBuffer gpsStrBuf = null;

    public void cleanup() {
        allowRunning = false;
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        log("check - 1 -");
        openGSensorModule();
        log("check - 2 -");
        startGPSThread();
        while(allowRunning) {
            log("check - 3 -");
            readGSensorModule();
            log("check - 4 -");
			
			try {
			    log("Gsensor Sleep");
				Thread.sleep(1000);
			}catch (InterruptedException e) {
				log("InterruptedException:" + e);
			}
            if (isUpdated) {
                isUpdated = false;
                //netlog("lo:" + longitude + ",la:" + latitude + ",step:" + stepcount + ",date:" + gpsdate + ",time:" + gpstime);
				//按照服务器要求上报指定格式数据，没有获取属性的使用默认属性数据
				String info = "&imei=" + imei + "&imsi=" + imsi + "&password=" + password + "&longitude="
                + longitude + "&latitude=" + latitude + "&altitude=" + altitude + "&speed=" + speed
                + "&deviation=" + stepcount + "&time=" + time + "&cellid=" + cellid + "&lac=" + lac
                + "&batterylevel=" + batterylevel;
				//reportTestInfo(info);
				log (info);
				
           }
       }
       log("check - 5 -");
       closeGSensorModule();
       log("check - 6 -");
       notifyDestroyed();
    }

    public void openGPSModule() {
        try {
            ldo = new Gpio(60); //60 for board, 7 for shoe
            ldo.setCurrentMode(Gpio.WRITE_MODE);
            ldo.write(true);
            log("check - 0.1 -");
            gpsComm = CommConnectionImpl.getComInstance(0, 9600);
            log("check - 0.2 -");
            parser = new GPSParser();
            gis = gpsComm.openInputStream();
            log("check - 0.3 -");
            gpsBuf = new byte[128];
			gpsStrBuf = new StringBuffer(128);
            Thread.sleep(3000);
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
            log("check - 3.1 -");
            int readSize = gis.read(gpsBuf, 0, 128);
			
            log("check - 3.2 - readSize:" + readSize);
            if (readSize < 0)
            {
                log("exit when readSize is less than 0");
                return;
            }
			
            String readString = new String(gpsBuf).trim();
			log("read:" + convertEscapedChar(readString));
			
			/*
            parser.save(readString);
            longitude = parser.getLongtiInfo();
            latitude = parser.getLatiInfo();
            gpstime = parser.getTimeInfo();
            gpsdate = parser.getDateInfo();
			*/
			
            Thread.sleep(1000);
        } catch (IOException e) {
            log("GPS IOException:" + e);
        } catch (InterruptedException e) {
          log("Gpio InterruptedException:" + e);
    }
    }

    public void closeGPSModule() {
        try {
            ldo.setCurrentMode(Gpio.WRITE_MODE);
            ldo.write(false);
            gpsComm.close();
            gpsComm = null;
            gis = null;
            parser = null;
            gpsBuf = null;
        } catch (IOException e) {
            log("GPS IOException:" + e);
        }
    }

    public void startGPSThread() {
        new Thread() {
            public void run() {
                log("check - 0 -");
                openGPSModule();

                while(allowRunning) {
				    
					readGPSModule();
						
                }

                closeGPSModule();
                notifyDestroyed();
            }
        }.start();
    }

    public void openGSensorModule() {
        try {
            manager = new I2CManager(busId, I2CManager.DATA_RATE_FAST, slaveAddress, regAddressNumber);
            log("check - 1.1 -");
            initGSensor();
            log("check - 1.2 -");
            initAccelerator();
            log("check - 1.3 -");
            enableAccelerator();
            log("check - 1.4 -");
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
            log("2 save value:" + xAc + ":" + yAc + ":" + zAc);
			
            counter.saveAccValue(xAc, yAc, zAc);
            if (counter.available()) {
                stepcount += counter.fetchStepCount();
                log("update stepcount to " + stepcount);
                isUpdated = true;
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

        //set filter data bandwidth to 1000HZ (See 5.8 Bandwidths)
        reg[0] = 0x10;
        buff[0] = 0x10;
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
	
		final String reportInfo = REPORT_SERVER_FORMAT + msg.replace(' ', '.');
			
	    new Thread() {
            public void run() {

				
				log("reportTestInfo sending.................");
				log(reportInfo);
				
				try {
					URL url = new URL(reportInfo);
					HttpURLConnection httpConn = (HttpURLConnection)url.openConnection();
					//httpConn = (HttpURLConnection)url.openConnection();
					httpConn.setRequestMethod(HttpURLConnection.POST);
					InputStream dis = httpConn.getInputStream();
					dis.close();
					httpConn.disconnect();
				} catch (IOException e) {
					System.out.println("IOException:" + e);
				}
				notifyDestroyed();
			}
		}.start();
    }

    private void netlog(String msg)
    {
        String content = "SmartShoe:" + msg.replace(' ', '.');
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

    private void log(String msg)
    {
        System.out.println("SmartShoe:" + msg);
    }
}


