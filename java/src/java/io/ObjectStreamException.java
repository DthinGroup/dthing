/*
 * @(#)ObjectStreamException.java	1.15 05/03/12
 *
 * Copyright � 2005 Sun Microsystems, Inc. All rights reserved.  
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */

package java.io;

import java.io.IOException;

/**
 * Superclass of all exceptions specific to Object Stream classes.
 *
 * @author  unascribed
 * @version 1.10, 02/02/00
 * @since   JDK1.1
 */
public abstract class ObjectStreamException extends IOException {
    /**
	 * 
	 */
	private static final long serialVersionUID = 3222291823500790543L;

	/**
     * Create an ObjectStreamException with the specified argument.
     *
     * @param classname the detailed message for the exception
     */
    protected ObjectStreamException(String classname) {
    	super(classname);
    }

    /**
     * Create an ObjectStreamException.
     */
    protected ObjectStreamException() {
    	super();
    }
}
