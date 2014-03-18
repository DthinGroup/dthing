package jp.co.aplix.event;

public abstract class Applet {

	public Applet() {
		
	}

	public abstract void startup();

	public abstract void cleanup();

	public abstract void processEvent(Event event);
}
