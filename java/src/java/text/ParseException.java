package java.text;

/**
 * Thrown when the string being parsed is not in the correct form.
 */
public class ParseException extends Exception {

    private static final long serialVersionUID = 2703218443322787634L;

    private int errorOffset;

    /**
     * Constructs a new instance of this class with its stack trace, detail
     * message and the location of the error filled in.
     *
     * @param detailMessage
     *            the detail message for this exception.
     * @param location
     *            the index at which the parse exception occurred.
     */
    public ParseException(String detailMessage, int location) {
        super(detailMessage);
        errorOffset = location;
    }

    /**
     * Returns the index at which this parse exception occurred.
     *
     * @return the location of this exception in the parsed string.
     */
    public int getErrorOffset() {
        return errorOffset;
    }
}
