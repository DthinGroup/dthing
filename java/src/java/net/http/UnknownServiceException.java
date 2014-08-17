package java.net.http;

import java.io.IOException;

public class UnknownServiceException extends IOException {

	private static final long serialVersionUID = -4169033248853639508L;

	/**
     * Constructs a new <code>UnknownServiceException</code> with no 
     * detail message. 
     */
    public UnknownServiceException() {
    }

    /**
     * Constructs a new <code>UnknownServiceException</code> with the 
     * specified detail message. 
     *
     * @param   msg   the detail message.
     */
    public UnknownServiceException(String msg) {
    	super(msg);
    }
}
