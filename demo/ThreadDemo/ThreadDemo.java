import java.io.PrintStream;
import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

public class ThreadDemo extends Applet
{
    private static int count = 1200;

    public ThreadDemo()
    {
        System.out.println("[ThreadDemo] constructor is called!");
    }

    public void cleanup()
    {
        System.out.println("[ThreadDemo] cleanup is called!");
    }

    public void processEvent(Event paramEvent)
    {
    }

    public void startup()
    {
        System.out.println("[ThreadDemo] startup is called!");
        //Thread-1
        new Thread() {
            private int lastCount = count;
            public void run() {
                System.out.println("[Thread-1] === Start of thread 1 ===");
                do {
                    if (lastCount > count)
                    {
                        lastCount = count;
                        System.out.println("[Thread-1] === read native data ===");
                    }
                } while(count > 0);
            }
        }.start();
        //Thread-2
        new Thread() {
            private long lastScheduledTime = 0;
            public void run() {
                System.out.println("[Thread-2] === Start of thread 2 ===");

                do {
                    long currentTime = System.currentTimeMillis();

                    if ((currentTime - lastScheduledTime) > 1000)
                    {
                        System.out.println("[Thread-2] === count " + count + " ===");
                        lastScheduledTime = currentTime;
                        count--;
                    }
                } while(count > 0);
            }
        }.start();
    }
}
