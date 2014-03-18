package com.yarlungsoft.ams;

import jp.co.aplix.event.Applet;
import jp.co.aplix.event.PackageAccess;

public class AppletState implements Runnable {

    private static final boolean DEBUG = AmsConfig.debug();

    static final int ERROR = -1,
                     UNINITIALIZED = 0,
                     INITIALIZED = 1,
                     STARTING = 2,
                     STARTED = 3,
                     PAUSE_PENDING = 4,
                     PAUSING = 5,
                     PAUSED = 6,
                     RESUME_PENDING = 7,
                     RESUMING = 8,
                     DESTROY_PENDING = 9,
                     DESTROYING = 10,
                     INVALID = 11;

    public static final String stateStrings[] = {
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

    private int state;
    private int requestState;
    Applet app;
    Thread daemonThread;

    protected AppletState() {
        this(null);
    }

    public AppletState(Applet a) {
        this.app = a;
        state = UNINITIALIZED;
        requestState = INVALID;
        Scheduler.register(this);
    }

    int getState() {
        return state;
    }

    boolean gotoState(int newState) {
        if (DEBUG)
            System.out.println("goto State: " + stateStrings[newState+1]);

        synchronized (Scheduler.mutex) {

            if (newState < ERROR && newState > INVALID) {
                return false;
            }

            setState(newState);

            if (newState == STARTING || newState == PAUSING ||
                newState == RESUMING || newState == DESTROYING) {
                if (daemonThread == null) {
                    daemonThread = new Thread(this);
                    daemonThread.start();
                }
            }
        }
        return true;
    }

    void setState(int newState) {
        if (state == newState) {
            if (DEBUG) {
                System.out.println("setState: same state = " + stateStrings[newState+1]);
            }
            return;
        }
        if (DEBUG) {
            System.out.println("setState: newState = " + stateStrings[newState+1]);
        }

        state = newState;
    }

    void setRequestState(int nState) {
        requestState = nState;
    }

    int getRequestState() {
        return requestState;
    }


    public void notifyDestroyed() {
        if (DEBUG)
            System.out.println("notifyDestroyed for " + app.toString());

        synchronized(Scheduler.mutex) {
            setRequestState(DESTROY_PENDING);
            if (daemonThread == null) {
                Scheduler.mutex.notify();
            }
        }
    }


    public void notifypaused() {
        if (DEBUG)
            System.out.println("notifypaused for " + app.toString());

        synchronized(Scheduler.mutex) {
            setRequestState(PAUSE_PENDING);
            if (daemonThread == null) {
                Scheduler.mutex.notify();
            }
        }
    }

    public void resumeRequest() {
        if (DEBUG)
            System.out.println("resumeRequest for " + app.toString());

        synchronized(Scheduler.mutex) {
            setRequestState(RESUME_PENDING);
            if (daemonThread == null) {
                Scheduler.mutex.notify();
            }
        }
    }

    protected void startup() {
        PackageAccess.Application_startup(app);
    }

    protected void cleanup() {
        PackageAccess.Application_cleanup(app);
    }

    protected void processEvent() {
        PackageAccess.Application_processEvent(app, null);
    }

    public void run() {

        if (state != STARTING && state != PAUSING &&
            state != RESUMING && state != DESTROYING) {
            //nothing to do here.
            daemonThread = null;
            return;
        }

        try {
            switch (state) {

                case STARTING:
                    startup();
                    setState(STARTED);
                    break;

                case DESTROYING:
                	processEvent();
                    setState(UNINITIALIZED);
                    break;

                default:
                    break;
            }
        } catch (Throwable t) {
            if (DEBUG)
                System.out.println("Caught unexpected throwable: " + t);
            //TODO: report user error info.
            setState(ERROR);
        } finally {
            synchronized(Scheduler.mutex) {
                daemonThread = null;
                Scheduler.mutex.notify();
            }
        }
    }
}
