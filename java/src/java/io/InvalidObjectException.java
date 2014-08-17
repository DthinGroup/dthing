/*
 * @(#)InvalidObjectException.java	1.17 05/03/12
 *
 * Copyright � 2005 Sun Microsystems, Inc. All rights reserved.  
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 */

package java.io;

import java.io.ObjectStreamException;

/**
 * Indicates that one or more deserialized objects failed validation
 * tests.  The argument should provide the reason for the failure.
 *
 * @see ObjectInputValidation
 * @since JDK1.1
 *
 * @author  unascribed
 * @version 1.12, 02/02/00
 * @since   JDK1.1
 */
public class InvalidObjectException extends ObjectStreamException {
    /**
	 * 
	 */
	private static final long serialVersionUID = 267068200329906089L;

	/**
     * Constructs an <code>InvalidObjectException</code>.
     * @param reason Detailed message explaing the reason for the failure.
     *
     * @see ObjectInputValidation
     */
    public  InvalidObjectException(String reason) {
    	super(reason);
    }
}
