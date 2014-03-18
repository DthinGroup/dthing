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

package java.net;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.ConnectException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.SocketException;
import java.net.SocketImpl;
import java.net.SocketTimeoutException;
import java.lang.ArrayIndexOutOfBoundsException;
//import libcore.io.ErrnoException;


/**
 * @hide used in java.nio.
 */
public class PlainSocketImpl extends SocketImpl {

    // For SOCKS support. A SOCKS bind() uses the last
    // host connected to in its request.
    private static InetAddress lastConnectedAddress;

    private static int lastConnectedPort;

    private boolean shutdownInput;
    
    private volatile int sockHandle;


    public PlainSocketImpl() {
        this.sockHandle = 0;
    }

    public PlainSocketImpl(int localport, InetAddress addr, int port) {
        this();
        this.localport = localport;
        this.address = addr;
        this.port = port;
    }

    public void initLocalPort(int localPort) {
        this.localport = localPort;
    }

    public void initRemoteAddressAndPort(InetAddress remoteAddress, int remotePort) {
        this.address = remoteAddress;
        this.port = remotePort;
    }

    private void checkNotClosed() throws IOException {
        if (this.sockHandle==0) 
        {
            throw new SocketException("Socket is closed");
        }
    }

    protected synchronized int available() throws IOException {
        checkNotClosed();
        // we need to check if the input has been shutdown. If so
        // we should return that there is no data to be read
        if (shutdownInput) {
            return 0;
        }
        return NetNativeBridge.available(sockHandle);
    }

    protected void bind(InetAddress address, int port) throws IOException {
        NetNativeBridge.bind(sockHandle, address, port);
        this.address = address;
        if (port != 0) {
            this.localport = port;
        } else {
            this.localport = NetNativeBridge.getSocketLocalPort(sockHandle);
        }
    }

    
    protected synchronized void close() throws IOException {
        NetNativeBridge.closeSocket(sockHandle);
    }

    
    protected void connect(String aHost, int aPort) throws IOException {
        //connect(InetAddress.getByName(aHost), aPort);
    }

    
    protected void connect(InetAddress anAddr, int aPort) throws IOException {
        connect(anAddr, aPort, 0);
    }

    /**
     * Connects this socket to the specified remote host address/port.
     *
     * @param anAddr
     *            the remote host address to connect to
     * @param aPort
     *            the remote port to connect to
     * @param timeout
     *            a timeout where supported. 0 means no timeout
     * @throws IOException
     *             if an error occurs while connecting
     */
    private void connect(InetAddress anAddr, int aPort, int timeout) throws IOException {
        InetAddress normalAddr = anAddr.isAnyLocalAddress() ? InetAddress.getLocalHost() : anAddr;

        NetNativeBridge.connect(sockHandle, normalAddr, aPort, timeout);

        super.address = normalAddr;
        super.port = aPort;
    }

    
    protected void create() throws IOException {
        this.sockHandle = NetNativeBridge.socket(true);
    }

    protected void finalize() throws Throwable {
        try {
            close();
        } finally {
            super.finalize();
        }
    }

    protected synchronized InputStream getInputStream() throws IOException {
        checkNotClosed();
        return new PlainSocketInputStream(this);
    }

    private static class PlainSocketInputStream extends InputStream {
        private final PlainSocketImpl socketImpl;

        public PlainSocketInputStream(PlainSocketImpl socketImpl) {
            this.socketImpl = socketImpl;
        }

        public int available() throws IOException {
            return socketImpl.available();
        }

        
        public void close() throws IOException {
            socketImpl.close();
        }

        public int read() throws IOException {            
            byte[] buffer = new byte[1];
        	int result = read(buffer, 0, 1);
        	return (result != -1) ? buffer[0] & 0xff : -1;
        }

        public int read(byte[] buffer, int offset, int byteCount) throws IOException {
            return socketImpl.read(buffer, offset, byteCount);
        }
    }

    public Object getOption(int option) throws SocketException {
        return NetNativeBridge.getSocketOption(sockHandle, option);
    }

    protected synchronized OutputStream getOutputStream() throws IOException {
        checkNotClosed();
        return new PlainSocketOutputStream(this);
    }

    private static class PlainSocketOutputStream extends OutputStream {
        private final PlainSocketImpl socketImpl;

        public PlainSocketOutputStream(PlainSocketImpl socketImpl) {
            this.socketImpl = socketImpl;
        }

        public void close() throws IOException {
            this.socketImpl.close();
        }

        public void write(int oneByte) throws IOException {
            byte[] buffer = new byte[1];
	        buffer[0] = (byte) (oneByte & 0xff);
	        write(buffer,0,1);
        }

        public void write(byte[] buffer, int offset, int byteCount) throws IOException {
            this.socketImpl.write(buffer, offset, byteCount);
        }
    }

    public void setOption(int option, int value) throws SocketException {
        NetNativeBridge.setSocketOption(sockHandle, option, value);
    }

    /**
     * Shutdown the input portion of the socket.
     */
    protected void shutdownInput() throws IOException {
        shutdownInput = true;
//        try {
//            Libcore.os.shutdown(sockHandle, SHUT_RD);
//        } catch (ErrnoException errnoException) {
//            throw errnoException.rethrowAsSocketException();
//        }
    }

    /**
     * Shutdown the output portion of the socket.
     */
    protected void shutdownOutput() throws IOException {
//        try {
//            Libcore.os.shutdown(sockHandle, SHUT_WR);
//        } catch (ErrnoException errnoException) {
//            throw errnoException.rethrowAsSocketException();
//        }
    }


    protected void connect(SocketAddress remoteAddr, int timeout) throws IOException {
        InetSocketAddress inetAddr = (InetSocketAddress) remoteAddr;
        connect(inetAddr.getAddress(), inetAddr.getPort(), timeout);
    }


    /**
     * For PlainSocketInputStream.
     */
    private int read(byte[] buffer, int offset, int byteCount) throws IOException {
        if (byteCount == 0) {
            return 0;
        }
        PlainSocketImpl.checkOffsetAndCount(buffer.length, offset, byteCount);
        if (shutdownInput) {
            return -1;
        }
        int readCount = NetNativeBridge.recv(sockHandle, buffer, offset, byteCount, 0);
        // Return of zero bytes for a blocking socket means a timeout occurred
        if (readCount == 0) {
            throw new SocketTimeoutException();
        }
        // Return of -1 indicates the peer was closed
        if (readCount == -1) {
            shutdownInput = true;
        }
        return readCount;
    }

    /**
     * For PlainSocketOutputStream.
     */
    private void write(byte[] buffer, int offset, int byteCount) throws IOException {
        PlainSocketImpl.checkOffsetAndCount(buffer.length, offset, byteCount);
        while (byteCount > 0) {
                int bytesWritten = NetNativeBridge.send(sockHandle, buffer, offset, byteCount, 0);
                byteCount -= bytesWritten;
                offset += bytesWritten;
        }
    }
    
    /**
     * Checks that the range described by {@code offset} and {@code count} doesn't exceed
     * {@code arrayLength}.
     *
     * @hide
     */
    public static void checkOffsetAndCount(int arrayLength, int offset, int count) {
        if ((offset | count) < 0 || offset > arrayLength || arrayLength - offset < count) {
            throw new ArrayIndexOutOfBoundsException("ArrayIndexOutOfBoundsException");
        }
    }
}
