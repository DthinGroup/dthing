/**
 * Copyright 2012-2013 Yarlung Soft. All Rights Reserved.
 * $Id: $
 */
import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;
import java.io.DataInputStream;
import java.io.IOException;

import iot.oem.adc.ADCManager;

import javax.microedition.io.Connector;
import javax.microedition.io.HttpConnection;

/**
 * 加速度传感器演示软件
 *
 * 测试次数：16
 * 间    隔：10秒左右
 * 端    口: 0，7
 */
public class JSDSensor extends Applet {
    private static final int MAX_TEST_COUNT = 18;
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=jsd&parmInfo=";
    private static int count = MAX_TEST_COUNT;
    private static boolean allowLogOnServer = false;

    public JSDSensor() {
      // TODO Auto-generated constructor stub
    }

    public void cleanup() {
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        new Thread() {
            private ADCManager manager = null;
            private int result = -1;
            private double valueOfPort0 = 0;
            private double valueOfPort7 = 0;

            public void run() {
                System.out.println("[ADCDemo]Start ADC sensor test");

                do {
                    //创建加速度传感器实例
                    manager = ADCManager.getInstance();

                    if (manager == null) break;

                    //轮流读取端口0和7的数据并解析
                    try {
                        //read from channel 0
                        result = manager.read(0);
                        valueOfPort0 = convert(result);
                        System.out.println("[JSDDemo] read channelID[0] with result[" + valueOfPort0 + "]");

                        //read from channel 7
                        result = manager.read(7);
                        valueOfPort7 = convert(result);
                        System.out.println("[JSDDemo] read channelID[7] with result[" + valueOfPort7 + "]");
                    } catch (IllegalArgumentException e) {
                        System.out.println("JSDDemo IllegalArgumentException:" + e);
                    } catch (IOException e) {
                        System.out.println("JSDDemo IOException:" + e);
                    }
                    //上报端口对应数据到服务器
                    try {
                        reportADCInfo(valueOfPort0, valueOfPort7);
                    } catch (IOException e2) {
                        System.out.println("IOException: " + e2);
                    }

                    count--;
                    //休眠
                    try {
                        Thread.sleep(10000L);
                    } catch (InterruptedException e) {
                        System.out.println("InterruptedException: " + e);
                    }
                } while(count > 0);
                //销毁传感器实例
                try {
                    manager.destroy();
                } catch (IOException e1) {
                    System.out.println("IOException: " + e1);
                }
                System.out.println("[JSDDemo] End ADC sensor test");
            }

            /**
             * @brief 传感器数据转化
             * @param value 传感器返回数据
             * @return 输出的加速度数据
             */
            private double convert(int value)
            {
                double zero_g = 512.0;
                double scale = 102.3;
                double result = 0;
                result = (value - zero_g) / scale;
                int temp = (int)(result * 100);
                result = (double)temp / 100;
                return result;
            }

            /**
             * @brief 上报加速度信息到指定服务器
             * @param value 端口数据
             * @param cid   端口ID
             * @exception   当网络连接有问题时抛出IO异常
             */
            private void reportADCInfo(double value, int cid) throws IOException
            {
                String content = "JSD:%20" + value + "%20Channel:%20" + cid;
                String reportInfo = REPORT_SERVER_FORMAT + content;

                if (!allowLogOnServer)
                {
                    System.out.println("[JSDSensor]" + content);
                    return;
                }

                HttpConnection httpConn = (HttpConnection)Connector.open(reportInfo);
                httpConn.setRequestMethod(HttpConnection.POST);
                DataInputStream dis = httpConn.openDataInputStream();
                dis.close();
                httpConn.close();
            }

            /**
             * @brief 上报所有加速度信息到指定服务器
             * @param valueX X轴数据
             * @param valueY Y轴数据
             * @exception   当网络连接有问题时抛出IO异常
             */
            private void reportADCInfo(double valueX, double valueY) throws IOException
            {
                String content = "JSD:X:" + valueX + "%20Y:" + valueY;
                String reportInfo = REPORT_SERVER_FORMAT + content;

                if (!allowLogOnServer)
                {
                    System.out.println("[JSDSensor]" + content);
                    return;
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