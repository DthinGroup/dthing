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
                } while(count > 0);
                //���ٴ�����ʵ��
                try {
                    manager.destroy();
                } catch (IOException e1) {
                    System.out.println("IOException: " + e1);
                }
                System.out.println("[JSDDemo] End ADC sensor test");
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
             * @brief �ϱ����ٶ���Ϣ��ָ��������
             * @param value �˿�����
             * @param cid   �˿�ID
             * @exception   ����������������ʱ�׳�IO�쳣
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
             * @brief �ϱ����м��ٶ���Ϣ��ָ��������
             * @param valueX X������
             * @param valueY Y������
             * @exception   ����������������ʱ�׳�IO�쳣
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