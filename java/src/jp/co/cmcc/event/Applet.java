package jp.co.cmcc.event;

import com.yarlungsoft.ams.AppletState;

/*
 * lifecycle class of dthing
 */
public abstract class Applet {
	private AppletState appState;

	/*
	 * Applet constructor
	 */
	public Applet() {
		appState = new AppletState(this);
	}

	/*
	 * startup method Of dthing applet
	 * @ be called when applet starts
	 */
	public abstract void startup();

	/*
	 * clean method Of dthing applet
	 * @ be called when applet ends
	 */	
	public abstract void cleanup();

	/*
	 * process event method Of dthing applet
	 * @ be called when deliver event
	 */
	public abstract void processEvent(Event event);

	/*
	 * to destroy the dthing applet
	 * @ be called by the user
	 */
	public void notifyDestroyed() {
		appState.notifyDestroyed();
	}

}
