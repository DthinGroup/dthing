
package jp.co.cmcc.message.sms;

import java.io.IOException;
import java.io.InterruptedIOException;
import java.util.Date;

import com.yarlungsoft.util.Log;

public class SMSConnection {

    private int port;

    private static SMSConnection mInstance = null;

    private SMSListener listener;

    /**
     * Next in linked list of registered server connections.
     */
    protected SMSConnection next;

    /**
     * Is this connection instance open ? Cleared on close()
     */
    protected boolean opened;

    /**
     * Is this connection instance a server rather than a client ?
     */
    protected boolean server;

    /**
     * For a server, the port we are listening on.
     * For a client, -1
     */
    protected int localPort = -1;

    protected static final int BIND_ANYPORT = 0;

    protected static final int MAX_IN_MSGS = 8; // must be power of 2
    private static final long CHECK_INTERVAL = 60000;

    /**
     * For a server, the queue of pending (complete) messages.
     * <p>
     * Operations on this table should synchronize on inMsgs.
     * <p>
     * A ringbuffer is implemented via curReadPos and curWritePos.
     * <p>
     * curReadPos points at the next slot to read (modulo {@link #MAX_IN_MSGS}) curWritePos points
     * at the next slot to write (modulo {@link #MAX_IN_MSGS})
     * <p>
     * queue is empty if curWritePos == curReadPos queue is full if curWritePos == curReadPos +
     * {@link #MAX_IN_MSGS}
     * <p>
     * In effect, curReadPos and curWritePos traverse a space 2**32 bits, but we map them to a
     * sliding window of size {@link #MAX_IN_MSGS}.
     */
    private SMSMessage[] inMsgs;

    /**
     * Current index for adding to inMsgs.
     * When we add a message, we use it modulo {@link #MAX_IN_MSGS}, then increase it.
     *
     * @see {@link #inMsgs}
     */
    private int curWritePos;

    /**
     * Current index for removing from inMsgs.
     * When we remove a message, we use it modulo {@link #MAX_IN_MSGS}, then increase it.
     *
     * @see {@link #inMsgs}
     */
    private int curReadPos;

    private MyMessageSender sender;

    private MyMessageReceiver receiver;

    public SMSConnection() {
        log("SMSConnection() created.");
        opened = true;
    }

    public SMSConnection(int port) throws IOException {
        log("SMSConnection(" + port + ") created.");

        if (port < 0 || port > 0xFFFF) {
            throw new IOException();
        }

        this.port = port;
        if (!MySmsConnectionThread.bind(this, port)) {
            throw new IOException();
        }
        opened = true;
        inMsgs = new SMSMessage[MAX_IN_MSGS];
    }

    private static void log(String msg) {
        Log.netLog("SMSConnection", msg);
    }

    /**
     * According to specification, when application uses {@link SMSConnection#getInstance()} to
     * create the instance of {@link SMSConnection}, it will receive all message including with port
     * or not. In this case, we specify its bind port as 0;
     */
    public static SMSConnection getInstance() {
        try {
            if (mInstance == null) {
                mInstance = new SMSConnection(BIND_ANYPORT);
            }
        } catch (IOException ex) {
            log("SMSConnection.getInstance failed, return null directly.");
            return null;
        }

        return mInstance;
    }

    public MessageReceiver getReceiver() throws IOException {
        log("SMSConnection.getReceiver() enter");

        if (!opened) {
            throw new IOException("SMSConnection is closed.");
        }

        if (receiver == null) {
            receiver = new MyMessageReceiver(this);
        }
        return (MessageReceiver) receiver;
    }

    public MessageSender getSender() throws IOException {
        log("SMSConnection.getSender() enter");

        if (!opened) {
            throw new IOException("SMSConnection is closed.");
        }

        if (sender == null) {
            sender = new MyMessageSender(this);
        }
        return (MessageSender) sender;
    }

    public void setListener(SMSListener listener) throws IOException {
        log("SMSConnection.setListener() enter");

        if (!opened) {
            throw new IOException("SMSConnection has closed.");
        }

        this.listener = listener;
    }

    public void close() throws IOException {
        log("SMSConnection.close()");

        if (opened) {
            opened = false;
            MySmsConnectionThread.unbind(this);

            // unblock any readers
            synchronized (inMsgs) {
                inMsgs.notifyAll();
            }
        }

        if (port == BIND_ANYPORT) {
            mInstance = null;
        }

        receiver = null;
        sender = null;
    }

    /**
     * Adds a new incoming message (fragments) to the receiver queue.
     *
     * @param msgfrags received message fragment
     */
    void putMessage(SMSMessage msg) {
        synchronized (this) {
            if (!opened) {
                log("Not an opened server connection");
                return;
            }
        }

        synchronized (inMsgs) {
            if (curWritePos == (curReadPos + MAX_IN_MSGS)) {
                log("queue full");
                return;
            }

            inMsgs[curWritePos & (MAX_IN_MSGS - 1)] = msg;
            ++curWritePos;
            if (this.listener != null) {
                try {
                    listener.messageArrived(this);
                } catch (Throwable ex) {
                    log("SMSConnection.putMessage() catch Throwable exception.");
                }
            }
            inMsgs.notifyAll();
        }
    }

    /**
     * Receives a message.
     * <p>
     * If there are no {@link Message}s for this {@link SMSConnection} waiting, this method will
     * block until a message for this {@link SMSConnection} is received, or the
     * {@link SMSConnection} is closed.
     *
     * @return a {@link Message} object representing the information in the received message
     * @throws IOException if an error occurs while receiving a message
     * @throws InterruptedIOException if this {@link SMSConnection} object is closed during this
     * receive method call
     * @throws SecurityException if the application does not have permission to receive messages
     * using the given port number
     * @see {@link #putMessage(SMSMessage)}
     */
    Message receive() throws IOException {
        log("SMSConnection.receive()");
        if (!opened) {
            throw new IOException("CLOSED");
        }

        synchronized (inMsgs) {
            while (curReadPos == curWritePos) {
                if (!opened) {
                    throw new IOException("CLOSED");
                }
                try {
                    inMsgs.wait(CHECK_INTERVAL);
                } catch (InterruptedException ie) {
                }
            }
            int i = curReadPos & (MAX_IN_MSGS - 1);
            ++curReadPos;
            SMSMessage m = inMsgs[i];
            inMsgs[i] = null;

            return (Message) m;
        }
    }

    protected int getLocalPort() {
        return localPort;
    }
}
