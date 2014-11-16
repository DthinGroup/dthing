/**
 * Copyright 2012-2014 Yarlung Soft. All Rights Reserved.
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
 * 红外测距传感器演示软件
 *
 * 测试次数：100000
 * 间    隔：1秒左右
 * 端    口: 0
 */
public class InfraredDistanceSensor extends Applet {
    private static final int MAX_TEST_COUNT = 100000;
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";
    private static int count = MAX_TEST_COUNT;
    private static boolean allowLogPrint = true;

    public InfraredDistanceSensor() {
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
            private int cid = 1;

            public void run() {
                System.out.println("Starting infrared distance sensor test ...");

                do {
                    //创建红外测距传感器实例
                    manager = ADCManager.getInstance();

                    if (manager == null) break;

                    //读取数据并解析上报
                    try {
                        result = manager.read(cid);
                        if (result > 0)
                        {
                            reportADCInfo(result, cid);
                        }
                    } catch (IllegalArgumentException e) {
                        System.out.println("IRDistance IllegalArgumentException:" + e);
                    } catch (IOException e) {
                        System.out.println("IRDistance IOException:" + e);
                    }

                    count--;
                    //休眠
                    try {
                        Thread.sleep(500L);
                    } catch (InterruptedException e) {
                        System.out.println("IRDistance InterruptedException: " + e);
                    }
                }while(count > 0);
                //销毁红外测距传感器实例
                try {
                    manager.destroy();
                } catch (IOException e1) {
                    System.out.println("IRDistance IOException: " + e1);
                }
                System.out.println("Exiting IRDistance test ...");
            }

            /**
             * @brief 上报红外测距信息到指定服务器
             * @param value 端口数据
             * @param cid   端口ID
             * @exception   当网络连接有问题时抛出IO异常
             */
            private void reportADCInfo(double value, int cid) throws IOException
            {
                String content = "Distance:" + value + "%20CID:%20" + cid;
                String reportInfo = REPORT_SERVER_FORMAT + content;

                if (allowLogPrint)
                {
                    System.out.println("[IRDistance][Test Result]" + content);
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