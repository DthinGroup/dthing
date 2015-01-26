
package iot.oem.ear;

import java.io.IOException;

public class EarManager extends Object {

    private static EarManager me = null;
    private static boolean isEnabled = false;

    public static EarManager getInstance() {
        if (me == null) {
            // TODO: Check if support Ear in native
            me = new EarManager();
            isEnabled = true;
        } else {
            isEnabled = true;
        }
        return me;
    }

    public boolean getStatus() throws IOException {
        if (!isEnabled) {
            throw new IOException("EarManager is not enabled");
        }

        int result = getStatus0();
        if (result < 0) {
            throw new IOException("Failed to get status ");
        }

        return result != 0;
    }

    public void setStatus(boolean status) throws IOException {
        if (!isEnabled) {
            throw new IOException("EarManager is not enabled");
        }

        if (setStatus0(status) < 0) {
            throw new IOException("Failed to set status " + status);
        }
    }

    public void close() throws IOException {
        if (!isEnabled) {
            throw new IOException("EarManager is not enabled");
        }

        if (close0() < 0) {
            throw new IOException("Failed to close");
        }

        isEnabled = false;
    }

    private static native int init0();

    private static native int getStatus0();

    private static native int setStatus0(boolean status);

    private static native int close0();
}
