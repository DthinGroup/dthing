/**
 * Copyright 2012-2014 Yarlung Soft. All Rights Reserved.
 * $Id: $
 */
import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;
import java.io.DataInputStream;
import java.io.IOException;

import iot.oem.i2c.I2CManager;

import javax.microedition.io.Connector;
import javax.microedition.io.HttpConnection;

/**
 * 温湿度传感器演示软件
 *
 * 测 试 次 数：12
 * 间       隔：10秒左右
 * 总  线   ID: 2
 * 主  地   址：0xB8
 * 从  地   址：0x040003 (03, 00, 04)
 */
public class WSDSensor extends Applet {
    private static final int I2CBusID = 1; //1:8800 2:8500/8900
    private static final int DevAddress = 0xB8;
    private static final int SubAddress = 0x00040003;
    private double Temperature;
    private double Humidity;
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=wsd&parmInfo=";
    private static int count = 12;
    private static boolean allowLogPrint = true;

    public WSDSensor() {
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
                System.out.println("Starting WSD demo test ...");

                do {
                    count--;
                    //创建I2CManager实例以访问传感器
                    try {
                        this.manager = new I2CManager(I2CBusID, I2CManager.DATA_RATE_STANDARD);
                    } catch (IllegalArgumentException e) {
                        System.out.println("I2C IllegalArgumentException: " + e);
                    } catch (IOException e) {
                        System.out.println("I2C IOException: " + e);
                    }

                    //向传感器请求温湿度数据
                    byte[] wdata = {0x03, 0x00, 0x04};
                    try {
                      System.out.println("Requesting WSD data ...");
                        this.manager.send(DevAddress, I2CManager.ADDRESS_TYPE_7BIT, SubAddress, wdata);
                    } catch (IllegalArgumentException e) {
                        System.out.println("I2C IllegalArgumentException: " + e);
                    } catch (NullPointerException e) {
                        System.out.println("I2C NullPointerException: " + e);
                    } catch (IOException e) {
                        System.out.println("I2C IOException: " + e);
                    }

                    //从传感器读取温湿度数据
                    byte[] rdata = new byte[15];
                    try {
                        this.manager.receive(DevAddress, I2CManager.ADDRESS_TYPE_7BIT, SubAddress, rdata);
                        System.out.println("Receiving WSD data ...");
                    } catch (IllegalArgumentException e) {
                        System.out.println("I2C IllegalArgumentException: " + e);
                    } catch (NullPointerException e) {
                        System.out.println("I2C NullPointerException: " + e);
                    } catch (IOException e) {
                        System.out.println("I2C IOException: " + e);
                    }

                    //解析读取的数据
                    parse(rdata);
                    System.out.println("WSD data processing ...");

                    //关闭传感器访问实例
                    try {
                        this.manager.destroy();
                    } catch (IOException e) {
                        System.out.println("I2C IOException: " + e);
                    }
                    //上报测试数据
                    try {
                        reportI2CInfo(Humidity/10, Temperature/10);
                    } catch (IOException e) {
                        System.out.println("I2C report exception " + e);
                    }
                    //休眠约10秒
                    try {
                        Thread.sleep(10000L);
                    } catch (InterruptedException e) {
                        System.out.println("I2C InterruptedException: " + e);
                    }
                }while(count > 0);
                System.out.println("Exiting WSD Demo Test ...");
            }
        }.start();
    }

    /**
     * @brief 解析传感器返回数据并生成实时温度和湿度
     * @param data 用于解析的返回数据
     * @note 解析及计算公式见代码
     */
    private void parse(byte[] data)
    {
        //TODO: Check CRC
        Humidity = convertByteToChar(data[2])*256 + convertByteToChar(data[3]);
        Temperature = convertByteToChar(data[4])*256 + convertByteToChar(data[5]);
    }

    /**
     * @brief 转换传感器返回的Byte数值为Char
     * @param value 传感器返回的Byte数值
     * @return 转化后的char数值
     */
    private char convertByteToChar(byte value)
    {
        char result = (char) ((value < 0)? (value + 256) : value);
        return result;
    }

    /**
     * @brief 上报温湿度信息到指定服务器
     * @param humidity 湿度数值
     * @param temperature 温度数值
     * @exception 当网络连接有问题时抛出IO异常
     */
    private void reportI2CInfo(double humidity, double temperature) throws IOException
    {
        String content = "Humidity:" + humidity + "%25;temperature:" + temperature + "C";
        String reportInfo = REPORT_SERVER_FORMAT + content;

        if (allowLogPrint)
        {
            System.out.println("[WSD][Test Result]" + content);
        }

        HttpConnection httpConn = (HttpConnection)Connector.open(reportInfo);
        httpConn.setRequestMethod(HttpConnection.POST);
        DataInputStream dis = httpConn.openDataInputStream();
        dis.close();
        httpConn.close();
    }
}