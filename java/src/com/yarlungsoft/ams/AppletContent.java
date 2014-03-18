package com.yarlungsoft.ams;

public class AppletContent {

    private String appName;
    private String appVersion;
    private String appVendor;
    private String appFileName;
    private String appMainClass;
    private String appFullPathName;

    public AppletContent() {
        appName = "";
        appVersion = "";
        appVendor = "";
        appFileName = "";
        appMainClass = "";
    }

	private static void log(String msg) {
		Main.log("AppletContent", msg);
	}

    public AppletContent(String name, String mainClsName) {
        this(name, null, null, null, mainClsName);
    }

    public AppletContent(String name, String version, String vendor, String filename, String mainClsName) {
        appName = name;
        appVersion = version;
        appVendor = vendor;
        appFileName = filename;
        appMainClass = mainClsName;
    }
    
    public void setAppName(String name) {
        log("setAppName---name=(" + name + ")");
        appName = name;
    }

    public String getAppName() {
        return appName;
    }

    public void setAppFullPathName(String name) {
        log("setAppFullPathName---name=(" + name + ")");
        appFullPathName = name;
    }

    public String getAppFullPathName() {
        return appFullPathName;
    }

    public void setAppVersion(String version) {
        log("setAppVersion---version=(" + version + ")");
        appVersion = version;
    }

    public String getAppVersion() {
        return appVersion;
    }

    public void setAppVendor(String vendor) {
        log("setAppVendor---vendor=(" + vendor + ")");
        appVendor = vendor;
    }

    public String getAppVendor() {
        return appVendor;
    }

    public void setAppFileName(String filename) {
        log("setAppFileName---filename=(" + filename + ")");
        appFileName = filename;
    }

    public String getAppFileName() {
        return appFileName;
    }

    public void setAppMainClass(String mainclass) {
        log("setAppMainClass---mainclass=(" + mainclass + ")");
        appMainClass = mainclass;
    }

    public String getAppMainClass() {
        return appMainClass;
    }
	
	
}
