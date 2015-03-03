
package iot.oem.pwm;

import java.io.IOException;

public class PWMManager extends Object {

    public static final int PWM_CMD_SET_ONOFF = 0x30;
    public static final int PWM_CMD_SET_CLKSRC = 0x31;
    public static final int PWM_CMD_GET_CLKSRC = 0x32;
    public static final int PWM_CMD_SET_POLARITY = 0x33;
    public static final int PWM_PORT_ID_A = 0;
    public static final int PWM_PORT_ID_B = 1;
    public static final int PWM_CLKSRC_XTL = 0;
    public static final int PWM_CLKSRC_RTC = 1;

    private static PWMManager me = null;
    private static boolean isEnabled = false;

    public static PWMManager getInstance() {
        if (me == null) {
            // TODO: Check if support PWM in native
            me = new PWMManager();
            isEnabled = true;
        } else {
            isEnabled = true;
        }
        return me;
    }

    public void config(int id, int freq, int dutyCycle) throws IOException {
        if (!isAvailPortId(id)) {
            throw new IOException("Not available port id " + id);
        }

        // TODO: Check value of freq and dutyCycle

        config0(id, freq, dutyCycle);
    }

    public int command(int id, int cmd, int arg) throws IOException {
        if (!isAvailPortId(id)) {
            throw new IOException("Not available port id " + id);
        }

        if (!isAvailCommand(cmd)) {
            throw new IOException("Not available command " + cmd);
        }

        int result = command0(id, cmd, arg);

        // TODO: check value of result

        return result;
    }

    private static native int config0(int id, int freq, int dutyCycle);

    private static native int command0(int id, int cmd, int arg);

    private boolean isAvailCommand(int cmd) {
        return cmd == PWM_CMD_SET_ONOFF || cmd == PWM_CMD_SET_CLKSRC || cmd == PWM_CMD_GET_CLKSRC
                || cmd == PWM_CMD_SET_POLARITY;
    }

    private boolean isAvailPortId(int id) {
        return id == PWM_PORT_ID_A || id == PWM_PORT_ID_B;
    }
}
