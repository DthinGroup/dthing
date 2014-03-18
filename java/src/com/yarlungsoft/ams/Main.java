package com.yarlungsoft.ams;

public class Main {

	private static final boolean DEBUG = AmsConfig.debug();
	
	private static final String AMS_NAME = "AppletsManager";
	
	private static final String INSTALLED_PATH = "appdb/";
	
	static CommandProcessor cmdState;
	
	public static void main(String args[]) {
		if (DEBUG) {
			for (int i = 0; i < args.length; i++) {
				log("args[" + i +  "] = " + args[i]);
			}
		}
		
        cmdState = new CommandProcessor();
        cmdState.curCmd = CommandProcessor.CMD_EXIT;

        cmdState.parseCommandArgs(args);

        switch (cmdState.curCmd)
        {
        case CommandProcessor.CMD_RUN:
            launchApp(cmdState.appName);
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
	
	static AppletListNode listApplets() {
		return null;		
	}
	
	private static void launchAms() {
        try {
    		AppletContent ac = new AppletContent(AMS_NAME, "com.yarlungsoft.ams.AppletsManager");
            Scheduler.schedule(ac);
            System.out.println(">>>>>>>>>>>>>>launchAMS end");
        } catch (Throwable t) {
            log("launch AMS failure!");
        }
	}
	
    private static void launchApp(String appName) {
    	
    	AppletContent ac = null;
    	String appPath = AmsConfig.getRoot() + INSTALLED_PATH;
    	
    	
    	
    	
//        AppletContent ac = null;
//        String appfolder = AmsConfig.getRoot() + "/AppList/";
//
//        appInfoManage appInfoNode = fb.List("file:///"+appfolder);
//
//        //search the corresponding appInfo
//        while(appInfoNode != null) {
//            ai = appInfoNode.getCurAppInfo();
//            if (ai.getAppName().equals(appName)) {
//                break;
//            }
//            appInfoNode = appInfoNode.getNextInfoNode();
//        }
//        if (appInfoNode == null) {
//            //should notify user no this application!
//            if (DEBUG)
//                log("Do not find request app!");
//        }
//
//        try {
//            Scheduler.schedule(ai);
//        } catch (Throwable t) {
//            log("launch APP failure!");
//        }
    }

	private static void log(String msg) {
		log("Main", msg);
	}

	static void log(String module, String msg) {
		if (msg == null) {
			return; //nothing to do!
		}
		
		System.out.println("ams." + module + " - " + msg);
	}
}
