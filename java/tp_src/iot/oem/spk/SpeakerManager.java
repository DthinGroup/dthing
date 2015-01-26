
package iot.oem.spk;

import java.io.IOException;

public class SpeakerManager extends Object {

    private static SpeakerManager me = null;
    private static boolean isEnabled = false;

    public static SpeakerManager getInstance() {
        if (me == null) {
            // TODO: Check if support speaker in native
            me = new SpeakerManager();
            isEnabled = true;
        } else {
            isEnabled = true;
        }
        return me;
    }

    public boolean getStatus() throws IOException {
        if (!isEnabled) {
            throw new IOException("SpeakerManager is not enabled");
        }

        int result = getStatus0();
        if (result < 0) {
            throw new IOException("Failed to get status ");
        }
        return result != 0;
    }

    public void setStatus(boolean status) throws IOException {
        if (!isEnabled) {
            throw new IOException("SpeakerManager is not enabled");
        }

        if (setStatus0(status) < 0) {
            throw new IOException("Failed to set status " + status);
        }
    }

    public void close() throws IOException {
        if (!isEnabled) {
            throw new IOException("SpeakerManager is not enabled");
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
