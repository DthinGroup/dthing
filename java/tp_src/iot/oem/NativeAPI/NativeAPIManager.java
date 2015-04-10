
package iot.oem.NativeAPI;

import java.io.IOException;

public class NativeAPIManager {

    public static final int API_ID_MIN=0;
    public static final int API_ID_1=1;
    public static final int API_ID_2=2;
    public static final int API_ID_3=3;
    public static final int API_ID_4=4;
    public static final int API_ID_5=5;
    public static final int API_ID_6=6;
    public static final int API_ID_7=7;
    public static final int API_ID_8=8;
    public static final int API_ID_9=9;
    public static final int API_ID_MAX=10;
    private static NativeAPIManager me = null;
    private static boolean isEnabled = false;

    /**
     * initiate ADC functional unit to get singleton instance
     *
     * @return singleton instance of ADC manager when success, or null when failed
     */
    public static NativeAPIManager getInstance()
    {
        if (me == null)
        {
            if (init0() >= 0)
            {
                me = new NativeAPIManager();
                isEnabled = true;
            }
        }
        else
        {
            isEnabled = true;
        }
        return me;
    }

    private static native int init0();

    /**
     * read data from specified channel
     *
     * @param channelID int channel id
     * @return length when success
     * @throws IllegalArgumentException when illgegal param
     * @throws IOException when exception happens
     */
    public int call(int channelID, boolean swt ) throws IllegalArgumentException, IOException
    {
        int ret = -1;

        if (!isAvailable())
        {
            throw new IOException("NativeAPIManager instance is destroyed");
        }

        if (!isValidChannelID(channelID))
        {
            throw new IllegalArgumentException("Illegal channel ID " + channelID);
        }

        ret = call0(channelID,swt);

        return ret;
    }

    private static native int call0(int cid,boolean swt);


    /**
     * destroy ADC related source
     *
     * @throws IOException when failed to destroy
     */
    public void destroy(int channelID) throws IOException
    {
        if (!isAvailable())
        {
            throw new IOException("NativeAPIManager instance is destroyed");
        }

        if (destroy0(channelID) < 0)
        {
            throw new IOException("Failed to destroy");
        }
        isEnabled = false;
    }

    private static native int destroy0(int channelID);

    private boolean isValidChannelID(int channelID)
    {
        boolean isValid = false;

        if ((channelID > API_ID_MIN) &&  (channelID < API_ID_MAX))
        {
            isValid = true;
        }
        return isValid;
    }

    private boolean isAvailable()
    {
        return isEnabled;
    }
}