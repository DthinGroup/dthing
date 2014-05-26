
package com.yarlungsoft.ams;

import com.yarlungsoft.util.Log;

import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.PackageAccess;

/**
 * Applet state management.
 */
public class AppletState implements Runnable {

    protected static final int ERROR = -1;
    protected static final int UNINITIALIZED = 0;
    protected static final int INITIALIZED = 1;
    protected static final int STARTING = 2;
    protected static final int STARTED = 3;
    protected static final int PAUSE_PENDING = 4;
    protected static final int PAUSING = 5;
    protected static final int PAUSED = 6;
    protected static final int RESUME_PENDING = 7;
    protected static final int RESUMING = 8;
    protected static final int DESTROY_PENDING = 9;
    protected static final int DESTROYING = 10;
    protected static final int INVALID = 11;

    private static final String TAG = "AppletState";

    private static final String[] STATE_STRINGS = {
        "ERROR",
        "UNINITIALIZED",
        "INITIALIZED",
        "STARTING",
        "STARTED",
        "PAUSE_PENDING",
        "PAUSING",
        "PAUSED",
        "RESUME_PENDING",
        "RESUMING",
        "DESTROY_PENDING",
        "DESTROYING",
        "INVALID",
    };

    private int mState;
    private int mRequestState;
    private Applet mApp;
    private Thread mDaemonThread;

    protected AppletState() {
        this(null);
    }

    public AppletState(Applet a) {
        mApp = a;
        mState = UNINITIALIZED;
        mRequestState = INVALID;
        Scheduler.register(this);
    }

    protected int getState() {
        return mState;
    }

    private String getStateString(int state) {
        state += 1;
        if (state < 0 || state >= STATE_STRINGS.length) {
            return "unknown state " + (state - 1);
        }
        return STATE_STRINGS[state];
    }

    protected boolean gotoState(int newState) {
        Log.amsLog(TAG, "goto State: " + getStateString(newState));

        synchronized (Scheduler.MUTEX) {
            if (newState < ERROR && newState > INVALID) {
                return false;
            }

            setState(newState);

            if (newState == STARTING || newState == PAUSING || newState == RESUMING
                    || newState == DESTROYING) {
                if (mDaemonThread == null) {
                    mDaemonThread = new Thread(this);
                    mDaemonThread.start();
                }
            }
        }
        return true;
    }

    protected void setState(int newState) {
        if (mState == newState) {
            Log.amsLog(TAG, "setState: same mState = " + getStateString(newState));
            return;
        }
        Log.amsLog(TAG, "setState: newState = " + getStateString(newState));
        mState = newState;
        Scheduler.reportState(mState);
    }

    protected void setRequestState(int nState) {
        mRequestState = nState;
    }

    protected int getRequestState() {
        return mRequestState;
    }

    public void notifyDestroyed() {
        Log.amsLog(TAG, "notifyDestroyed");

        synchronized (Scheduler.MUTEX) {
            setRequestState(DESTROY_PENDING);
            if (mDaemonThread == null) {
                Scheduler.MUTEX.notify();
            }
        }
    }

    public void notifypaused() {
        Log.amsLog(TAG, "notifypaused for " + mApp.toString());

        synchronized (Scheduler.MUTEX) {
            setRequestState(PAUSE_PENDING);
            if (mDaemonThread == null) {
                Scheduler.MUTEX.notify();
            }
        }
    }

    public void resumeRequest() {
        Log.amsLog(TAG, "resumeRequest for " + mApp.toString());

        synchronized (Scheduler.MUTEX) {
            setRequestState(RESUME_PENDING);
            if (mDaemonThread == null) {
                Scheduler.MUTEX.notify();
            }
        }
    }

    protected void startup() {
        PackageAccess.Application_startup(mApp);
    }

    protected void cleanup() {
        PackageAccess.Application_cleanup(mApp);
    }

    protected void processEvent() {
        PackageAccess.Application_processEvent(mApp, null);
    }

    public void run() {
        if (mState != STARTING && mState != PAUSING && 
        		mState != RESUMING && mState != DESTROYING) {
            // nothing to do here.
            mDaemonThread = null;
            Log.amsLog(TAG, "AMS thread, mState = " + mState);
            return;
        }

        try {
            switch (mState) {
            case STARTING:            	
                startup();
                setState(STARTED);
                break;

            case DESTROYING:
            	cleanup();
                setState(UNINITIALIZED);
                break;

            default:
                Log.amsLog(TAG, "AMS Thread, default mState = " + mState);
                break;
            }
        } catch (Throwable t) {
            Log.amsLog(TAG, "Caught unexpected throwable:", t);
            // TODO: report user error info.
            setState(ERROR);
        } finally {
            synchronized (Scheduler.MUTEX) {
                mDaemonThread = null;
                Scheduler.MUTEX.notify();
            }
        }
    }
}
