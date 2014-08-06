
package jp.co.cmcc.message.sms;


import java.io.IOException;
import java.lang.IllegalArgumentException;
import java.lang.NullPointerException;


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
