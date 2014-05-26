
package jp.co.cmcc.message.sms;

import java.util.Date;

/**
 * The interface of Message
 */
public interface Message {

    /**
     * Returns the address associated with this message.
     * <p>
     * If this is a message to be sent, this address is the address of the recipient.
     * <p>
     * If this is a message that has been received, this address is the sender's address.
     * <p>
     * Note: This design allows sending responses to a received message easily by reusing the same
     * {@link Message} object and just replacing the payload. The address field can normally be kept
     * untouched (unless the used messaging protocol requires some special handling of the address).
     *
     * @return the address of this message, or {@code null} if the address is not set
     * @see {@link #setMessageAddress(String)}
     */
    public String getMessageAddress();

    /**
     * Sets the address associated with this message, i.e. the address returned by the
     * {@link #getMessageAddress()} method. The address may be set to {@code null}.
     *
     * @param addr address for the message
     * @see {@link #getMessageAddress()}
     */
    public void setMessageAddress(String addr);

    /**
     * Returns the timestamp indicating when this message has been sent.
     *
     * @return {@link Date} indicating the timestamp in the message or {@code null} if the timestamp
     * is not set or if the time information is not available in the underlying protocol message.
     */
    public Date getTimeStamp();
}
