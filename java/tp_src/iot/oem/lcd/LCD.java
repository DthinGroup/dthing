
package iot.oem.lcd;

import java.io.IOException;

public class LCD {

    private static LCD me = null;

    /**
     * Get unique LCD instance
     *
     * @return unique LCD instance
     */
    public static LCD getInstance() {
        if (me == null) {
            me = new LCD();
        }
        return me;
    }

    /**
     * Set LCD background status
     *
     * @param status boolean background status, {@code true} for open, {@code false} for closed
     * @exception IOException When failed to set LCD background status
     */
    public void setBackgroundStatus(boolean status) throws IOException {
        if (setBackgroundStatus0(status) < 0) {
            throw new IOException("Failed to set background status to " + status);
        }
    }

    private static native int setBackgroundStatus0(boolean status);

    /**
     * Get LCD background status
     *
     * @exception IOException When failed to get LCD background status
     * @return background status, {@code true} for open, {@code false} for closed
     */
    public boolean getBackgroundStatus() throws IOException {
        int result = getBackgroundStatus0();
        if (result < 0) {
            throw new IOException("Failed to get background status");
        }

        return result > 0;
    }

    private static native int getBackgroundStatus0();

    /**
     * Get LCD height
     *
     * @return LCD height (by pixel)
     */
    public int getHeight() {
        // TODO:
        return 10;
    }

    private static native int getHeight0();

    /**
     * Get LCD width
     *
     * @return LCD width (by pixel)
     */
    public int getWidth() {
        // TODO:
        return 10;
    }

    private static native int getWidth0();

    /**
     * Draw String at specified position
     *
     * @param x x position (by pixel)
     * @param y y position (by pixel)
     * @param value string to draw
     * @exception IllegalArgumentException When illegal argument
     * @exception NullPointerException When access null pointer
     * @exception IOException When failed to draw string
     */
    public void drawString(int x, int y, String value) throws IllegalArgumentException,
            NullPointerException, IOException {
        if (value == null) {
            throw new NullPointerException("Null pointer");
        }

        if (x < 0 || y < 0 || value.length() == 0) {
            throw new IllegalArgumentException("Illegal argument");
        }

        if (drawString0(x, y, value) < 0) {
            throw new IOException("Failed to draw String " + value);
        }
    }

    private static native int drawString0(int x, int y, String value);

    /**
     * Clean LCD displaying content
     *
     * @exception IOException when failed to clean screen
     */
    public void clean() throws IOException {
        if (clean0() < 0) {
            throw new IOException("Failed to clean screen");
        }
    }

    private static native int clean0();
}
