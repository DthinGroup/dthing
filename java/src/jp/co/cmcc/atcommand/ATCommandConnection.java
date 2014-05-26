/*
 * Copyright 2014 Yarlung Soft. All rights reserved.
 */

package jp.co.cmcc.atcommand;

import java.io.IOException;

import com.yarlungsoft.util.TextUtils;

public class ATCommandConnection extends Object {

    // Need to parse the response to make sure whether enter AT Data Mode
    private static final int AT_COMMAND_MODE = 0; // send command and get response
    private static final int AT_DATA_MODE = 1; // create channel to send/receive data
    private AsynSendResponseListener asynListener;
    private ATCommandURCListener urcListener;
    private static boolean isOpened = false;
    private static ATCommandDataChannel myChannel = null;

    private ATCommandConnection() {
    }

    private static native int open0();

    public static ATCommandConnection getInstance() {
        ATCommandConnection conn = new ATCommandConnection();
        // TODO: Do we need any initialization in native
        if (open0() < 0) {
            // throw new IOException("Failed to init AT Command");
            conn = null;
        } else {
            isOpened = true;
        }
        return conn;
    }

    /**
     * @param cmd AT command
     * @return responded string
     * @throws NullPointerException if parameter {@link #cmd} is {@code null} or empty string.
     * @throws IOException
     */
    public String send(String cmd) throws NullPointerException, IOException {
        String result = null;

        if (!isAvailable()) {
            throw new IOException("ATCommandConnection instance is closed");
        }

        if (TextUtils.isEmpty(cmd)) {
            throw new NullPointerException("null cmd");
        }

        while (AsyncIO.loop()) {
            result = send0(cmd);
        }

        return result;
    }

    private static native String send0(String atcmd);

    public void sendAsyn(String cmd) throws NullPointerException, IOException {
        int result = 0;
        String response = null;

        if (!isAvailable()) {
            throw new IOException("ATCommandConnection instance is closed");
        }

        if (TextUtils.isEmpty(cmd)) {
            throw new NullPointerException("Null command");
        }

        if (asynListener != null) {
            result = sendAsyn0(cmd);
            if (result < 0) {
                throw new IOException("Failed to send async AT command");
            }

            while (AsyncIO.loop()) {
                response = get0();
            }

            asynListener.onReceiveResponse(response);
/*
            new Thread() {

                public void run() {
                    String response = null;
                    while (true) {
                        response = get0();
                        if (response != null)
                            break;
                    }
                    asynListener.onReceiveResponse(response);
                }
            }.start();
*/
        } else {
            throw new NullPointerException("Null async listerner");
        }
    }

    private static native int sendAsyn0(String atcmd);

    private static native String get0();

    public void setResponseListener(AsynSendResponseListener listener) {
        asynListener = listener;
    }

    public void setURCListener(ATCommandURCListener listener) {
        urcListener = listener;
    }

    public void close() throws IOException {
        if (!this.isAvailable()) {
            throw new IOException("ATCommandConnection instance is closed");
        }

        if (myChannel != null) {
            myChannel.close();
            myChannel = null;
        }

        close0();
        isOpened = false;
        asynListener = null;
        urcListener = null;
    }

    private static native int close0();

    public ATCommandDataChannel getDataChannel() throws IOException {
        if (!this.isAvailable()) {
            throw new IOException("ATCommandConnection instance is closed");
        }

        if (myChannel == null) {
            myChannel = new ATCommandDataChannel();
        }
        return myChannel;
    }

    private boolean isAvailable() {
        return isOpened;
    }
}
