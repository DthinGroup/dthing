import java.io.PrintStream;
import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

public class HelloDemo extends Applet
{
    public HelloDemo()
    {
        System.out.println("[HelloDemo] constructor is called!");
        justToBeCalled(true);
    }

    private void justToBeCalled(boolean paramBoolean)
    {
        if (paramBoolean)
        {
            System.out.println("[HelloDemo] justToBeCalled checked");
            return;
        }
        System.out.println("[HelloDemo] justToBeCalled non-checked");
    }

    public void cleanup()
    {
        System.out.println("[HelloDemo] cleanup is called!");
    }

    public void processEvent(Event paramEvent)
    {
    }

    public void startup()
    {
        System.out.println("[HelloDemo] startup is called!");
        System.gc();
        String str1 = System.getProperty("property1");
        System.out.println("[HelloDemo] key = property1 | value = " + str1);
        String str2 = System.getProperty("property2", "value2");
        System.out.println("[HelloDemo] key = property2 | value = " + str2);
        Runtime.getRuntime().gc();
    }
}