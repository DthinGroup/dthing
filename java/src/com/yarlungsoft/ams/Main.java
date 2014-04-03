
package com.yarlungsoft.ams;

import com.yarlungsoft.util.Log;

/**
 * The first class loaded in VM by main.c to perform a MIDP command.
 * <p>
 * This class performs 6 basic commands:
 * <ul>
 * <li>Application installation
 * <li>Application removal
 * <li>Application listing
 * <li>Application execution
 * <li>Graphical Application Management
 * <li>Execute a single Applet from the classpath
 * </ul>
 * For the case of executing an installed applet, the wrapper c code in main.c takes care of adding
 * the appropriate .jar file to the class path.
 * <p>
 * One extra duty for this class is to reset the VM in a callback.
 */
public class Main {

    private static final String AMS_NAME = "AppletsManager";
    private static final String INSTALLED_PATH = "appdb/";
    private static final String TAG = "Main";
    private static CommandProcessor sCmdState;

    public static void main(String[] args) {
        for (int i = 0; i < args.length; i++) {
            Log.amsLog(TAG, "args[" + i + "] = " + args[i]);
        }

        sCmdState = new CommandProcessor();
        sCmdState.setCurCmd(CommandProcessor.CMD_EXIT);
        sCmdState.parseCommandArgs(args);

        switch (sCmdState.getCurCmd()) {
        case CommandProcessor.CMD_RUN:
            launchApp(sCmdState.getAppName());
            break;

        case CommandProcessor.CMD_LIST:
        case CommandProcessor.CMD_INSTALL:
            launchAms();
            break;

        default:
        case CommandProcessor.CMD_EXIT:
            break;
        }
    }

    protected static CommandProcessor getCmdState() {
        return sCmdState;
    }

    private static AppletListNode listApplets() {
        return null;
    }

    private static void launchAms() {
        try {
            AppletContent ac = new AppletContent(AMS_NAME, "com.yarlungsoft.ams.AppletsManager");
            Scheduler.schedule(ac);
            Log.amsLog(TAG, ">>>>>>>>>>>>>>launchAMS end");
        } catch (Throwable t) {
            Log.amsLog(TAG, "launch AMS failure!");
        }
    }

    private static void launchApp(String appName) {
        AppletContent ac = null;
        String appPath = AmsConfig.getRoot() + INSTALLED_PATH;

//        AppletContent ac = null;
//        String appfolder = AmsConfig.getRoot() + "/AppList/";
//
//        appInfoManage appInfoNode = fb.List("file:///" + appfolder);
//
//        // search the corresponding appInfo
//        while (appInfoNode != null) {
//            ai = appInfoNode.getCurAppInfo();
//            if (ai.getAppName().equals(appName)) {
//                break;
//            }
//            appInfoNode = appInfoNode.getNextInfoNode();
//        }
//        if (appInfoNode == null) {
//            // should notify user no this application!
//            if (DEBUG)
//                Log.amsLog(TAG, "Do not find request app!");
//        }
//
//        try {
//            Scheduler.schedule(ai);
//        } catch (Throwable t) {
//            Log.amsLog(TAG, "launch APP failure!");
//        }
    }
}
