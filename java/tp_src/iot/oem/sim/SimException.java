
package iot.oem.sim;

public class SimException extends Exception {

    /**
     * Constructs an {@link #SimException} with {@code null} as its error detail message.
     */
    public SimException() {
        super();
    }

    /**
     * Constructs an {@link #SimException} with the specified detail message. The error message
     * string {@code s} can later be retrieved by the {@link java.lang.Throwable#getMessage} method
     * of class {@link java.lang.Throwable}.
     *
     * @param s the detail message.
     */
    public SimException(String s) {
        super(s);
    }
}
