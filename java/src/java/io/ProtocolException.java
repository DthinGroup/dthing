package java.io;

import java.io.IOException;

public class ProtocolException extends IOException{

	private static final long serialVersionUID = -6098449442062388080L;
	/**
     * Constructs a new <code>ProtocolException</code> with the
     * specified detail message.
     *
     * @param   host   the detail message.
     */
    public ProtocolException(String host){
    	super(host);
    }

    /**
     * Constructs a new <code>ProtocolException</code> with no detail message.
     */
    public ProtocolException() {
    }
}
