/**
 * Copyright 2012-2014 Yarlung Soft. All Rights Reserved.
 * $Id: $
 */
import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;

import iot.oem.adc.ADCManager;

/**
 * 气体传感器演示软件
 *
 * 测试次数：16
 * 间    隔：10秒左右
 * 端    口: 0
 */
public class QTSensor extends Applet {
    private static final int MAX_TEST_COUNT = 16;
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=qt&parmInfo=";
    private static int count = MAX_TEST_COUNT;
    private static boolean allowLogPrint = true;
    private static boolean allowRunning = true;

    public QTSensor() {
      // TODO Auto-generated constructor stub
    }

    public void cleanup() {
        allowRunning = false;
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        new Thread() {
            private ADCManager manager = null;
            private int result = -1;
            private String valueOfPort0 = null;

            public void run() {
                System.out.println("Starting QT sensor test ...");

                do {
                    //创建气体传感器实例
                    manager = ADCManager.getInstance();

                    if (manager == null) break;

                    //读取气体数据并解析
                    try {
                        int cid = 0;
                        //read from channel 0
                        result = manager.read(cid);
                        valueOfPort0 = convert(result);
                        System.out.println("Reading channelID[" + cid + "] with result[" + valueOfPort0 + "]");
                    } catch (IllegalArgumentException e) {
                        System.out.println("QT IllegalArgumentException:" + e);
                    } catch (IOException e) {
                        System.out.println("QT IOException:" + e);
                    }
                    //上报气体数据信息到服务器
                    try {
                        reportADCInfo(valueOfPort0, 0);
                    } catch (IOException e2) {
                        System.out.println("QT IOException: " + e2);
                    }

                    count--;
                    //休眠
                    try {
                        Thread.sleep(10000L);
                    } catch (InterruptedException e) {
                        System.out.println("QT InterruptedException: " + e);
                    }
                }while((count > 0) && allowRunning);
                //销毁气体传感器实例
                try {
                    manager.destroy();
                } catch (IOException e1) {
                    System.out.println("QT IOException: " + e1);
                }
                System.out.println("Exiting QT sensor test ...");
                notifyDestroyed();
            }

            /**
             * @brief 传感器数据转化
             * @param value 传感器返回数据
             * @return 输出的气体数据, 保留三位小数
             */
            private String convert(int value)
            {
                String hResult = value * 3 / 1000 + ".";
                int tail = (value * 3) % 1000;
                hResult = hResult + tail / 100;
                hResult = hResult + (tail % 100) / 10;
                hResult = hResult + (tail % 100) % 10;
                return hResult;
            }

            /**
             * @brief 上报气体信息到指定服务器
             * @param value 端口数据
             * @param cid   端口ID
             * @exception   当网络连接有问题时抛出IO异常
             */
            private void reportADCInfo(String value, int cid) throws IOException
            {
                String content = "QT:" + value + "v%20CID:%20" + cid;
                String reportInfo = REPORT_SERVER_FORMAT + content;

                if (allowLogPrint)
                {
                    System.out.println("[QT][Test Result]" + content);
                    //return;
                }

                URL url = new URL(reportInfo);
                HttpURLConnection httpConn = (HttpURLConnection)url.openConnection();
                httpConn.setRequestMethod(HttpURLConnection.POST);
                InputStream dis = httpConn.getInputStream();
                dis.close();
                httpConn.disconnect();
            }
        }.start();
    }
}