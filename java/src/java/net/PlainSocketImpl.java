/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements. See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package java.net;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.net.SocketException;
import java.net.SocketImpl;
import java.net.SocketTimeoutException;
import java.util.Arrays;

/**
 * @hide used in java.nio.
 */
public class PlainSocketImpl extends SocketImpl {

    // For SOCKS support. A SOCKS bind() uses the last host connected to in its request.
//    private static InetAddress lastConnectedAddress;
//    private static int lastConnectedPort;

    private boolean isInputShutdown;
    private boolean isOutputShutdown;

    private volatile int sockHandle;

    public PlainSocketImpl() {
        sockHandle = 0;
    }

    public PlainSocketImpl(int localport, InetAddress addr, int port) {
        this();
        this.localport = localport;
        this.address = addr;
        this.port = port;
    }

    /**
     * Check whether the socket is closed.
     *
     * @throws IOException if the socket is closed.
     */
    private void checkNotClosed() throws IOException {
        if (sockHandle == 0) {
            throw new SocketException("Socket is closed");
        }
    }

    protected synchronized int available() throws IOException {
        checkNotClosed();
        // we need to check if the input has been shutdown. If so we should return that there is no
        // data to be read
        if (isInputShutdown) {
            return 0;
        }
        return NetNativeBridge.available(sockHandle);
    }

    protected void bind(InetAddress addr, int port) throws IOException {
        NetNativeBridge.bind(sockHandle, addr, port);
        address = addr;
        if (port != 0) {
            localport = port;
        } else {
            localport = NetNativeBridge.getSocketLocalPort(sockHandle);
        }
    }

    protected synchronized void close() throws IOException {
        NetNativeBridge.closeSocket(sockHandle);
        sockHandle = 0;
    }

    protected void connect(String host, int port) throws IOException {
        connect(InetAddress.getByName(host), port);
    }

    protected void connect(InetAddress addr, int port) throws IOException {
        connect(addr, port, 0);
    }

    protected void connect(SocketAddress remoteAddr, int timeout) throws IOException {
        InetSocketAddress inetAddr = (InetSocketAddress) remoteAddr;
        connect(inetAddr.getAddress(), inetAddr.getPort(), timeout);
    }

    /**
     * Connects this socket to the specified remote host address/port.
     *
     * @param addr the remote host address to connect to
     * @param port the remote port to connect to
     * @param timeout a timeout where supported. 0 means no timeout
     * @throws IOException if an error occurs while connecting
     */
    private void connect(InetAddress addr, int port, int timeout) throws IOException {
        InetAddress normAddr = addr.isAnyLocalAddress() ? InetAddress.getLocalHost() : addr;

        NetNativeBridge.connect(sockHandle, normAddr, port, timeout);

        address = normAddr;
        this.port = port;
    }

    protected void create() throws IOException {
        sockHandle = NetNativeBridge.socket(true);
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

        public PlainSocketInputStream(PlainSocketImpl impl) {
            socketImpl = impl;
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
        return new Integer(NetNativeBridge.getSocketOption(sockHandle, option));
    }

    protected synchronized OutputStream getOutputStream() throws IOException {
        checkNotClosed();
        return new PlainSocketOutputStream(this);
    }

    private static class PlainSocketOutputStream extends OutputStream {
        private final PlainSocketImpl socketImpl;

        public PlainSocketOutputStream(PlainSocketImpl impl) {
            socketImpl = impl;
        }

        public void close() throws IOException {
            socketImpl.close();
        }

        public void write(int oneByte) throws IOException {
            byte[] buffer = new byte[1];
            buffer[0] = (byte) (oneByte & 0xff);
            write(buffer, 0, 1);
        }

        public void write(byte[] buffer, int offset, int byteCount) throws IOException {
            socketImpl.write(buffer, offset, byteCount);
        }
    }

    public void setOption(int option, int value) throws SocketException {
        NetNativeBridge.setSocketOption(sockHandle, option, value);
    }

    /**
     * Shutdown the input portion of the socket.
     */
    protected void shutdownInput() throws IOException {
        isInputShutdown = NetNativeBridge.shutdownInput(sockHandle);
    }

    /**
     * Shutdown the output portion of the socket.
     */
    protected void shutdownOutput() throws IOException {
        isOutputShutdown = NetNativeBridge.shutdownOutput(sockHandle);
    }

    /**
     * For PlainSocketInputStream.
     */
    private int read(byte[] buffer, int offset, int byteCount) throws IOException {
        if (byteCount == 0) {
            return 0;
        }
        Arrays.checkOffsetAndCount(buffer.length, offset, byteCount);
        if (isInputShutdown) {
            return -1;
        }
        int readCount = NetNativeBridge.recv(sockHandle, buffer, offset, byteCount, 0);
        // Return of zero bytes for a blocking socket means a timeout occurred
        if (readCount == 0) {
            throw new SocketTimeoutException();
        }
        // Return of -1 indicates the peer was closed
        if (readCount == -1) {
            isInputShutdown = true;
        }
        return readCount;
    }

    /**
     * For PlainSocketOutputStream.
     */
    private void write(byte[] buffer, int offset, int byteCount) throws IOException {
        Arrays.checkOffsetAndCount(buffer.length, offset, byteCount);
        if (isOutputShutdown) {
            return;
        }
        while (byteCount > 0) {
            int bytesWritten = NetNativeBridge.send(sockHandle, buffer, offset, byteCount, 0);
            byteCount -= bytesWritten;
            offset += bytesWritten;
        }
    }
}
