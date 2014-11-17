import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;
import java.io.DataInputStream;
import java.io.IOException;

import iot.oem.spk.*;
import iot.oem.ear.*;

public class AudioTest extends Applet {
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";

    public AudioTest() {

    }

    public void cleanup() {
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
        new Thread() {
            private SpeakerManager speaker = null;
            private EarManager ear = null;
            private boolean allowLogPrint = true;

            public void run() {
                System.out.println("[AudioDemo]Start SPK/EAR test");
                speaker = SpeakerManager.getInstance();
                ear = EarManager.getInstance();

                try {
                    for (int i = 0; i < 20; i++)
                    {
                        if (i % 2 == 1)
                        {
                            if(ear.getStatus() != true)
                            {
                            ear.setStatus(true);
                            reportTestInfo("Ear", "opened");
                            }
                        } else {
                            if(speaker.getStatus() != true)
                            {
                            speaker.setStatus(true);
                            reportTestInfo("Speaker", "opened");
                            }
                        }

                        try {
                            Thread.sleep(10000L);
                        } catch (InterruptedException e) {
                            e.printStackTrace();
                            System.out.println("[Audio Demo] sleep exception:" + e);
                        }
                    }
                    ear.close();
                    speaker.close();
                    reportTestInfo("Ear/Spk", "closed");
                } catch (IOException e) {
                    e.printStackTrace();
                    System.out.println("[Audio Demo] met IOException " + e);
                    try {
                        reportTestInfo("Ear/spk", "Exception:" + e);
                    } catch (IOException e1) {
                        // TODO Auto-generated catch block
                        e1.printStackTrace();
                    }
                }

                System.out.println("[AudioDemo] End SPK/EAR test");
            }

            private void reportTestInfo(String name, String msg) throws IOException
            {
                String content = name + ":" + msg;
                String reportInfo = REPORT_SERVER_FORMAT + content;

                if (allowLogPrint)
                {
                    System.out.println("[" + name + "]" + content);
                }
            }
        }.start();
    }
}