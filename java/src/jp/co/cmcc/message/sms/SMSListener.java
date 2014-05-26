
package jp.co.cmcc.message.sms;

/**
 * The {@link SMSListener} interface provides a mechanism for the application to be notified of
 * incoming messages.
 * <p>
 * When an incoming message arrives, the {@link #messageArrived} method is called. The application
 * MUST retrieve the message using the {@link SMSConnection#receive} method of the
 * {@link SMSConnection}.
 * </p>
 * <p>
 * This listener mechanism allows applications to receive incoming messages without needing to have
 * a thread blocked in the {@link SMSConnection#receive} method call.
 * </p>
 * <p>
 * If multiple messages arrive very closely together in time, the implementation MAY call this
 * listener from multiple threads in parallel. Applications MUST be prepared to handle this and
 * implement any necessary synchronization as part of the application code, while obeying the
 * requirements set for the listener method.
 * </p>
 */
public interface SMSListener {

    /**
     * This method will be called by the platform when an incoming message arrives to a
     * {@link SMSConnection} where the application has registered this listener object.
     * <p>
     * This method is called once per each incoming message to that {@link SMSConnection}.
     * </p>
     * <p>
     * NOTE: The implementation of this method MUST return quickly and MUST NOT perform any
     * extensive operations. The application SHOULD NOT receive and handle the message during this
     * method call but this should only act as a trigger to start that activity in the application's
     * own thread.
     * </p>
     *
     * @param conn the {@link SMSConnection} where the incoming message has arrived
     */
    public void messageArrived(SMSConnection conn);

}
