
package jp.co.cmcc.wmmp;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;

public class WMMPDataItem {

    private final static boolean DEBUG =false;

    private final static int INVALID_INT_VALUE = -1;
    private final static String INVALID_STRING_VALUE = null;
    private final static byte[] INVALID_BYTES_VALUE = null;

    public final static int TLV_INVALID = -1;
    public final static int TLV_INT = 0;
    public final static int TLV_STR = 1;
    public final static int TLV_BIN = 2;

    private int valueType = TLV_INVALID;
    private int tag;

    private int intValue;
    private byte[] byteValue;
    private String strValue;

    private int length = 0;

    public WMMPDataItem(int tag, byte[] value) {
        log("WMMPDataItem(int,byte[])");

        intValue = INVALID_INT_VALUE;
        strValue = INVALID_STRING_VALUE;

        this.tag = tag;
        if (value == null) {
            valueType = TLV_INVALID;
            byteValue = INVALID_BYTES_VALUE;
            length = 0;
        } else {
            valueType = TLV_BIN;
            byteValue = new byte[value.length];
            System.arraycopy(value, 0, byteValue, 0, value.length);
            length = value.length;
        }
    }

    public WMMPDataItem(int tag, int value) {
        log("WMMPDataItem(int,int)");

        byteValue = INVALID_BYTES_VALUE;
        strValue = INVALID_STRING_VALUE;

        this.tag = tag;
        if (value < 0) {
            valueType = TLV_INVALID;
            intValue = INVALID_INT_VALUE;
            length = 0;
        } else {
            valueType = TLV_INT;
            intValue = value;
            length = 4;
        }
    }

    public WMMPDataItem(int tag, String value) {
        log("WMMPDataItem(int,String)");

        byteValue = INVALID_BYTES_VALUE;
        intValue = INVALID_INT_VALUE;

        this.tag = tag;
        if (value == null) {
            valueType = TLV_INVALID;
            strValue = INVALID_STRING_VALUE;
            length = 0;
        } else {
            valueType = TLV_STR;
            strValue = new String(value);
            length = value.length();
        }
    }

    protected static WMMPDataItem createTlv(int tag, byte[] value, int type) {
        WMMPDataItem item = null;

        switch (type) {
        case TLV_BIN:
            // cast parameter value in case it is null
            item = new WMMPDataItem(tag, (byte[]) value);
            break;

        case TLV_INT:
            if (value == null) {
                item = new WMMPDataItem(tag, INVALID_INT_VALUE);
            } else {
                int val = 0;
                int nBytes = Math.min(value.length, 4);
                for (int i = 0; i < nBytes; i++) {
                    val = val | ((value[i] & 0x0FF) << (i * 8));
                }
                item = new WMMPDataItem(tag, val);
            }
            break;

        case TLV_STR:
            String val = INVALID_STRING_VALUE;
            if (value != null) {
                val = new String(value);
            }
            item = new WMMPDataItem(tag, val);
            break;

        case TLV_INVALID:
            item = new WMMPDataItem(tag, INVALID_BYTES_VALUE);
            break;
        }

        return item;
    }

    public short getTag() {
        log("getTag() tag=" + tag);
        return (short) tag;
    }

    public int getType() {
        log("getType() valueType=" + valueType);
        return valueType;
    }

    public int getLen() {
        log("getLen() length=" + length);
        return length;
    }

    public int getInt() {
        log("getInt() intValue=" + intValue);
        return intValue;
    }

    public String getString() {
        log("getString() strValue=" + strValue);
        return strValue;
    }

    public byte[] getBytes() {
        return byteValue;
    }

    protected byte[] getRawData() {
        switch (valueType) {
        case TLV_BIN:
            return byteValue;

        case TLV_INT:
            if (intValue == INVALID_INT_VALUE) {
                return null;
            }
            byte val[] = new byte[4];
            val[0] = (byte) (intValue & 0xFF);
            val[1] = (byte) ((intValue >>> 8) & 0xFF);
            val[2] = (byte) ((intValue >>> 16) & 0xFF);
            val[3] = (byte) ((intValue >>> 24) & 0xFF);
            return val;

        case TLV_STR:
            if (strValue == INVALID_STRING_VALUE) {
                return null;
            }
            return strValue.getBytes();

        case TLV_INVALID:
        default:
            return null;
        }
    }

    public byte[] getTlvRawData() {
        ByteArrayOutputStream baos = new ByteArrayOutputStream();
        DataOutputStream dos = new DataOutputStream(baos);
        try {
            dos.writeChar(tag);
            dos.writeChar(length);

            byte[] rawData = getRawData();
            if (rawData != null) {
                dos.write(rawData);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }

        byte[] data = baos.toByteArray();
        try {
            dos.close();
            baos.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return data;
    }

    private void log(String msg) {
        if (DEBUG) {
            System.out.println("WMMPDataItem:" + msg);
        }
    }
}
