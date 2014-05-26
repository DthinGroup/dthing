
package iot.oem.gpio;

import java.io.IOException;

public class Gpio extends Object {

    public static final int WRITE_MODE = 0;
    public static final int READ_MODE = 1;
    public static final int RDWR_MODE = 2;

    public static final int INTERRUPT_TYPE_LOW = 0;
    public static final int INTERRUPT_TYPE_HIGH = 1;
    public static final int INTERRUPT_TYPE_HIGH_TO_LOW = 2;
    public static final int INTERRUPT_TYPE_LOW_TO_HIGH = 3;
    public static final int INTERRUPT_TYPE_BOTH_EDGES = 4;

    private int number = -1; // GPIO number
    private int mode = -1; // Access mode
    private boolean isEnabled = false;

    /**
     * Construct GPIO object by GPIO number
     *
     * @param number GPIO number
     * @exception IllegalArgumentException when has illegal arguments
     * @exception IOException when failed to create instance
     * @return GPIO instance
     */
    public Gpio(int number) throws IllegalArgumentException, IOException {
        // Check if GPIO instance is available
        if (!isValidNumber(number)) {
            throw new IllegalArgumentException("Illegal gpio number");
        }

        this.number = number;

        if (open0(number) < 0) {
            throw new IOException("Failed to open gpio pin");
        }

        isEnabled = true;
    }

    private static native int open0(int number);

    /**
     * Get GPIO number of current GPIO instance
     *
     * @return current GPIO number
     */
    public int getNumber() {
        return number;
    }

    /**
     * Set current access mode for GPIO
     *
     * @param mode {@link #WRITE_MODE}, {@link #READ_MODE} or {@link #RDWR_MODE}.
     * @exception IllegalArgumentException when has illegal arguments
     * @exception IOException when failed to set or GPIO instance is destroyed
     */
    public void setCurrentMode(int mode) throws IllegalArgumentException, IOException {
        // Check if GPIO instance is available
        if (!isAvailable()) {
            throw new IOException("Gpio instance is destroyed or not initialized");
        }

        if (!this.isValidMode(mode)) {
            throw new IllegalArgumentException("Unknown gpio access mode " + mode);
        }

        // TODO: Consider of asynchronous implementation
        if (setCurrentMode0(number, mode) >= 0) {
            this.mode = mode;
        } else {
            throw new IOException("Failed to set gpio mode " + mode);
        }
    }

    private static native int setCurrentMode0(int number, int mode);

    /**
     * Get current GPIO access mode
     *
     * @exception IOException when failed to get or GPIO instance is destroyed
     * @return current GPIO access mode
     */
    public int getCurrentMode() throws IOException {
        // Check if GPIO instance is available
        if (!isAvailable()) {
            throw new IOException("Gpio instance is destroyed or not initialized");
        }

        // TODO: Consider of asynchronous implementation
        int curMode = getCurrentMode0(number);

        if (isValidMode(curMode)) {
            mode = curMode;
        } else {
            throw new IOException("Failed to get gpio mode " + curMode);
        }

        return mode;
    }

    private static native int getCurrentMode0(int number);

    /**
     * Read current GPIO value
     *
     * @exception IOException when failed to read or GPIO instance is destroyed
     * @return GPIO value. {@code true} for high electric potential, {@code false} for low electric
     * potential
     */
    synchronized public boolean read() throws IOException {
        // Check if GPIO instance is available
        if (!isAvailable()) {
            throw new IOException("Gpio instance is destroyed or not initialized");
        }

        // FIXME: To pass TCK, maybe not proper to check it in OEM layer
        if (READ_MODE != getCurrentMode()) {
            throw new IOException("Gpio mode should be Gpio.READ_MODE");
        }

        // TODO: Consider of asynchronous implementation
        int ret = read0(number);
        if (ret < 0) {
            throw new IOException("Failed to read gpio " + number);
        }

        return ret != 0;
    }

    private static native int read0(int number);

    /**
     * Write current GPIO value
     *
     * @param value GPIO value to write in. {@code true} for high electric potential, {@code false}
     * for low electric potential
     * @exception IOException when failed to write or GPIO instance is destroyed
     */
    synchronized public void write(boolean value) throws IOException {
        // Check if GPIO instance is available
        if (!isAvailable()) {
            throw new IOException("Gpio instance is destroyed or not initialized");
        }

        // FIXME: To pass TCK, maybe not proper to check it in OEM layer
        if (WRITE_MODE != getCurrentMode()) {
            throw new IOException("Gpio mode should be Gpio.WRITE_MODE");
        }

        if (write0(number, value) < 0) {
            throw new IOException("Failed to write value to Gpio " + number);
        }
    }

    private static native int write0(int number, boolean value);

    /**
     * Register listener for interruption.
     *
     * @param listener GpioInterruptListener interrupt listener
     * @param type type of interruption
     * @exception NullPointerException when parameter is null
     * @exception IllegalArgumentException when type is illegal
     * @exception IOException when failed to register or GPIO instance is destroyed
     */
    public void registerInterruptListener(GpioInterruptListener listener, int type)
            throws NullPointerException, IllegalArgumentException, IOException {
        if (listener == null) {
            throw new NullPointerException("Null listener");
        }

        if (!isValidIntType(type)) {
            throw new IllegalArgumentException("Illegal int type");
        }

        // Check if GPIO instance is available
        if (!isAvailable()) {
            throw new IOException("Gpio instance is destroyed or not initialized");
        }

        // TODO: push the listener with GPIO id type

        if (registerInt0(number, type) < 0) {
            throw new IOException("Failed to register");
        }
    }

    private static native int registerInt0(int number, int type);

    private void registeredIntCallback(int gpioId, int gpio_state) {
        GpioInterruptListener listener = null;
        int type = 0; // TODO: initial with concrete value
        boolean value = false;

        // TODO: Find listener via gpioId

        try {
            value = read();
        } catch (IOException e) {
            e.printStackTrace();
        }
        if (listener != null) {
            listener.onInterrupt(this, type, value);
        }
    }

    /**
     * Unregister specified listener
     *
     * @param listener GpioInterruptListener interrupt listener
     * @exception NullPointerException when parameter is null
     * @exception IllegalArgumentException when listener is not registered
     * @exception IOException when failed to unregister or GPIO instance is destroyed
     */
    public void unRegisterInterruptListener(GpioInterruptListener listener)
            throws NullPointerException, IllegalArgumentException, IOException {
        if (listener == null) {
            throw new NullPointerException("Null listener");
        }

        // Check if GPIO instance is available
        if (!isAvailable()) {
            throw new IOException("Gpio instance is destroyed or not initialized");
        }

        // TODO: Check if registered

        if (unregisterInt0(number) < 0) {
            throw new IOException("Failed to unregister");
        }
    }

    private static native int unregisterInt0(int number);

    /**
     * Destroy current GPIO instance
     *
     * @exception IOException when failed to release resource
     */
    public void destroy() throws IOException {
        // Check if GPIO instance is available
        if (!isAvailable()) {
            throw new IOException("Gpio instance is destroyed or not initialized");
        }

        if (close0(number) < 0) {
            throw new IOException("Failed to release resource for Gpio " + number);
        }

        number = -1;
        mode = -1;
        isEnabled = false;
    }

    private static native int close0(int number);

    private boolean isValidMode(int mode) {
        return mode == READ_MODE || mode == WRITE_MODE || mode == RDWR_MODE;
    }

    private boolean isValidIntType(int type) {
        return type == INTERRUPT_TYPE_LOW || type == INTERRUPT_TYPE_HIGH
                || type == INTERRUPT_TYPE_HIGH_TO_LOW || type == INTERRUPT_TYPE_LOW_TO_HIGH
                || type == INTERRUPT_TYPE_BOTH_EDGES;
    }

    private boolean isValidNumber(int number) {
        // Check if GPIO number is valid
        return number >= 0;
    }

    private boolean isAvailable() {
        // Check if current GPIO instance is available
        return number >= 0 && isEnabled;
    }
}
