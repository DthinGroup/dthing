import java.io.PrintStream;
import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;
import java.net.http.HttpURLConnection;
import java.net.http.URL;
import java.io.IOException;
import java.io.InputStream;

public class HelloDemo extends Applet
{
    private static boolean allowLogPrint = true;
    private static final String REPORT_SERVER_FORMAT = "http://42.121.18.62:8080/dthing/ParmInfo.action?saveDataInfo&saveType=log&parmInfo=";

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

	 try {
            reportTestInfo("[Hello] Test.OK");
	 } catch (IOException  e) {
	     System.out.println("[Hello] IOException:" + e);
	 }
    }

    private void reportTestInfo(String info) throws IOException {
        String content = "Hello:" + info;
        String reportInfo = REPORT_SERVER_FORMAT + info;

        if (allowLogPrint) {
            System.out.println("[Hello][Test Result]" + content);
        }

        URL url = new URL(reportInfo);
        HttpURLConnection httpConn = (HttpURLConnection)url.openConnection();
        httpConn.setRequestMethod(HttpURLConnection.POST);
        InputStream dis = httpConn.getInputStream();
        dis.close();
        httpConn.disconnect();
    }
}