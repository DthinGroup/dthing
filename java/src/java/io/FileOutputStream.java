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

//Refer to: http://opengrok-cn.myriad.int:8180/masterpack/xref/dev/cfg/cldc/cdc/11/java/sun/java/io/FileOutputStream.java#openAppend

/**
 * An output stream that writes bytes to a file. If the output file exists, it
 * can be replaced or appended to. If it does not exist, a new file will be
 * created.
 * <pre>   {@code
 *   File file = ...
 *   OutputStream out = null;
 *   try {
 *     out = new BufferedOutputStream(new FileOutputStream(file));
 *     ...
 *   } finally {
 *     if (out != null) {
 *       out.close();
 *     }
 *   }
 * }</pre>
 *
 * <p>This stream is <strong>not buffered</strong>. Most callers should wrap
 * this stream with a {@link BufferedOutputStream}.
 *
 * <p>Use {@link FileWriter} to write characters, as opposed to bytes, to a file.
 *
 * @see BufferedOutputStream
 * @see FileInputStream
 */
public class FileOutputStream extends OutputStream {

    private FileDescriptor fd;
    
    private boolean append =false;    
    
    private native int writeFile(int handle,byte[] b,int off,int count);
    
    private native int openFile(String name,boolean append);
    
    private native boolean closeFile(int handle);
    /**
     * Constructs a new {@code FileOutputStream} that writes to {@code file}. The file will be
     * truncated if it exists, and created if it doesn't exist.
     *
     * @throws FileNotFoundException if file cannot be opened for writing.
     */
    public FileOutputStream(File file) throws FileNotFoundException {
        this(file, false);
    }

    /**
     * Constructs a new {@code FileOutputStream} that writes to {@code file}.
     * If {@code append} is true and the file already exists, it will be appended to; otherwise
     * it will be truncated. The file will be created if it does not exist.
     *
     * @throws FileNotFoundException if the file cannot be opened for writing.
     */
    public FileOutputStream(File file, boolean append) throws FileNotFoundException {
        if (file == null) {
            throw new NullPointerException("file == null");
        }
        String name = file.getPath();
        if(name ==null){
        	throw new NullPointerException();
        }
        this.fd = new FileDescriptor();
        this.append = append;
        fd.handle = openFile(name,append); 
        if(fd.handle <=0){
        	throw new FileNotFoundException("FileOutputStream:open " + name + " fail,file not found!");
        }
    }

    /**
     * Constructs a new {@code FileOutputStream} that writes to {@code fd}.
     *
     * @throws NullPointerException if {@code fd} is null.
     */
    public FileOutputStream(FileDescriptor fd) {
        if (fd == null) {
            throw new NullPointerException("fd == null");
        }
        // TODO: to be implemented
    }

    /**
     * Constructs a new {@code FileOutputStream} that writes to {@code path}. The file will be
     * truncated if it exists, and created if it doesn't exist.
     *
     * @throws FileNotFoundException if file cannot be opened for writing.
     */
    public FileOutputStream(String path) throws FileNotFoundException {
        this(path, false);
    }

    /**
     * Constructs a new {@code FileOutputStream} that writes to {@code path}.
     * If {@code append} is true and the file already exists, it will be appended to; otherwise
     * it will be truncated. The file will be created if it does not exist.
     *
     * @throws FileNotFoundException if the file cannot be opened for writing.
     */
    public FileOutputStream(String path, boolean append) throws FileNotFoundException {
        this(new File(path), append);
    }

    //@Override
    public void close() throws IOException {
    	closeFile(fd.handle);
    }

    //@Override
	public void flush() throws IOException {
		super.flush();
	}

	//@Override 
    protected void finalize() throws IOException {
        if(fd !=null){
        	close();        	
        }
    }

    /**
     * Returns the underlying file descriptor.
     */
    public final FileDescriptor getFD() throws IOException {
        return fd;
    }

    //@Override
    public void write(byte[] buffer, int byteOffset, int byteCount) throws IOException {
        int byteWritten = 0;
        int ret = 0;

        if(byteCount < 0){
            return;
        }

        if(byteOffset < 0 || byteCount < 0
            || buffer.length < byteOffset + byteCount) {
            throw new IOException("FileOutput write: out of bounds");
        }

        while(byteWritten < byteCount)
        {
            ret = writeFile(fd.handle, buffer, byteOffset + byteWritten, byteCount - byteWritten);

            if(ret < 0) {
                throw new IOException("FileOutput: write IO Exception");
            }
            byteWritten += ret;
        }
    }

    public void write(byte[] buffer) throws IOException {
    	write(buffer,0,buffer.length);
    }

    //@Override
    public void write(int oneByte) throws IOException {
        byte[] b = {(byte) oneByte};
        write(b,0,1);
    }
}
