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
 * ���崫������ʾ���
 *
 * ���Դ�����16
 * ��    ����10������
 * ��    ��: 0
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
                    //�������崫����ʵ��
                    manager = ADCManager.getInstance();

                    if (manager == null) break;

                    //��ȡ�������ݲ�����
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
                    //�ϱ�����������Ϣ��������
                    try {
                        reportADCInfo(valueOfPort0, 0);
                    } catch (IOException e2) {
                        System.out.println("QT IOException: " + e2);
                    }

                    count--;
                    //����
                    try {
                        Thread.sleep(10000L);
                    } catch (InterruptedException e) {
                        System.out.println("QT InterruptedException: " + e);
                    }
                }while((count > 0) && allowRunning);
                //�������崫����ʵ��
                try {
                    manager.destroy();
                } catch (IOException e1) {
                    System.out.println("QT IOException: " + e1);
                }
                System.out.println("Exiting QT sensor test ...");
                notifyDestroyed();
            }

            /**
             * @brief ����������ת��
             * @param value ��������������
             * @return �������������, ������λС��
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
             * @brief �ϱ�������Ϣ��ָ��������
             * @param value �˿�����
             * @param cid   �˿�ID
             * @exception   ����������������ʱ�׳�IO�쳣
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