
package java.lang;
//package os.thread;

/*
  while(AsyncIO.loop())
  {
    res = func(param);
  }
 */
public class AsyncIO
{
    public static final int AIO_IDLE    = 0;  //idle,original state
    public static final int AIO_BUSY    = 1;  //io in progress (thread still runnable) [support?]    
    public static final int AIO_ASLEEP  = 2;  //io in progress (thread is now suspended)
    public static final int AIO_FIRST   = 3;  //first in,used to distinguish first call,like done0 in jbed
    public static final int AIO_DONE    = 4;  //done (io has completed)
    public static final int AIO_TIMEOUT = 5;  //like DONE, but indicates the IO timed out
    
    public static boolean loop()
    {
        int  state = getCurNotifierState();
        
        switch(state)
        {
            case AIO_IDLE:
            {
                
                setCurNotifierState(AIO_FIRST);
            }
            return true;   //after return,async_firstCall at once. continue the AsyncIO_loop()
            
            case AIO_BUSY:
            //not support the case for now!!!
            return false;
            
            case AIO_ASLEEP:
            {
                waitSignalOrTimeOut();
                //when return from waitSignalOrTimeOut,the state is AIO_DONE or AIO_TIMEOUT
                //state = getCurNotifierState();
            }
            return true; //continue the AsyncIO_loop()
            
            case AIO_FIRST:  //not use asyn call,func completes in one call
            {
                setCurNotifierState(AIO_IDLE);
            }
            return false;  //break the AsyncIO_loop()
            
            case AIO_DONE:
            case AIO_TIMEOUT:
            {
                setCurNotifierState(AIO_IDLE);
            }
            return false;   //break the AsyncIO_loop()

            default:
            //system state error!!!
            return false;
            
        }
    }
    
    /*get state of current notifier of this(current) thread*/
    private native static int getCurNotifierState();
    
    private native static int setCurNotifierState(int state);
    
    //give up schduler,wait signal or time out to wake
    //this func is blocked,when return it means the thread is waked up again
    //return true: the wake source is signal
    //return false: the wake source is time out
    private native static void waitSignalOrTimeOut();
    
}