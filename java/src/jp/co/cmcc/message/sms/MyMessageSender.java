
package jp.co.cmcc.message.sms;


import java.io.IOException;
import java.lang.IllegalArgumentException;
import java.lang.NullPointerException;

import com.yarlungsoft.util.Log;


/**
 * The class which implements the interface of MessageSender.
 */
public class MyMessageSender implements MessageSender {

    private SMSConnection smsConnection;

    MyMessageSender(SMSConnection s) {
        log("MyMessageSender() created.");
        smsConnection = s;
    }

    private void log(String msg) {
        // Log.netLog("MyMessageSender", msg);
    }

    /** Send the message. */
    public void send(Message msg) throws IOException, IllegalArgumentException,
            NullPointerException {
        log("MyMessageSender.send()");

        if (msg == null) {
            throw new NullPointerException();
        }

        if (!smsConnection.opened) {
            throw new IOException("SMS connection has been closed!");
        }

        SMSMessage smsg = (SMSMessage) msg;
        String address = smsg.getMessageAddress();
        if (address == null) {
            throw new IllegalArgumentException("No address.");
        }
        int srcPort = ((smsConnection != null) ? smsConnection.getLocalPort() : -1);
        int dstPort = smsg.getRecipientPort();
        byte[] payload = smsg.getData();
        int payloadLen = smsg.getData().length;
        int type = smsg.getType();

        log("MyMessageSender.send() address = " + address + ",srcPort " + srcPort + ", port = "
                + dstPort + ",tpye = " + type + ",payload = " + payload + ", payloadLen = "
                + payloadLen);

        boolean result = false;
        /* We allow only one SMS send in progress globally */
        synchronized (this.getClass()) {
            while (AsyncIO.loop()) {
                result = nSend(address, srcPort, dstPort, type, payload, payloadLen);
            }
        }

        log("Java send msg result = " + result);

        if (!result) {
            throw new IOException("Cannot send message");
        }
    }

    /**
     * Sends a SMS to a given address
     * This function is allowed to use non-blocking operations.
     *
     * @param address string with the recipient's phone number.
     * @param srcPort The sender's port number, or -1 if no port
     * @param dstPort The recipient's port number, or 0 if no port
     * @param dataBA byte array containing the data to send
     * @param dataLen The length of the data to be sent
     * @return true if the message was sent correctly.
     **/
    private static native boolean nSend(String address, int srcPort, int dstPort, int type,
            byte[] dataBA, int dataLen);
}
