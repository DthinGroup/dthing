/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package java.lang;

/**
 * The superclass of all classes which can be thrown by the VM. The
 * two direct subclasses are recoverable exceptions ({@code Exception}) and
 * unrecoverable errors ({@code Error}). This class provides common methods for
 * accessing a string message which provides extra information about the
 * circumstances in which the {@code Throwable} was created (basically an error
 * message in most cases), and for saving a stack trace (that is, a record of
 * the call stack at a particular point in time) which can be printed later.
 * <p>
 * A {@code Throwable} can also include a cause, which is a nested {@code
 * Throwable} that represents the original problem that led to this {@code
 * Throwable}. It is often used for wrapping various types of errors into a
 * common {@code Throwable} without losing the detailed original error
 * information. When printing the stack trace, the trace of the cause is
 * included.
 *
 * @see Error
 * @see Exception
 * @see RuntimeException
 */
public class Throwable {
    private static final long serialVersionUID = -3042686055658047285L;

    /**
     * The message provided when the exception was created.
     */
    private String detailMessage;


    //TODO: how to add backtrace into this variable?
    private transient int[] backtrace;
    
    /**
     * Constructs a new {@code Throwable} that includes the current stack trace.
     */
    public Throwable() {

    }

    /**
     * Constructs a new {@code Throwable} with the current stack trace and the
     * specified detail message.
     *
     * @param detailMessage
     *            the detail message for this {@code Throwable}.
     */
    public Throwable(String detailMessage) {
        this.detailMessage = detailMessage;
    }

    /**
     * Returns the extra information message which was provided when this
     * {@code Throwable} was created. Returns {@code null} if no message was
     * provided at creation time.
     *
     * @return this {@code Throwable}'s detail message.
     */
    public String getMessage() {
        return detailMessage;
    }

    /**
     * Returns the extra information message which was provided when this
     * {@code Throwable} was created. Returns {@code null} if no message was
     * provided at creation time. Subclasses may override this method to return
     * localized text for the message. Android returns the regular detail message.
     *
     * @return this {@code Throwable}'s localized detail message.
     */
    public String getLocalizedMessage() {
        return getMessage();
    }


    /**
     * Writes a printable representation of this {@code Throwable}'s stack trace
     * to the {@code System.err} stream.
     *
     */
    public void printStackTrace() {
    	java.io.PrintStream err = System.err;
    	String message = getMessage();
    	err.print(this.getClass().getName());
    	if (message != null) {
    	    err.print(": ");
    	    err.println(message);
    	} else {
    	    err.println();
    	}
    	if (backtrace != null) {
    	    printStackTrace0(System.err);
    	}
    }

    /**
     * Returns a short description of this throwable object.
     * If this <code>Throwable</code> object was
     * {@link #Throwable(String) created} with an error message string,
     * then the result is the concatenation of three strings:
     * <ul>
     * <li>The name of the actual class of this object
     * <li>": " (a colon and a space)
     * <li>The result of the {@link #getMessage} method for this object
     * </ul>
     * If this <code>Throwable</code> object was {@link #Throwable() created}
     * with no error message string, then the name of the actual class of
     * this object is returned.
     *
     * @return  a string representation of this <code>Throwable</code>.
     */
    //@Override
    public String toString() {
        String msg = getLocalizedMessage();
        String name = getClass().getName();
        if (msg == null) {
            return name;
        }
        return name + ": " + msg;
    }

    /*
     * The given object must have a void println(char[]) method 
     */ 
    private native void printStackTrace0(Object s);
}
