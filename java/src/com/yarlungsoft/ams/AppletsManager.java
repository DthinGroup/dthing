package com.yarlungsoft.ams;

import jp.co.aplix.event.Applet;
import jp.co.aplix.event.Event;

public class AppletsManager extends Applet {

	
	
	
	
	private void listApplets() {
		
	}
	
	private void installApplet() {
		
	}
	
	public AppletsManager() {
		CommandProcessor state = Main.cmdState;
		if (state.curCmd == CommandProcessor.CMD_INSTALL) {
			installApplet();
		} else {
			/**
			 * Normally launch Applets_Manager as defalut command. 
			 */
			listApplets();
		}
	}
	
	@Override
	public void cleanup() {
		
	}

	@Override
	public void processEvent(Event event) {
		
	}

	@Override
	public void startup() {
		
	}

}
