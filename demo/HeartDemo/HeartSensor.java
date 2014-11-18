/**
 * Copyright 2012-2014 Yarlung Soft. All Rights Reserved.
 * $Id: $
 */
import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;
import java.io.DataInputStream;
import java.io.IOException;

import iot.oem.adc.ADCManager;

/**
 * ������������ʾ���
 *
 * ���Դ�����96000 ���������50����  �� ��: 0
 */
public class HeartSensor extends Applet {
    private static final int DEFAULT_ADC_CHANNEL = 0;
    private static final int MAX_TEST_COUNT = 96000;
    private static final int MAX_BUFFER_COUNT = 1200;
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=csxl&parmInfo=";
    private static int count = MAX_TEST_COUNT;
    private static int reportDataIndex = 0;
    private static int readDataCount = 0;
    private static boolean allowLogPrint = true;
    private static int[] buffer = new int[MAX_BUFFER_COUNT];

    public HeartSensor() {
        // TODO Auto-generated constructor stub
    }

    protected static void resetBuffer()
    {
        readDataCount = 0;
        reportDataIndex = 0;
        buffer = new int[MAX_BUFFER_COUNT];
    }

    public void cleanup() {
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        //ADC�����߳�
        new Thread() {
            private long lastScheduledTime = 0;
            private ADCManager manager = null;

            public void run() {
                do {
                    long currentTime = System.currentTimeMillis();
                    if ((currentTime - lastScheduledTime) >= 50)
                    {
                        lastScheduledTime = currentTime;
                        read();
                    }
                } while(count > 0);
            }

            public void read() {
                System.out.println("Starting Heart sensor read task ...");

                // ��������������ʵ��
                manager = ADCManager.getInstance();

                if (manager == null)
                    return;

                if (count <= 0)
                {
                    // ��������������ʵ��
                    try {
                        manager.destroy();
                    } catch (IOException e1) {
                        System.out.println("Heart IOException: " + e1);
                    }
                }

                // ��ȡ�������ݲ�����
                try {
                    if (readDataCount < MAX_BUFFER_COUNT)
                    {
                        buffer[readDataCount] = manager.read(DEFAULT_ADC_CHANNEL);
                        readDataCount++;
                        count--;
                    }
                } catch (IllegalArgumentException e) {
                    System.out.println("Heart IllegalArgumentException:" + e);
                } catch (IOException e) {
                    System.out.println("Heart IOException:" + e);
                }

                System.out.println("Exiting Heart sensor read task ...");
            }
        }.start();

        new Thread() {
            public void run() {
                System.out.println("Starting report adc data ...");

                do {
                    if ((readDataCount == MAX_BUFFER_COUNT)
                        && (reportDataIndex == readDataCount))
                    {
                        HeartSensor.resetBuffer();
                    }

                    if (reportDataIndex < readDataCount)
                    {
                        try {
                            double value = buffer[reportDataIndex]*3.5/1000;
                            reportDataIndex++;
                            reportADCInfo(value, DEFAULT_ADC_CHANNEL);
                        } catch (IOException e) {
                            System.out.println("Heart IOException:" + e);
                        }
                    }
                } while(count > 0);
            }
            /**
             * @brief �ϱ�������Ϣ��ָ��������
             * @param value �˿�����
             * @param cid �˿�ID
             * @exception ����������������ʱ�׳�IO�쳣
             */
            private void reportADCInfo(double value, int cid) throws IOException {
                String content = "Heart:" + value + " CID:" + cid;
                String reportInfo = REPORT_SERVER_FORMAT + value;

                if (allowLogPrint) {
                    System.out.println("[Heart][Test Result]" + content);
                }
            }
        }.start();
    }
}