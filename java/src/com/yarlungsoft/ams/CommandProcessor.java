package com.yarlungsoft.ams;

public class CommandProcessor {

	static final int CMD_NONE = -1,
					 CMD_RUN = 0,
					 CMD_LIST = 1,
					 CMD_INSTALL = 2,
					 CMD_EXIT = 3;
	
	int curCmd;
	String appName;
	String installURL;
	
	protected CommandProcessor() {
		curCmd  = CMD_NONE;
		appName = null;
	}
	
    void parseCommandArgs(String args[]) {
        for (int i = 0; i < args.length; i++) {

            String arg = args[i];
            log("parseCommandArgs arg = " + arg);

            try {
            	if (arg.equals("-run") || arg.equals("-runapk")) {
                	curCmd = CMD_RUN;
                    appName = args[++i];
                } else if (arg.equals("-list")) {
                	curCmd = CMD_LIST;
                } else if (arg.equals("-install")) {
                	curCmd = CMD_INSTALL;
                	installURL = args[++i];
                } else {
                	log("argument:args[" + i + "]=" + args[i] + "is not exists!");
                	curCmd = CMD_EXIT;
                }
            } catch (IndexOutOfBoundsException ioobe) {
                log("Error: Index out of bounds exception!");
                curCmd = CMD_EXIT;
            }
        }
    }

	private static void log(String msg) {
		Main.log("CommandProcess", msg);
	}
}
