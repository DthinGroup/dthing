/*
 *  Licensed to the Apache Software Foundation (ASF) under one or more
 *  contributor license agreements.  See the NOTICE file distributed with
 *  this work for additional information regarding copyright ownership.
 *  The ASF licenses this file to You under the Apache License, Version 2.0
 *  (the "License"); you may not use this file except in compliance with
 *  the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

package java.io;


/**
 * Wraps a Unix file descriptor. It's possible to get the file descriptor used by some
 * classes (such as {@link FileInputStream}, {@link FileOutputStream},
 * and {@link RandomAccessFile}), and then create new streams that point to the same
 * file descriptor.
 */
public final class FileDescriptor {

    /* FileDescriptor type IDs */
    static final int INVALID = -1;
    static final int STDIN = 0;
    static final int STDOUT = 1;
    static final int STDERR = 2;

    protected int handle = -1;
    /**
     * Corresponds to {@code stdin}.
     */
    public static final FileDescriptor in = new FileDescriptor(STDIN);

    /**
     * Corresponds to {@code stdout}.
     */
    public static final FileDescriptor out = new FileDescriptor(STDOUT);

    /**
     * Corresponds to {@code stderr}.
     */
    public static final FileDescriptor err = new FileDescriptor(STDERR);

    /**
     * The Unix file descriptor backing this FileDescriptor.
     * A value of -1 indicates that this FileDescriptor is invalid.
     */
    private int fd = -1;


    /**
     * Constructs a new invalid FileDescriptor.
     */
    public FileDescriptor() {
    }
    
    private FileDescriptor(int fd) {
    	this.fd = fd;
    }

    /**
     * Ensures that data which is buffered within the underlying implementation
     * is written out to the appropriate device before returning.
     */
    public void sync() throws SyncFailedException {
    	// TODO: to be implemented
    }

    /**
     * Tests whether this {@code FileDescriptor} is valid.
     */
    public boolean valid() {
        return fd != -1;
    }

    /**
     * Returns the int fd. It's highly unlikely you should be calling this. Please discuss
     * your needs with a libcore maintainer before using this method.
     * @hide internal use only
     */
    public final int getInt$() {
        return fd;
    }

    /**
     * Sets the int fd. It's highly unlikely you should be calling this. Please discuss
     * your needs with a libcore maintainer before using this method.
     * @hide internal use only
     */
    public final void setInt$(int fd) {
        this.fd = fd;
    }

    //@Override 
    public String toString() {
        return "FileDescriptor[" + fd + "]";
    }
}
