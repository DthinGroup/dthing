
package jp.co.cmcc.event;

import com.yarlungsoft.util.Log;

/**
 * Package access class for application.
 */
public class PackageAccess {

    private static final String TAG = "PackageAccess";

    PackageAccess() {
        // nothing to do
    }

    public static void Application_startup(Applet a) {
        Log.log(TAG, "Application_startup");
        a.startup();
    }

    public static void Application_cleanup(Applet a) {
        Log.log(TAG, "Application_cleanup");
        a.cleanup();
    }

    public static void Application_processEvent(Applet a, Event e) {
        Log.log(TAG, "Application_processEvent");
        a.processEvent(e);
    }
}
