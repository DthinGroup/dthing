import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;
import java.io.DataInputStream;
import java.io.IOException;

import iot.oem.adc.ADCManager;

public class ADCSensor extends Applet {
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=qt&parmInfo=";

    public ADCSensor() {
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
            private int valueOfPort0 = 0;
            private int valueOfPort7 = 0;
            private boolean reportPort0 = true;
            private boolean allowLogOnServer = false;

            public void run() {
                System.out.println("[ADCDemo]Start ADC sensor test");

                do {
                    manager = ADCManager.getInstance();

                    if (manager == null) break;

                    try {
                        if (reportPort0 == true)
                        {
                            int cid = 0;
                            //read from channel 0
                            result = manager.read(cid);
                            valueOfPort0 = convert(result);
                            System.out.println("[ADCDemo] read channelID[" + cid + "] with result[" + valueOfPort0 + "]");
                        }
                        else
                        {
                            //read from channel 7
                            int cid = 7;
                            result = manager.read(cid);
                            valueOfPort7 = convert(result);
                            System.out.println("[ADCDemo] read channelID[" + cid + "] with result[" + valueOfPort7 + "]");
                        }
                    } catch (IllegalArgumentException e) {
                        System.out.println("ADCDemo IllegalArgumentException:" + e);
                    } catch (IOException e) {
                        System.out.println("ADCDemo IOException:" + e);
                    }

                    try {
                        if (reportPort0 == true)
                        {
                            reportADCInfo(valueOfPort0, 0);
                            reportPort0 = false;
                        }
                        else
                        {
                            reportADCInfo(valueOfPort7, 7);
                            reportPort0 = true;
                        }
                    } catch (IOException e2) {
                        System.out.println("IOException: " + e2);
                    }

                    try {
                        Thread.sleep(5000L);
                    } catch (InterruptedException e) {
                        System.out.println("InterruptedException: " + e);
                    }
                }while(true);

                try {
                    manager.destroy();
                } catch (IOException e1) {
                    System.out.println("IOException: " + e1);
                }
                System.out.println("[ADCDemo] End ADC sensor test");
            }

            private int convert(int value)
            {
                int zero_g = 512000;
                int scale = 1023;
                int result = 0;
                result = (value * 1000 - zero_g) / scale;
                return result;
            }

            private void reportADCInfo(int value, int cid) throws IOException
            {
                String content = "ADC:%20" + value + "%20Channel:%20" + cid;
                String reportInfo = REPORT_SERVER_FORMAT + content;

                if (!allowLogOnServer)
                {
                    System.out.println("[ADCSensor]" + content);
                }
            }
        }.start();
    }
}