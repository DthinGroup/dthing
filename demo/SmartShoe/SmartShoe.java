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

    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";
    private static boolean allowLogPrint = true;
    private static boolean allowRunning = true;
    protected static String longitude = "100.000001";
    protected static String latitude = "36.000001";
    protected static String gpstime = "0";
    protected static String gpsdate = "0";
    protected static int stepcount = 0;
    private static I2CManager manager = null;
    private static int busId = 2;
    private static int slaveAddress = 0x30;
    private static int regAddressNumber = 1;
    private static int subAccAddress = 0x02; //sub address for accelerator data read
    private static int accRange = 0;

    public void cleanup() {
        allowRunning = false;
    }

    public void processEvent(Event paramEvent) {
    }
    
    public void startup() {
    	startupGPSThread();
    	startupGSensorThread();

    	while(allowRunning) {
            try {
            	log("longitude:" + longitude + ",latitude:" + latitude + ",stepcount:" + stepcount + ",date:" + gpsdate + ",time:" + gpstime);
				Thread.sleep(5000);
			} catch (InterruptedException e) {
				log("Exception:" + e);
			}
    	}
    }

    public void startupGPSThread() {
        new Thread() {
            public void run() {
                try {
                    Gpio ldo = new Gpio(60); //60 for board, 7 for shoe
                    ldo.setCurrentMode(Gpio.WRITE_MODE);
                    ldo.write(true);
                    log("pull GPIO 60 to high");
                } catch (IllegalArgumentException e1) {
                    log("Gpio IllegalArgumentException:" + e1);
                } catch (IOException e1) {
                    log("Gpio IOException:" + e1);
                }

                CommConnectionImpl gpsComm = CommConnectionImpl.getComInstance(0, 9600);
                try {
                    byte[] buf = new byte[128];

                    InputStream is = gpsComm.openInputStream();
                    int readSize = 0;
                    GPSParser parser = new GPSParser();

                    do {
                        try {
                            Thread.sleep(10000L);
                        } catch (InterruptedException e) {
                        	log("InterruptedException:" + e);
                        }

                        readSize = is.read(buf, 0, 128);

                        if (readSize < 0)
                        {
                        	log("exit when readSize is less than 0");
                            break;
                        }

                        String readString = new String(buf);
                        parser.save(readString);
                        longitude = parser.getLongtiInfo();
                        latitude = parser.getLatiInfo();
                        gpstime = parser.getTimeInfo();
                        gpsdate = parser.getDateInfo();
                        //log("read:" + convertEscapedChar(readString));
                    } while (allowRunning);
                    gpsComm.close();
                    notifyDestroyed();
                } catch (IOException e) {
                	log("GPS IOException:" + e);
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
        }.start();
    }
    
    public void startupGSensorThread() {
        try {
			startGSensor();
	        AcceleratorTest();
		} catch (IOException e) {
			log("IOException:" + e);
		}
    }
    
    private void startGSensor() throws IOException
    {
        manager = new I2CManager(busId, I2CManager.DATA_RATE_FAST, slaveAddress, regAddressNumber);
        initGSensor();
    }

    private void resetGSensor()
    {
        //TODO:
    }

    private void stopGSensor() throws IOException
    {
        manager.destroy();
        manager = null;
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

    //accValue = sign + valueHead + "." + valueTail
    //sign = "-" or ""
    public String getAccValue(int lsb, int msb)
    {
        String accValue = null;
        int sign = (msb & 0x80) >> 7;
        int value = ((msb & 0x7f) << 2) + ((lsb & 0xc0) >> 6);
        int valueHead = value * accRange / 512;
        int valueTail = (value * accRange % 512) * 1000 / 512;
        accValue = ((sign > 0)? "-" : "") + valueHead + "." + valueTail;
        return accValue;
    }

    //accValue = sign + valueHead + "." + valueTail
    //sign = "-" or ""
    public int getAccIntValue(int lsb, int msb)
    {
        int accValue = 0;
        int sign = (msb & 0x80) >> 7;
        int value = ((msb & 0x7f) << 2) + ((lsb & 0xc0) >> 6);
        int valueHead = value * accRange / 512;
        int valueTail = (value * accRange % 512) * 1000 / 512;
        accValue = valueHead * 100 + valueTail;
  
        if (sign > 0) {
        	accValue = -accValue;
        }
        return accValue;
    }
    
    private void AcceleratorTest() throws IOException
    {
        initAccelerator();
        enableAccelerator();
        //data collector thread
        new Thread() {
            byte[] buf = new byte[6];
            /*
            String xStr = null;
            String yStr = null;
            String zStr = null;
            */
            int xAc = 0;
            int yAc = 0;
            int zAc = 0;

            public void run() {
            	StepCounter counter = new StepCounter();

                while(allowRunning) {
                    try {
                        manager.receive(slaveAddress, I2CManager.ADDRESS_TYPE_7BIT, subAccAddress, buf);
                        /*
                        xStr = getAccValue(buf[0], buf[1]);
                        yStr = getAccValue(buf[2], buf[3]);
                        zStr = getAccValue(buf[4], buf[5]);
                        */
                        xAc = getAccIntValue(buf[0], buf[1]);
                        yAc = getAccIntValue(buf[2], buf[3]);
                        zAc = getAccIntValue(buf[4], buf[5]);
                        counter.saveAccValue(xAc, yAc, zAc);
                        log(xAc + ":" + yAc + ":" + zAc);

                        if (counter.available()) {
                        	stepcount += counter.fetchStepCount();
                        	log("update stepcount to " + stepcount);
                        }

                    } catch (IllegalArgumentException e) {
                        log("IllegalArgumentException:" + e);
                    } catch (NullPointerException e) {
                        log("NullPointerException:" + e);
                    } catch (IOException e) {
                        log("IOException" + e);
                    }
                }
                try {
					stopGSensor();
				} catch (IOException e) {
					log("IOException:" + e);
				}
                notifyDestroyed();
            }
        }.start();
    }
    
    private void reportTestInfo(String msg) throws IOException
    {
        String content = "SmartShoe:" + msg.replace(' ', '.');
        String reportInfo = REPORT_SERVER_FORMAT + content;

        if (allowLogPrint)
        {
            System.out.println(content);
        }

        URL url = new URL(reportInfo);
        HttpURLConnection httpConn = (HttpURLConnection)url.openConnection();
        httpConn.setRequestMethod(HttpURLConnection.POST);
        InputStream dis = httpConn.getInputStream();
        dis.close();
        httpConn.disconnect();
    }

    private void log(String msg)
    {
        try {
            reportTestInfo(msg);
        } catch (IOException e) {
            System.out.println("IOException:" + e);
        }
    }
}
