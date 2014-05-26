
package iot.oem.gpio;

public interface GpioInterruptListener {

    /**
     * Call the method when listened interruption happened
     *
     * @param gpio GPIO instance
     * @param type interruption type
     * @param value GPIO value when interrupt. {@code true} for 低电位, {@code false} for 高电位.
     */
    void onInterrupt(Gpio gpio, int type, boolean value);
}
