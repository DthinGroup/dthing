
package com.yarlungsoft.ams;

import com.yarlungsoft.util.Log;

/**
 * Performs basic Applet Suite management commands.
 * <p>
 * The <code>Main</code> class performs four basic operations:
 * <ul>
 * <li>Application installation
 * <li>Application removal
 * <li>Application listing
 * <li>Application execution
 * </ul>
 * <p>
 * Together, these operations are referred to as the Java Application Manager or <em>JAM</em>.
 */
public class CommandProcessor {

    public static final int CMD_NONE    = -1;
    public static final int CMD_RUN     = 0;
    public static final int CMD_LIST    = 1;
    public static final int CMD_INSTALL = 2;
    public static final int CMD_EXIT    = 3;
    public static final int CMD_OTA     = 4;

    private static final String TAG = "CommandProcessor";

    private int mCurCmd;
    private String mAppName;
    private String mClsName;
    private String mInstallURL;

    protected CommandProcessor() {
        mCurCmd = CMD_NONE;
        mAppName = null;
        mClsName = null;
    }

    protected void setCurCmd(int cmd) {
        mCurCmd = cmd;
    }

    protected int getCurCmd() {
        return mCurCmd;
    }

    protected String getAppName() {
        return mAppName;
    }
    
    protected String getMainClsName() {
    	return mClsName;
    }

    protected String getInstallURL() {
        return mInstallURL;
    }

    protected void parseCommandArgs(String[] args) {
        for (int i = 0; i < args.length; i++) {

            String arg = args[i];
            Log.amsLog(TAG, "parseCommandArgs arg = " + arg);

            try {
                if ("-run".equals(arg) || "-runapk".equals(arg)) {
                    mCurCmd = CMD_RUN;
                    mClsName = args[++i];
                } else if ("-list".equals(arg)) {
                    mCurCmd = CMD_LIST;
                } else if ("-install".equals(arg)) {
                    mCurCmd = CMD_INSTALL;
                    mInstallURL = args[++i];
                } else if ("-ota".equals(arg)) {
                	mCurCmd = CMD_OTA;
                	mInstallURL = args[++i];
                } else {
                    Log.amsLog(TAG, "argument:args[" + i + "]=" + args[i] + " is not exists!");
                    mCurCmd = CMD_EXIT;
                }
            } catch (IndexOutOfBoundsException ioobe) {
                Log.amsLog(TAG, "Error: Index out of bounds exception!");
                mCurCmd = CMD_EXIT;
            }
        }
    }
}
