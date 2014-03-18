package jp.co.aplix.event;

public class PackageAccess {

	PackageAccess() {
	    //nothing to do
	}
	
	public static void Application_startup(Applet a) {
	    System.out.println("Application_startup");
	    a.startup();
	}
	
	public static void Application_cleanup(Applet a) {
	    System.out.println("Application_cleanup");
	    a.cleanup();
	}
	
	public static void Application_processEvent(Applet a, Event e) {
	    System.out.println("Application_processEvent");
	    a.processEvent(e);
	}
}
