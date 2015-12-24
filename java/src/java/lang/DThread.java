
package java.lang;

import com.yarlungsoft.util.Log;

/**
 * Thread reference implementation.
 */
public class DThread implements java.lang.Runnable {

    public static final int MAX_PRIORITY  = 10;
    public static final int MIN_PRIORITY  = 1;
    public static final int NORM_PRIORITY = 5;
    private static final long MAX_SLEEP   = Long.MAX_VALUE / 1000;
    private static final String TAG = "DThread";

    private Runnable mTarget;

    public DThread(Runnable target1) {
        this.mTarget = target1;
        if (target1 == null) {
            mTarget = this;
        }
    }

    public DThread() {
        this(null);
    }

    private native void start0();

    private static native void sleep0(long ms);

    private static native int activeCount0();

    private static native DThread currentThread0();

    private static native boolean isAlive0();

    public void run() {
        if (mTarget != null) {
            mTarget.run();
        } else {
            Log.log(TAG, "Thread Start Run Error!");
        }
    }

    public static int activeCount() {
        return activeCount0();
    }

    public static DThread currentThread() {
        return currentThread0();
    }

    public final int getPriority() {
        return DThread.NORM_PRIORITY;
    }

    /**
     * Test if the thread is alive.
     */
    public final boolean isAlive() {
        return isAlive0();
    }

    /**
     * Waits for this thread to die.
     */
    public final void join() throws java.lang.InterruptedException {
    }

    public final void setPriority(int newPriority) {
        if (newPriority < DThread.MIN_PRIORITY || newPriority > DThread.MAX_PRIORITY) {
            throw new IllegalArgumentException();
        }
    }

    public static void sleep(long millis) throws java.lang.InterruptedException {
        if (millis < 0) {
            throw new IllegalArgumentException();
        }
        if (millis > MAX_SLEEP) {
            millis = MAX_SLEEP;
        }
        sleep0(millis);
    }

    public synchronized void start() {
        start0();
    }

    public String toString() {
        return null; // String("DThread");
    }

    public static void yield() {
        // use sleep method to implement yield
        sleep0(1);
    }
}
