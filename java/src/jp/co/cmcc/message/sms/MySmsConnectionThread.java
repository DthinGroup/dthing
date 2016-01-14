
package jp.co.cmcc.message.sms;


import java.io.IOException;
import java.util.Date;

import com.yarlungsoft.util.Log;


/**
 * Singleton class implementing the code for a reader thread.
 * An instance is created when the first SMS connection is created,
 * and spends it's time blocked in native code waiting for
 * incoming messages.
 * The native code writes the details of the incoming message directly
 * into the various fields of {@code this}.
 */
class MySmsConnectionThread extends Thread {

    /** Head of linked list of registered server connections. */
    private static SMSConnection connections;

    /** Singleton instance of reader thread. */
    private static MySmsConnectionThread scThread;

    /** The single instance created by SMSConnection.getInstance(). */
    private static SMSConnection singleConnection;

    /** Is sms registered in native. */
    private static boolean isReg = false;

    private static int MSG_BUF_LEN = 1024;

    /**
     * Find a server connection by provided port number
     *
     * @internal
     * @param port port number to find.
     * @return {@link SMSConnection} instance which registered this port, or null.
     */
    private static synchronized SMSConnection findConnection(int port) {
        SMSConnection s;
        for (s = connections; s != null; s = s.next) {
            if (s.localPort == port) {
                log("findConnection matched " + port);
                return s;
            }
        }
        log("findConnection cannot match port " + port);
        return null;
    }

    private static void log(String msg) {
        Log.netLog("MySmsConnectionThread", msg);
    }

    /**
     * Register the port number in case it has not had connection yet.
     * It is static so we can use synchronization in an obvious way.
     * If this is the first server to bind, start the thread.
     *
     * @param instance SMSConnection instance handling this port
     * @param port port number to bind
     * @return true if the port was bound successfully
     * @throws IOException
     */
    static synchronized boolean bind(SMSConnection instance, int port) throws IOException {
        log("MySmsConnectionThread: bind " + port);

        if (singleConnection != null || findConnection(port) != null) {
            log("the connection already in use or exist single connection.");
            return false; // already in use
        }

        if (!isReg) {
            if (!nRegister()) {
                throw new IOException("sms register failed.");
            }
            isReg = true;
        }

        // Start the thread if it is not already running
        if (scThread == null) {
            (scThread = new MySmsConnectionThread()).start();
        }

        instance.localPort = port;
        if (port == SMSConnection.BIND_ANYPORT) {
            singleConnection = instance;
        } else {
            // Add this instance to the head of the list
            // We'd better write the port field ourselves so that it is
            // valid when it is on the list
            instance.next = connections;
            connections = instance;
        }

        return true;
    }

    /**
     * Stop servicing a server connection.
     * Deregister the port from the native code.
     *
     * @param connection SMSConnection instance
     */
    static synchronized void unbind(SMSConnection connection) {
        int port = connection.localPort;
        log("unbind() " + port);

        if (port == SMSConnection.BIND_ANYPORT && singleConnection != null) {
            log("unbind() for singleConnection! ");
            singleConnection = null;
        } else {
            connection.localPort = -1;

            // Remove from linked list of connections.
            if (connections == connection) {
                // head of list
                connections = connection.next;
            } else {
                // search the list
                for (SMSConnection prev = connections; prev != null; prev = prev.next) {
                    if (prev.next == connection) {
                        prev.next = connection.next;
                        return;
                    }
                }

                log("unbind(): connection not found");
            }
        }

        if (singleConnection == null && connections == null) {
            nUnregister();
        }
    }

    /**
     * @name Fields to be filled out by the call to nReadMessage
     * The actual message header data and payload of received messages
     * should be filled in these fields as a
     */
    /* @{ */

    /**
     * Encoding of received message.
     * One of {@link SMSMessage#BINARY_MODE}, or {@link SMSMessage#TEXT_MODE}
     *
     * @see {@link #nReadMessage()}
     */
    private volatile int type;

    /**
     * Address of received message. in address international format, i.e. with leading +, with one
     * digit in ASCII per byte.
     *
     * @see {@link #nReadMessage()}
     */
    private volatile String address;

    /**
     * Payload of received message.
     *
     * @see {@link #nReadMessage()}
     */
    private volatile byte[] data;

    /**
     * Source port number of received message.
     * (-1 if not applicable)
     *
     * @see {@link #nReadMessage()}
     */
    private volatile int srcPort;

    /**
     * Destination port number of received message for SMS.
     * (-1 if not applicable)
     *
     * @see {@link #nReadMessage()}
     */
    private volatile int dstPort;

    /**
     * Timestamp of received message (ms since 1970).
     *
     * @see {@link #nReadMessage()}
     */
    private volatile long timestamp;

    /* @} */

    /**
     * Translate a buffer bytes with start pos into integer.
     */
    private int tranBytesToInt(byte[] buf, int pos) {
        return
            (((int)buf[pos]) << 24) |
            (((int)buf[pos+1]) << 16) |
            (((int)buf[pos+2]) <<  8) |
            (((int)buf[pos+3]));    
    }

    /**
     * Main routine of the reader thread.
     * Maintains a collection of registered SmsConnection instances, and delivers messages as
     * required.
     */
    public void run() {
        log("SMS Java thread: Running");

        // We keep the msisdn byte array permanently allocated - since
        // toString() always copies the content, we may as well reuse
        // the byte array.

        for (;;) {
            try {
                // Wait for a message. nReadMessage() fills
                // the details into instance fields.

                boolean result = false;
                byte[] msgBuf = new byte[MSG_BUF_LEN];
                int timeHigh = 0, timeLow = 0;
                int addlen = 0, txtlen = 0;
                SMSMessage msg;

                while (AsyncIO.loop()) {
                    result = nReadMessage(msgBuf, MSG_BUF_LEN);
                }

                if (!result) {
                    log("nReadMessage failed");

                    sleep(10000);
                    continue;
                } else {
                    log("nReadMessage receive msg.");
                }

                /* Parse data from msg buffer as below format:
                 * srcPort(4bytes) + dstPort(4bytes) + type(4bytes) + time_high(4bytes) + time_high(4bytes) +
                 * addlen(4bytes) + address(addlen bytes) + txtlen(4bytes) + text(txtlen bytes).
                 */
                srcPort  = tranBytesToInt(msgBuf, 0);
                dstPort  = tranBytesToInt(msgBuf, 4);
                type     = tranBytesToInt(msgBuf, 8);
                timeHigh = tranBytesToInt(msgBuf, 12);
                timeLow  = tranBytesToInt(msgBuf, 16);
                addlen   = tranBytesToInt(msgBuf, 20);
                address  = new String(msgBuf, 24, addlen);
                txtlen   = tranBytesToInt(msgBuf, 24+addlen);
                data     = new byte[txtlen];
                System.arraycopy(msgBuf, 28+addlen, data, 0, txtlen);
                 
                // Print receive fields info.
                log("nReadMessage succeeded. srcPort = " + srcPort + " dstport =" + dstPort + " tpye = " + type);
                log("nReadMessage timeHigh = " + timeHigh + " timeLow =" + timeLow + " address = " + address + " txtlen = " + txtlen);
 
                // Translate the time stamp
                timestamp = (long)(((long)timeHigh << 32) | (long)timeLow);              

                // deliver the message to the sms connection
                if (type == SMSMessage.ENC_8BIT_BIN) { // BINARY_MODE
                    msg = new SMSMessage(data);
                } else { // TEXT_MODE
                    String plText = null;

                    if (type == SMSMessage.ENC_UCS_2) {
                        plText = SMSMessage.UCS2BEToString(data);
                    } else if (type == SMSMessage.ENC_GSM_7BIT) {
                        plText = SMSMessage.GSM7BitToString(data);
                    } else {
                        plText = SMSMessage.ASCIIToString(data);
                    }
                    msg = new SMSMessage(plText);
                }

                msg.setMessageAddress(address);
                msg.setRecipientPort(srcPort);
                msg.setTimeStamp(new Date(timestamp));

                deliverMessage(msg, dstPort);
                while (AsyncIO.loop()) {
                    nDeleteMessage();
                }

                // clear fields to allow GC to do its stuff
                data = null;
            } catch (Exception exc) {
                // exc.printStackTrace();
            }
        }
    }

    /**
     * Put the message in a connections inbound queue
     *
     * @param fragment (partial) inbound message
     * @param dstPort destination
     */
    static void deliverMessage(SMSMessage message, int dstPort) {
        SMSConnection connection = null;

        // singleConnection have the priority of receiving the message.
        if (singleConnection != null) {
            connection = singleConnection;
        } else {
            connection = findConnection(dstPort);
        }

        if (connection != null) {
            log("found a sms connection");
            connection.putMessage(message);
        } else {
            log("no listening connection found? port=" + dstPort);
        }
    }

    /**
     * Native method to read one message (or message fragment).
     * <p>
     * A thread that repeatedly calls this method is launched just before the first call to
     * nRegisterPort in this VM session.
     * <p>
     * After this method completes successfully the fields of the object instance passed (this) must
     * be filled out with the relevant data as a side effect. After the message (fragment) has been
     * processed on the Java side nDeleteMessage will be called to discarded the message.
     * <p>
     * This method should scan through all ports registered via this interface and for each such
     * port should do (in this order):
     * <ul>
     * <li>Check the push registry for queued messages on checked out ports</li>
     * <li>Check for pending messages for normally opened ports</li>
     * <li>If no messages are available it should use ASYNC operations to "block" and notify when
     * there are (and the check will restart from the top).</li>
     * </ul>
     * <p>
     * It is safe to call this method for ports that have not been previously registered (unless
     * they are in use by the push registry) but the message (fragment) will be silently ignored.
     *
     * @param buffer The buffer used to bring back message data.
     * @param bufLen The buffer length.
     * @return true if the fields contain a new message, false otherwise.
     */
    private native boolean nReadMessage(byte[] buffer, int bufLen);

    /**
     * Delete a message.
     * Called after the results from nReadMessage() have been processed.
     * The fields are unchanged from the call to nReadMessage.
     * The native layer may not have anything to do at this point.
     */
    private native void nDeleteMessage();

    /**
     * Register sms message in native.
     */
    private static native boolean nRegister();

    /**
     * Unregister.
     */
    private static native void nUnregister();
}
