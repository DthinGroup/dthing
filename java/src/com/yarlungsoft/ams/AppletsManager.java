
package com.yarlungsoft.ams;

import jp.co.cmcc.event.Applet;
import jp.co.cmcc.event.Event;

/**
 * Managing applets.
 */
public class AppletsManager extends Applet {

    public AppletsManager() {
        CommandProcessor state = Main.getCmdState();
        if (state.getCurCmd() == CommandProcessor.CMD_INSTALL) {
            installApplet();
        } else {
            /**
             * Normally launch Applets_Manager as default command.
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

    private void listApplets() {
    }

    private void installApplet() {
    }
}
