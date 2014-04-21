package jp.co.cmcc.event;

import com.yarlungsoft.ams.AppletState;

public abstract class Applet {
	private AppletState appState;

	public Applet() {
		appState = new AppletState(this);
	}

	public abstract void startup();

	public abstract void cleanup();

	public abstract void processEvent(Event event);
	
	public void notifyDestroyed() {
		appState.notifyDestroyed();
	}

}
