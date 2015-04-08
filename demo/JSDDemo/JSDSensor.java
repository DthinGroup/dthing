/**
 * Copyright 2012-2013 Yarlung Soft. All Rights Reserved.
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
 * ���ٶȴ�������ʾ���
 *
 * ���Դ�����16
 * ��    ����10������
 * ��    ��: 0��7
 */
public class JSDSensor extends Applet {
    private static final int MAX_TEST_COUNT = 18;
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=jsd&parmInfo=";
    private static int count = MAX_TEST_COUNT;
    private static boolean allowLogPrint = true;
    private static boolean allowRunning = true;

    public JSDSensor() {
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
            private double valueOfPort0 = 0;
            private double valueOfPort7 = 0;

            public void run() {
                System.out.println("[ADCDemo]Start ADC sensor test");

                do {
                    //�������ٶȴ�����ʵ��
                    manager = ADCManager.getInstance();

                    if (manager == null) break;

                    //������ȡ�˿�0��7�����ݲ�����
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
                    //�ϱ��˿ڶ�Ӧ���ݵ�������
                    try {
                        reportADCInfo(valueOfPort0, valueOfPort7);
                    } catch (IOException e2) {
                        System.out.println("IOException: " + e2);
                    }

                    count--;
                    //����
                    try {
                        Thread.sleep(10000L);
                    } catch (InterruptedException e) {
                        System.out.println("InterruptedException: " + e);
                    }
                } while((count > 0) && allowRunning);
                //���ٴ�����ʵ��
                try {
                    manager.destroy();
                } catch (IOException e1) {
                    System.out.println("IOException: " + e1);
                }
                System.out.println("[JSDDemo] End ADC sensor test");
                notifyDestroyed();
            }

            /**
             * @brief ����������ת��
             * @param value ��������������
             * @return ����ļ��ٶ�����
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
             * @brief �ϱ����м��ٶ���Ϣ��ָ��������
             * @param valueX X������
             * @param valueY Y������
             * @exception   ����������������ʱ�׳�IO�쳣
             */
            private void reportADCInfo(double valueX, double valueY) throws IOException
            {
                String content = "JSD:X:" + valueX + "%20Y:" + valueY;
                String reportInfo = REPORT_SERVER_FORMAT + content;

                if (allowLogPrint)
                {
                    System.out.println("[JSDSensor]" + content);
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