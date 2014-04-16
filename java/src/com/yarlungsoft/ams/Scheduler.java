
package com.yarlungsoft.ams;

import com.yarlungsoft.util.Log;
import jp.co.aplix.event.Applet;

/**
 * The Scheduler starts and controls Applets through the lifecycle states.
 */
public final class Scheduler {

    protected static final Object MUTEX = new Object();

    private static final String TAG = "Scheduler";

    /* Current running APP */
    private static Applet sApp;

    private static AppletState sAppState;

    Scheduler() {
        Log.amsLog(TAG, "Scheduler constructed!");
    }

    private static boolean appInit(String clsName, String appPath) {
        boolean ret = false;
        try {
            Log.amsLog(TAG, ">>>>>> clsName: " + clsName);
            Class<?> c = Class.forName(clsName);
            sApp = (Applet) c.newInstance();

            sAppState.setState(AppletState.INITIALIZED);
            ret = true;
            reportRunningObject(sApp);
        } catch (Throwable t) {
            Log.amsLog(TAG, "sApp init with throwable : " + t.toString());
        }
        return ret;
    }

    protected static void schedule(AppletContent appContent) {
        appInit(appContent.getAppMainClass(), appContent.getAppFullPathName());

        synchronized (MUTEX) {
            try {
                while (sAppState.getState() > AppletState.UNINITIALIZED) {
                    switch (sAppState.getState()) {
                    case AppletState.INITIALIZED:
                        sAppState.gotoState(AppletState.STARTING);
                        break;

                    case AppletState.STARTED:
                        if (sAppState.getRequestState() == AppletState.PAUSE_PENDING) {
                            sAppState.gotoState(AppletState.PAUSING);
                            sAppState.setRequestState(AppletState.INVALID);
                        } else if (sAppState.getRequestState() == AppletState.DESTROY_PENDING) {
                            sAppState.gotoState(AppletState.DESTROYING);
                            sAppState.setRequestState(AppletState.INVALID);
                        }
                        break;

                    case AppletState.PAUSED:
                        if (sAppState.getRequestState() == AppletState.RESUME_PENDING) {
                            sAppState.gotoState(AppletState.RESUMING);
                            sAppState.setRequestState(AppletState.INVALID);
                        } else if (sAppState.getRequestState() == AppletState.DESTROY_PENDING) {
                            sAppState.gotoState(AppletState.DESTROYING);
                            sAppState.setRequestState(AppletState.INVALID);
                        }
                        break;

                    case AppletState.STARTING:
                    case AppletState.PAUSING:
                    case AppletState.RESUMING:
                        if (sAppState.getRequestState() == AppletState.DESTROY_PENDING) {
                            sAppState.gotoState(AppletState.DESTROYING);
                            sAppState.setRequestState(AppletState.INVALID);
                        }
                        break;

                    case AppletState.DESTROYING:
                        // nothing to do
                        sAppState.setRequestState(AppletState.INVALID);
                        break;

                    default:
                        sAppState.setState(AppletState.ERROR);
                        continue;
                    }
                    Log.amsLog(TAG, "schedule MUTEX.wait.1");
                    MUTEX.wait();
                    Log.amsLog(TAG, "schedule MUTEX.wait.2");
                }
            } catch (Throwable t) {
                t.printStackTrace();
            }
        }
    }

    protected static Applet getCurrentRunningApp() {
        return sApp;
    }

    protected static void register(AppletState as) {
        sAppState = as;
    }

    protected static void deregister(AppletState as) {
        sAppState = null;
    }
    
    native static void reportState(int state);
    native static void reportRunningObject(Object obj);
}
