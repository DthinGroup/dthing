package com.yarlungsoft.ams;

import jp.co.aplix.event.Applet;

public final class Scheduler {

    private static final boolean DEBUG = AmsConfig.debug();

    static Object mutex = new Object();

    /* Current running APP */
    static Applet app;

    static AppletContent curRunningAppInfo;

    static AppletState appState;

    Scheduler() {
        if (DEBUG)
            System.out.println("Scheduler constructed!");
    }
    
    static boolean appInit(String clsName, String appPath) {

        boolean ret = false;
        try {
            System.out.println(">>>>>> clsName: " + clsName);
            Class<?> c = Class.forName(clsName);
            app = (Applet)c.newInstance();

            appState.setState(AppletState.INITIALIZED);
            ret = true;

        } catch (Throwable t) {
            if (DEBUG)
                System.out.println("app init with throwable : " + t.toString());
        }
        return ret;
    }

    static void schedule(AppletContent appContent) {

        curRunningAppInfo = appContent;
        appInit(appContent.getAppMainClass(), appContent.getAppFullPathName());

        synchronized (mutex) {

            try {

               while (appState.getState() > AppletState.UNINITIALIZED) {

                    switch (appState.getState()) {

                        case AppletState.INITIALIZED:
                            appState.gotoState(AppletState.STARTING);
                            break;

                        case AppletState.STARTED:
                            if (appState.getRequestState() == AppletState.PAUSE_PENDING) {
                                appState.gotoState(AppletState.PAUSING);
                                appState.setRequestState(AppletState.INVALID);
                            } else if (appState.getRequestState() == AppletState.DESTROY_PENDING) {
                                appState.gotoState(AppletState.DESTROYING);
                                appState.setRequestState(AppletState.INVALID);
                            }
                            break;

                        case AppletState.PAUSED:
                            if (appState.getRequestState() == AppletState.RESUME_PENDING) {
                                appState.gotoState(AppletState.RESUMING);
                                appState.setRequestState(AppletState.INVALID);
                            } else if (appState.getRequestState() == AppletState.DESTROY_PENDING) {
                                appState.gotoState(AppletState.DESTROYING);
                                appState.setRequestState(AppletState.INVALID);
                            }
                            break;

                        case AppletState.STARTING:
                        case AppletState.PAUSING:
                        case AppletState.RESUMING:
                            if (appState.getRequestState() == AppletState.DESTROY_PENDING) {
                                appState.gotoState(AppletState.DESTROYING);
                                appState.setRequestState(AppletState.INVALID);
                            }
                            break;

                        case AppletState.DESTROYING:
                            //nothing todo
                            appState.setRequestState(AppletState.INVALID);
                            break;

                        default:
                            appState.setState(AppletState.ERROR);
                            continue;
                    }
                    mutex.wait();
                }
            } catch (Throwable t) {
                t.printStackTrace();
            }
        }
    }

    static Applet getCurrentRunningApp() {
        return app;
    }
    
    static void register(AppletState as) {
        appState = as;
    }

    static void deregister(AppletState as) {
        appState = null;
    }

    static AppletContent getCurrentRunningAppInfo() {
        return curRunningAppInfo;
    }

}
