
package jp.co.cmcc.message.sms;


import java.io.IOException;

import com.yarlungsoft.util.Log;


/**
 * The class which implements the interface of MessageReceiver.
 */
public class MyMessageReceiver implements MessageReceiver {

    private static final String TAG = "MyMessageReceiver";

    private SMSConnection smsConnection;

    MyMessageReceiver(SMSConnection s) {
        Log.netLog(TAG, "MyMessageSender() created.");
        this.smsConnection = s;
    }

    /**
     * Receive the message.
     */
    public Message receive() throws IOException {
        Log.netLog(TAG, "MyMessageSender.receive enter.");

        if (smsConnection == null) {
            throw new IOException("Cannot receive message");
        }
        return smsConnection.receive();
    }
}
