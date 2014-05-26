
package jp.co.cmcc.message.sms;

import java.io.IOException;

/**
 * The interface of MessageReceiver
 */
public interface MessageReceiver {

    /**
     * Receive the message.
     */
    public Message receive() throws IOException;
}
