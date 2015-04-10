import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;
import java.io.IOException;
import iot.oem.NativeAPI.NativeAPIManager;

public class KeyPadLed extends Applet {

    private boolean exit = false;

    public KeyPadLed() {
      // TODO Auto-generated constructor stub
    }

    public void cleanup() {
        exit = true;
        NativeAPIManager am = NativeAPIManager.getInstance();
        try{
            am.destroy(NativeAPIManager.API_ID_1);
        }catch(IOException e){
        }
    }

    public void processEvent(Event paramEvent) {
    }

    public void startup() {
            new Thread(){
            public void run()
            {
                boolean test = false;
                try
                {
                    NativeAPIManager am = NativeAPIManager.getInstance();
                    for (int i = 0; i < 100; i++)
                    {
                        if(exit==true) break;
                        Thread.sleep(200);
                        if (test)
                        {
                            am.call(NativeAPIManager.API_ID_1,test);
                            test = false;
                        } else{
                            am.call(NativeAPIManager.API_ID_1,test);
                            test = true;
                        }
                    }
                }
                catch (IllegalArgumentException illegalargumentexception)
                {
                    illegalargumentexception.printStackTrace();
                }
                catch (IOException ioexception)
                {
                    ioexception.printStackTrace();
                }
                catch (InterruptedException interruptedexception)
                {
                    interruptedexception.printStackTrace();
                }
            }
        }.start();
     }
}