
package com.yarlungsoft.ams;

import com.yarlungsoft.util.Log;

/**
 * Store and fetch Applet information.
 */
public class AppletContent {

    private static final String TAG = "AppletContent";

    private String mAppName;
    private String mAppVersion;
    private String mAppVendor;
    private String mAppFileName;
    private String mAppMainClass;
    private String mAppFullPathName;

    public AppletContent() {
        mAppName = "";
        mAppVersion = "";
        mAppVendor = "";
        mAppFileName = "";
        mAppMainClass = "";
    }

    public AppletContent(String name, String clsName) {
        this(name, null, null, null, clsName);
    }

    public AppletContent(String name, String version, String vendor, String fname, String clsName) {
        mAppName = name;
        mAppVersion = version;
        mAppVendor = vendor;
        mAppFileName = fname;
        mAppMainClass = clsName;
    }

    public void setAppName(String name) {
        Log.amsLog(TAG, "setAppName---name=(" + name + ")");
        mAppName = name;
    }

    public String getAppName() {
        return mAppName;
    }

    public void setAppFullPathName(String name) {
        Log.amsLog(TAG, "setAppFullPathName---name=(" + name + ")");
        mAppFullPathName = name;
    }

    public String getAppFullPathName() {
        return mAppFullPathName;
    }

    public void setAppVersion(String version) {
        Log.amsLog(TAG, "setAppVersion---version=(" + version + ")");
        mAppVersion = version;
    }

    public String getAppVersion() {
        return mAppVersion;
    }

    public void setAppVendor(String vendor) {
        Log.amsLog(TAG, "setAppVendor---vendor=(" + vendor + ")");
        mAppVendor = vendor;
    }

    public String getAppVendor() {
        return mAppVendor;
    }

    public void setAppFileName(String fname) {
        Log.amsLog(TAG, "setAppFileName---filename=(" + fname + ")");
        mAppFileName = fname;
    }

    public String getAppFileName() {
        return mAppFileName;
    }

    public void setAppMainClass(String clsName) {
        Log.amsLog(TAG, "setAppMainClass---mainclass=(" + clsName + ")");
        mAppMainClass = clsName;
    }

    public String getAppMainClass() {
        return mAppMainClass;
    }
}
