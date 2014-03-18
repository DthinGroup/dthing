
package java.lang;
//package os.thread;


public class DThread implements java.lang.Runnable 
{

    public static final int MAX_PRIORITY  = 10;
    public static final int MIN_PRIORITY  = 1;
    public static final int NORM_PRIORITY = 5;
    private static final long MAX_SLEEP   = Long.MAX_VALUE / 1000;

    private Runnable target;
    
    
    private native void start0();

    private native static void    sleep0(long ms);
    private native static int     activeCount0();
    private native static DThread currentThread0();
    private native static boolean isAlive0();
    
    public DThread (Runnable target1) 
    {
        this.target = target1;
        if (target1 == null) 
        {
            target = this;
        }
    }

    public DThread () 
    {
        this(null);
    }

    public void run () 
    {
        if (target != null) 
        {
            target.run();
        }
    	else
    	{
    	    System.out.println("Fuck!");
    	}
    }
    
    public static int activeCount () 
    {
        return activeCount0();
    }
    
    public static DThread currentThread () 
    {
        return currentThread0();
    }
    
    public final int getPriority () 
    {
        return DThread.NORM_PRIORITY;
    }
    
    /*Test if the thread is alive*/
    public final boolean isAlive ()
    {
        return isAlive0();
    }
    
    /*waits for this thread to die*/
    public final void join () throws java.lang.InterruptedException 
    {
        
    }    
    
    public final void setPriority (int newPriority) 
    {
        if (newPriority < DThread.MIN_PRIORITY || newPriority > DThread.MAX_PRIORITY) 
        {
            throw new IllegalArgumentException();
        }
    
    }
    
    public static void sleep (long millis) throws java.lang.InterruptedException 
    {
        if (millis < 0) 
            throw new IllegalArgumentException();
        if (millis > MAX_SLEEP) 
        { 
            millis = MAX_SLEEP;
        }
        sleep0(millis);
    }

    public synchronized void start() 
    {
        start0();
    }
    
    public String toString () 
    {
        return null;//String("DThread");
    }
    
    public static void yield () 
    {
        //use sleep method to implement yield
        sleep0(1);
    }
}
