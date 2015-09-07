import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;
import java.lang.ArrayStoreException;

public class TckDemo extends Applet
{
    public TckDemo()
    {
        System.out.println("[TckDemo] constructor is called!");
    }

    public void cleanup()
    {
    }

    public void processEvent(Event paramEvent)
    {
    }

    public void startup()
    {
        System.out.println("[TckDemo] startup is called!");
        ArrayStoreTest();
        System.out.println("[TckDemo] startup is over!");
    }
    
    private void ArrayStoreTest(){
    	System.out.println("ArrayStoreTest");
    	try{
    		Object[] x = new String[3];
    		x[0] = new Integer(0);
    		System.out.println("not throw ArrayStoreTest");
    	} catch(ArrayStoreException e){
    		System.out.println("throw ArrayStoreTest");
    	}
    }
}
