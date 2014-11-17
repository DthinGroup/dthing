import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;
import java.io.DataInputStream;
import java.io.IOException;

import iot.oem.i2c.I2CManager;

public class I2CSensor extends Applet {
    private static final int I2CBusID = 2;
    private static final int DevAddress = 0xB8;
    private static final int SubAddress = 0x00040003;
    private double Temperature;
    private double Humidity;
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=wsd&parmInfo=";
    private static int count = 12;
    private static boolean allowLogPrint = true;

    public I2CSensor() {
      // TODO Auto-generated constructor stub
    }

    public void cleanup() {
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        new Thread() {
            private I2CManager manager = null;

            public void run()
            {
              System.out.println("Start to run I2C:");

                do {
                  count--;
                    // open I2C handle
                    try {
                    this.manager = new I2CManager(I2CBusID, I2CManager.DATA_RATE_STANDARD);
                    System.out.println("I2C: open manager " + this.manager);
                } catch (IllegalArgumentException e) {
                    System.out.println("I2C IllegalArgumentException: " + e);
                } catch (IOException e) {
                    System.out.println("I2C IOException: " + e);
                }

                // write to I2C
                byte[] wdata = {0x03, 0x00, 0x04};
                try {
                    System.out.println("I2C: write to I2C");
                  this.manager.send(DevAddress, I2CManager.ADDRESS_TYPE_7BIT, SubAddress, wdata);
                } catch (IllegalArgumentException e) {
                    System.out.println("I2C IllegalArgumentException: " + e);
                } catch (NullPointerException e) {
                    System.out.println("I2C NullPointerException: " + e);
                } catch (IOException e) {
                    System.out.println("I2C IOException: " + e);
                }

                // read from i2c
                byte[] rdata = new byte[15];
                try {
                    this.manager.receive(DevAddress, I2CManager.ADDRESS_TYPE_7BIT, SubAddress, rdata);
                    System.out.println("read from i2c");
                } catch (IllegalArgumentException e) {
                    System.out.println("I2C IllegalArgumentException: " + e);
                } catch (NullPointerException e) {
                    System.out.println("I2C NullPointerException: " + e);
                } catch (IOException e) {
                    System.out.println("I2C IOException: " + e);
                }

                // parse data
                parse(rdata);

                // close
                try {
                    this.manager.destroy();
                } catch (IOException e) {
                    System.out.println("I2C IOException: " + e);
                }

                try {
              reportI2CInfo(Humidity/10, Temperature/10);
            } catch (IOException e) {
              System.out.println("I2C report exception " + e);
            }

                try {
                    Thread.sleep(10000L);
                } catch (InterruptedException e) {
                    System.out.println("I2C InterruptedException: " + e);
                }
                }while(count > 0);
            }
        }.start();
    }

    private void parse(byte[] data)
    {
        //TODO: Check CRC

        Humidity = convertByteToChar(data[2])*256 + convertByteToChar(data[3]);
        Temperature = convertByteToChar(data[4])*256 + convertByteToChar(data[5]);

        System.out.println("==Humidity== " + Humidity + "%");
        System.out.println("==Temperature== " + Temperature + "C");
    }

    private char convertByteToChar(byte value)
    {
        char result = (char) ((value < 0)? (value + 256) : value);
        return result;
    }

    private void reportI2CInfo(double humidity, double temperature) throws IOException
    {
        String content = "Humidity:" + humidity + "%25;temperature:" + temperature + "C";
        String reportInfo = REPORT_SERVER_FORMAT + content;

        if (allowLogPrint)
        {
            System.out.println("[I2CSensor]" + content);
        }
    }
}