
package iot.oem.adc;

import java.io.IOException;

public class ADCManager {

    private static ADCManager me = null;
    private static boolean isEnabled = false;

    /**
     * Initiate ADC functional unit to get singleton instance
     *
     * @return ADC manager instance when success, or {@code null} when failed
     */
    public static ADCManager getInstance() {
        if (me == null) {
            if (init0() >= 0) {
                me = new ADCManager();
                isEnabled = true;
            }
        } else {
            isEnabled = true;
        }
        return me;
    }

    private static native int init0();

    /**
     * Read data
     *
     * @param channelId channel id
     * @exception IllegalArgumentException when parameter is illegal
     * @exception IOException when exception happens
     * @return length when success
     */
    public int read(int channelId) throws IllegalArgumentException, IOException {
        if (!isAvailable()) {
            throw new IOException("ADCManager instance is destroyed");
        }

        if (!isValidChannelID(channelId)) {
            throw new IllegalArgumentException("Illegal channel ID " + channelId);
        }

        int length = read0(channelId);

        /* Currently, allow to read negative value, not throw exception at present */
        /*
         * if (length < 0) {
         * throw new IOException("Failed to read");
         * }
         */
        return length;
    }

    private static native int read0(int cid);

    /**
     * Destroy ADC related source
     *
     * @exception IOException when exception happens
     */
    public void destroy() throws IOException {
        if (!isAvailable()) {
            throw new IOException("ADCManager instance is destroyed");
        }

        if (destroy0() < 0) {
            throw new IOException("Failed to destroy");
        }
        isEnabled = false;
    }

    private static native int destroy0();

    private boolean isValidChannelID(int channelId) {
        boolean isValid = true;

        if (channelId < 0) {
            isValid = false;
        }
        return isValid;
    }

    private boolean isAvailable() {
        return isEnabled;
    }
}
