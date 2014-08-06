
package jp.co.cmcc.message.sms;


import java.io.*;
import java.util.Date;

import com.yarlungsoft.util.Log;

/**
 * Base class for messages.
 * Required implementation of the Message interface.
 */
public class SMSMessage implements Message {

    /**
     * Controls GSM 7bit encoding usage.
     * <p>
     * GSM 7bit encoding is usable on GSM/UMTS compliant devices. But unusable on CDMA/WCDMA
     * devices.
     */
    public static final boolean GSM7BIT = true;

    /**
     * The binary mode of message content.
     */
    public static final int BINARY_MODE = 0;

    /**
     * The text mode of message content.
     */
    public static final int TEXT_MODE = 1;

    /**
     * 7-bit GSM text encoding (as bytes with top bit zero)
     */
    static final int ENC_GSM_7BIT = 0;

    /**
     * GSM binary encoding
     */
    static final int ENC_8BIT_BIN = 1;

    /**
     * UCS2 text encoding (as bytes in byte order used in SMS packet)
     */
    static final int ENC_UCS_2 = 2;

    /**
     * ASCII text encoding (for CDMA network which doesn't support GSM 7 bit)
     */
    static final int ENC_ASCII = 3;

    /**
     * The encoding type of message.
     */
    private int type;

    /**
     * The mode of message content.
     */
    private int mode;

    /**
     * The payload data for this message.
     */
    private byte[] data;

    /**
     * The content of this message.
     */
    private String payloadText;

    /**
     * The phone number address of this message.
     */
    private String address;

    /**
     * The port-number part of the address. 0 means unspecified.
     */
    private int port = 0;

    /**
     * A table mapping from 7-bit GSM encoding to unicode for 0 .. 0x1f
     * <p>
     * Note that the entry for 0x1b is just a placeholder since that is an escape character
     */
    private static final char[] GSM7BitToUcs2 = {
            0x0040, 0x00A3, 0x0024, 0x00A5, 0x00E8, 0x00E9, 0x00F9, 0x00EC, 0x00F2, 0x00C7, 0x000A,
            0x00D8, 0x00F8, 0x000D, 0x00C5, 0x00E5, 0x0394, 0x005F, 0x03A6, 0x0393, 0x039B, 0x03A9,
            0x03A0, 0x03A8, 0x03A3, 0x0398, 0x039E, 0xFFFF, 0x00C6, 0x00E6, 0x00DF, 0x00C9
    };

    /**
     * Timestamp of this message.
     * For received messages, timestamp of message in format of Date.
     * For sent messages, timestamp when it was sent.
     */
    private Date dTime;

    /**
     * Construct a new instance from binary data.
     */
    public SMSMessage(byte[] data) {
        if (data == null) {
            throw new NullPointerException();
        }

        mode = BINARY_MODE;
        type = ENC_8BIT_BIN;
        this.data = data;
        log("SMSMessage mode = BINARY_MODE");
    }

    /**
     * Construct a new instance from text.
     */
    public SMSMessage(String text) {
        if (text == null) {
            throw new NullPointerException();
        }

        mode = TEXT_MODE;
        payloadText = text;
        log("SMSMessage mode = TEXT_MODE");

        int pll = payloadText.length();
        byte[] cvtd = new byte[2 * pll];
        if (GSM7BIT) {
            pll = stringToGSM7Bit(payloadText, cvtd);
            type = ENC_GSM_7BIT;
        } else {
            pll = stringToASCII(payloadText, cvtd);
            type = ENC_ASCII;
        }

        if (pll < 0) {
            stringToUCS2BE(payloadText, cvtd);
            pll = cvtd.length;
            type = ENC_UCS_2;
        }

        data = new byte[pll];
        System.arraycopy(cvtd, 0, data, 0, pll);
    }

    private static void log(String string) {
        // Log.netLog("SMSMessage", msg);
    }

    /**
     * Convert string to UCS2 in network byte order
     *
     * @param payload Java string to convert
     * @param cvtd converted byte array
     */
    private static void stringToUCS2BE(String payload, byte[] cvtd) {
        int bc = 0;
        int len = payload.length();
        for (int cc = 0; cc < len; ++cc) {
            char c = payload.charAt(cc);
            cvtd[bc++] = (byte) (c >> 8);
            cvtd[bc++] = (byte) c;
        }
    }

    /**
     * Attempt to convert string to GSM7Bit format.
     *
     * @param payload String to convert
     * @param cvtd byte array to store conversion result
     * @return byte length of resulting string or -1 if unconvertible characters exist
     */
    private static int stringToGSM7Bit(String payload, byte[] cvtd) {
        int bytelen = 0;
        int len = payload.length();
        for (int pos = 0; pos < len; pos++) {
            char c = payload.charAt(pos);
            byte b;

            NOESCAPE: if (c == 0x40) {
                b = (byte) 0;
            } else if ((c >= 0x0020 && c <= 0x005a) || (c >= 0x0061 && c <= 0x007a)) {
                b = (byte) c;
            } else {
                // To save some space, we use two labels, such that
                // break ESCAPE takes us to code to insert a leading escape
                // break NOESCAPE skips the insertion
                // This is free, since break is just another name for goto
                // Strictly, we don't need the ESCAPE label since that's
                // where an unlabeled break would go, but making it
                // explicit makes things slightly clearer.

                ESCAPE: switch (c) {
                default:
                    // Unconvertible character. This is
                    // early to save having to jump over it later.
                    return -1;

                case 0x00a3:
                    b = 0x01;
                    break NOESCAPE;
                case 0x0024:
                    b = 0x02;
                    break NOESCAPE;
                case 0x00a5:
                    b = 0x03;
                    break NOESCAPE;
                case 0x00e8:
                    b = 0x04;
                    break NOESCAPE;
                case 0x00e9:
                    b = 0x05;
                    break NOESCAPE;
                case 0x00f9:
                    b = 0x06;
                    break NOESCAPE;
                case 0x00ec:
                    b = 0x07;
                    break NOESCAPE;
                case 0x00f2:
                    b = 0x08;
                    break NOESCAPE;
                case 0x00c7:
                    b = 0x09;
                    break NOESCAPE;
                case 0x000a:
                    b = 0x0a;
                    break NOESCAPE;
                case 0x00d8:
                    b = 0x0b;
                    break NOESCAPE;
                case 0x00f8:
                    b = 0x0c;
                    break NOESCAPE;
                case 0x000d:
                    b = 0x0d;
                    break NOESCAPE;
                case 0x00c5:
                    b = 0x0e;
                    break NOESCAPE;
                case 0x00e5:
                    b = 0x0f;
                    break NOESCAPE;
                case 0x0394:
                    b = 0x10;
                    break NOESCAPE;
                case 0x005f:
                    b = 0x11;
                    break NOESCAPE;
                case 0x03a6:
                    b = 0x12;
                    break NOESCAPE;
                case 0x0393:
                    b = 0x13;
                    break NOESCAPE;
                case 0x039b:
                    b = 0x14;
                    break NOESCAPE;
                case 0x03a9:
                    b = 0x15;
                    break NOESCAPE;
                case 0x03a0:
                    b = 0x16;
                    break NOESCAPE;
                case 0x03a8:
                    b = 0x17;
                    break NOESCAPE;
                case 0x03a3:
                    b = 0x18;
                    break NOESCAPE;
                case 0x0398:
                    b = 0x19;
                    break NOESCAPE;
                case 0x039e:
                    b = 0x1a;
                    break NOESCAPE;
                case 0x00c6:
                    b = 0x1c;
                    break NOESCAPE;
                case 0x00e6:
                    b = 0x1d;
                    break NOESCAPE;
                case 0x00df:
                    b = 0x1e;
                    break NOESCAPE;
                case 0x00c9:
                    b = 0x1f;
                    break NOESCAPE;
                case 0x00c4:
                    b = 0x5b;
                    break NOESCAPE;
                case 0x00d6:
                    b = 0x5c;
                    break NOESCAPE;
                case 0x00d1:
                    b = 0x5d;
                    break NOESCAPE;
                case 0x00dc:
                    b = 0x5e;
                    break NOESCAPE;
                case 0x00a7:
                    b = 0x5f;
                    break NOESCAPE;
                case 0x00bf:
                    b = 0x60;
                    break NOESCAPE;
                case 0x00e4:
                    b = 0x7b;
                    break NOESCAPE;
                case 0x00f6:
                    b = 0x7c;
                    break NOESCAPE;
                case 0x00f1:
                    b = 0x7d;
                    break NOESCAPE;
                case 0x00fc:
                    b = 0x7e;
                    break NOESCAPE;
                case 0x00e0:
                    b = 0x7f;
                    break NOESCAPE;

                // Extended character table require an escape prefix

                case 0x005e:
                    b = 0x14;
                    break ESCAPE;
                case 0x007b:
                    b = 0x28;
                    break ESCAPE;
                case 0x007d:
                    b = 0x29;
                    break ESCAPE;
                case 0x005c:
                    b = 0x2f;
                    break ESCAPE;
                case 0x005b:
                    b = 0x3c;
                    break ESCAPE;
                case 0x007e:
                    b = 0x3d;
                    break ESCAPE;
                case 0x005d:
                    b = 0x3e;
                    break ESCAPE;
                case 0x007c:
                    b = 0x40;
                    break ESCAPE;
                case 0x20ac:
                    b = 0x65;
                    break ESCAPE;
                }

                // break ESCAPE takes us to here.

                cvtd[bytelen++] = (byte) 0x1b;
            }

            // break NOESCAPE takes us to here
            cvtd[bytelen++] = b;
        }

        return bytelen;
    }

    /**
     * Attempt to convert string to ASCII.
     *
     * @param payload String to convert
     * @param cvtd byte array to store conversion result
     * @return byte length of resulting string or -1 if unconvertible characters exist
     */
    private static int stringToASCII(String payload, byte[] cvtd) {
        int len = payload.length();
        for (int i = 0; i < len; i++) {
            char c = payload.charAt(i);

            if (c > 127) {
                return -1;
            }

            cvtd[i] = (byte) c;
        }
        return len;
    }

    /**
     * Convert UCS2 string in network byte order to string
     *
     * @param bs
     * @return
     */
    static String UCS2BEToString(byte[] data) {
        int len = data.length / 2;
        // Naive implementation, could do with less allocation and copying.
        char[] chars = new char[len];
        int bpos = 0;
        for (int i = 0; i < len; ++i) {
            chars[i] = (char) ((data[bpos] << 8) | (data[bpos + 1] & 0xff));
            bpos += 2;
        }
        return new String(chars);
    }

    /**
     * Convert GSM 7 bit string in network byte order to string
     *
     * @param data
     * @return
     */
    static String GSM7BitToString(byte[] data) {

        char[] chars = new char[data.length];
        int strlen = 0;
        boolean escaped = false;

        for (int i = 0; i < data.length; i++) {
            byte b = data[i];

            if (b == 0x1b) {
                escaped = true;
                continue;
            }

            char c;
            if (escaped) {
                switch (b) {
                case 0x14:
                    c = 0x005e;
                    break;
                case 0x28:
                    c = 0x007b;
                    break;
                case 0x29:
                    c = 0x007d;
                    break;
                case 0x2f:
                    c = 0x005c;
                    break;
                case 0x3c:
                    c = 0x005b;
                    break;
                case 0x3d:
                    c = 0x007e;
                    break;
                case 0x3e:
                    c = 0x005d;
                    break;
                case 0x40:
                    c = 0x007c;
                    break;
                case 0x65:
                    c = 0x20ac;
                    break;
                default:
                    c = '*';
                    break; // show unknown as star
                }
                escaped = false;
            } else if (b < 0x20) {
                // We use a separate lookup table for the contiguous section at the low-end of the
                // range
                c = GSM7BitToUcs2[b];
            } else {
                // unfortunately, the normal (ASCII) case has
                // to work its way through this (lookup) switch,
                // but this is not performance critical code.

                switch (b) {
                case 0x40:
                    c = 0x00a1;
                    break;
                case 0x7b:
                    c = 0x00e4;
                    break;
                case 0x7c:
                    c = 0x00f6;
                    break;
                case 0x7d:
                    c = 0x00f1;
                    break;
                case 0x7e:
                    c = 0x00fc;
                    break;
                case 0x7f:
                    c = 0x00e0;
                    break;
                default:
                    c = (char) b;
                    break;
                }
            }

            chars[strlen++] = c;
        }

        return new String(chars, 0, strlen);
    }

    /**
     * Convert ASCII chars array to string
     *
     * @param data byte array with input data
     * @return converted String
     */
    static String ASCIIToString(byte[] data) {
        return new String(data);
    }

    /**
     * Check if it is a address.
     * Return true if it is a address, otherwise, return false.
     */
    private boolean checkAddress(String address) {
        boolean res = true;
        int len = address.length();

        if (len == 0) {
            return false;
        }

        for (int i = 0; i < len; i++) {
            char c = address.charAt(i);
            if (i == 0 && c == '+') {
                if (len == 1) {
                    res = false;
                    break;
                } else {
                    continue;
                }
            } else if (c == '0' || c == '1' || c == '2' || c == '3' || c == '4' || c == '5'
                    || c == '6' || c == '7' || c == '8' || c == '9') {
                continue;
            } else {
                res = false;
                break;
            }
        }

        return res;
    }

    /** Get SMS content mode. */
    public int getMode() {
        return mode;
    }

    /** Get encoding type of the message. */
    protected int getType() {
        return type;
    }

    public void setMessageAddress(String address) {
        log("setMessageAddress = " + address);

        if (address == null) {
            throw new NullPointerException();
        }

        if (!checkAddress(address)) {
            throw new IllegalArgumentException();
        }

        this.address = address;
    }

    /**
     * Returns the phone number address associated with this message.
     * <p>
     * If this is a message to be sent, this address is the address of the recipient.
     * <p>
     * If this is a message that has been received, this address is the sender's address.
     * <p>
     * Note: This design allows sending responses to a received message easily by reusing the same
     * {@link Message} object and just replacing the payload. The address field can normally be
     * kept untouched (unless the used messaging protocol requires some special handling of the
     * address).
     *
     * @return the address of this message, or {@code null} if the address is not set
     * @see {@link #setMessageAddress(String)}
     */
    public String getMessageAddress() {
        log("getMessageAddress =" + address);
        return address;
    }

    protected void setTimeStamp(Date time) {
        if (time == null) {
            throw new NullPointerException();
        }

        this.dTime = time;
    }

    /**
     * Returns the dTime indicating when this message has been sent.
     *
     * @return {@link Date} indicating the dTime in the message or {@code null} if the dTime is not
     * set or if the time information is not available in the underlying protocol message.
     */
    public Date getTimeStamp() {
        return dTime;
    }

    /**
     * Set the value of port.
     *
     * @param port Value to assign to port.
     */
    public void setRecipientPort(int port) {
        if (port < 0 || port > 0xFFFF) {
            throw new IllegalArgumentException();
        }

        this.port = port;
    }

    /**
     * Get the value of port.
     *
     * @return value of port.
     */
    public int getRecipientPort() {
        return port;
    }

    /**
     * Get content of this message.
     */
    public byte[] getData() {
        return data;
    }

}
