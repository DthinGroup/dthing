
package iot.oem.battery;

import java.io.IOException;

public class BatteryManager extends Object {

    public static final int BATTERY_STATUS_CHARGING = 0;
    public static final int BATTERY_STATUS_FULL = 1;
    public static final int BATTERY_STATUS_NORMAL = 2;
    public static final int BATTERY_STATUS_LOW = 3;
    public static final int BATTERY_STATUS_UNKNOW = 4;

    private static BatteryManager me = null;

    /**
     * Initiate Battery functional unit to get singleton instance
     *
     * @return battery manager instance when success, or {@code null} when failed
     */
    public static BatteryManager getInstance() {
        if (me == null) {
            me = new BatteryManager();
        }
        return me;
    }

    /**
     * Get battery status
     *
     * @exception IOException when failed to get battery status
     * @return battery status
     */
    public int getStatus() throws IOException {
        int result = getStatus0();

        if (result < 0) {
            throw new IOException("Failed to get battery status");
        }

        return (result > BATTERY_STATUS_UNKNOW) ? BATTERY_STATUS_UNKNOW : result;
    }

    private static native int getStatus0();

    /**
     * Get battery temperature
     *
     * @exception IOException when failed to get battery temperature
     * @return battery temperature
     */
    public int getTemperature() {
        int result = getTemperature0();
        // TODO: Check if valid temperature
        return result;
    }

    private static native int getTemperature0();

    private boolean isValidStatus(int status) {
        return status >= BATTERY_STATUS_CHARGING && status < BATTERY_STATUS_UNKNOW;
    }
}
