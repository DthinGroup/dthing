
package jp.co.cmcc.message.sms;

import java.io.IOException;
import java.lang.NullPointerException;
import java.lang.IllegalArgumentException;

/**
 * The interface of MessageSender
 */
public interface MessageSender {

    /**
     * Send the message.
     */
    public void send(Message msg) throws IOException, IllegalArgumentException,
            NullPointerException;
}
