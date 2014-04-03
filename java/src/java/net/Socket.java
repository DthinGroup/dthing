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
//import libcore.io.IoBridge;

/**
 * Provides a client-side TCP socket.
 */
public class Socket {
    final SocketImpl impl;

    volatile boolean isCreated = false;
    private boolean isBound = false;
    private boolean isConnected = false;
    private boolean isClosed = false;
    private boolean isInputShutdown = false;
    private boolean isOutputShutdown = false;

    private InetAddress localAddress = Inet4Address.ANY;

    private final Object connectLock = new Object();

    /**
     * Creates a new unconnected socket. When a SocketImplFactory is defined it
     * creates the internal socket implementation, otherwise the default socket
     * implementation will be used for this socket.
     *
     * @see SocketImplFactory
     * @see SocketImpl
     */
    public Socket() {
        this.impl = new PlainSocketImpl();
    }

	/**
	 * If the argument addr represents an IP
	 * address in dot notation format "%d.%d.%d.%d", returns
	 * the int value of it
	 *
	 * @param     addr The candidate
	 * @return the int value of the argument addr if it
	 * represents an IP address in dot notation format
	 * "%d.%d.%d.%d".
	 * @exception IllegalArgumentException If the argument addr
	 * does not represents an IP
	 * address in dot notation format "%d.%d.%d.%d"
	 **/
	private static byte[] getIP(String addr) throws IllegalArgumentException {
		byte[] ip = new byte[4];
	    int i = addr.indexOf('.');
	    ip[0] = Byte.parseByte(addr.substring(0, i++));
	    int j = addr.indexOf('.',i);
	    ip[1] = Byte.parseByte(addr.substring(i, j++));
	    i = addr.indexOf('.',j);
	    ip[2] = Byte.parseByte(addr.substring(j, i++));
	
	    ip[3] = Byte.parseByte(addr.substring(i));
	    
	    return ip;
	}

    /**
     * Tries to connect a socket to all IP addresses of the given hostname.
     *
     * @param dstip
     *            the target IP address to connect to.
     * @param dstPort
     *            the port on the target host to connect to.
     * @param localAddress
     *            the address on the local host to bind to.
     * @param localPort
     *            the port on the local host to bind to.
     * @throws UnknownHostException
     *             if the host name could not be resolved into an IP address.
     * @throws IOException
     *             if an error occurs while creating the socket.
     */
    private void tryAllAddresses(String dstip, int dstPort, InetAddress
            localAddress, int localPort) throws IOException 
    {
    	
    	byte[] ip ;
    	
    	try
    	{
    		ip = getIP(dstip);
    	}
    	catch(IllegalArgumentException e)
    	{
    		throw new IOException("illegal ip address");
    	}
    	
    	InetAddress dstAddress = new Inet4Address(ip);

        checkDestination(dstAddress, dstPort);
        startupSocket(dstAddress, dstPort, localAddress, localPort);
    }

    /**
     * Creates a new streaming socket connected to the target host specified by
     * the parameters {@code dstName} and {@code dstPort}. The socket is bound
     * to any available port on the local host.
     *
     * <p>This implementation tries each IP address for the given hostname (in
     * <a href="http://www.ietf.org/rfc/rfc3484.txt">RFC 3484</a> order)
     * until it either connects successfully or it exhausts the set.
     *
     * @param dstIp
     *            the target IP address to connect to. cann't be host name
     * @param dstPort
     *            the port on the target host to connect to.
     * @throws UnknownHostException
     *             if the host name could not be resolved into an IP address.
     * @throws IOException
     *             if an error occurs while creating the socket.
     */
    public Socket(String dstIp, int dstPort) throws UnknownHostException, IOException {
        this(dstIp, dstPort, null, 0);
    }

    /**
     * Creates a new streaming socket connected to the target host specified by
     * the parameters {@code dstName} and {@code dstPort}. On the local endpoint
     * the socket is bound to the given address {@code localAddress} on port
     * {@code localPort}. If {@code host} is {@code null} a loopback address is used to connect to.
     *
     * <p>This implementation tries each IP address for the given hostname (in
     * <a href="http://www.ietf.org/rfc/rfc3484.txt">RFC 3484</a> order)
     * until it either connects successfully or it exhausts the set.
     *
     * @param dstName
     *            the target IP address to connect to.
     * @param dstPort
     *            the port on the target host to connect to.
     * @param localAddress
     *            the address on the local host to bind to.
     * @param localPort
     *            the port on the local host to bind to.
     * @throws UnknownHostException
     *             if the host name could not be resolved into an IP address.
     * @throws IOException
     *             if an error occurs while creating the socket.
     */
    public Socket(String dstIp, int dstPort, InetAddress localAddress, int localPort) throws IOException {
        this();
        tryAllAddresses(dstIp, dstPort, localAddress, localPort);
    }

    /**
     * Creates a new streaming socket connected to the target host specified by
     * the parameters {@code dstAddress} and {@code dstPort}. The socket is
     * bound to any available port on the local host.
     *
     * @param dstAddress
     *            the target host address to connect to.
     * @param dstPort
     *            the port on the target host to connect to.
     * @throws IOException
     *             if an error occurs while creating the socket.
     */
    public Socket(InetAddress dstAddress, int dstPort) throws IOException {
        this();
        checkDestination(dstAddress, dstPort);
        startupSocket(dstAddress, dstPort, null, 0);
    }

    /**
     * Creates a new streaming socket connected to the target host specified by
     * the parameters {@code dstAddress} and {@code dstPort}. On the local
     * endpoint the socket is bound to the given address {@code localAddress} on
     * port {@code localPort}.
     *
     * @param dstAddress
     *            the target host address to connect to.
     * @param dstPort
     *            the port on the target host to connect to.
     * @param localAddress
     *            the address on the local host to bind to.
     * @param localPort
     *            the port on the local host to bind to.
     * @throws IOException
     *             if an error occurs while creating the socket.
     */
    public Socket(InetAddress dstAddress, int dstPort,InetAddress localAddress, int localPort) throws IOException {
        this();
        checkDestination(dstAddress, dstPort);
        startupSocket(dstAddress, dstPort, localAddress, localPort);
    }


    /**
     * Creates an unconnected socket with the given socket implementation.
     *
     * @param impl
     *            the socket implementation to be used.
     * @throws SocketException
     *             if an error occurs while creating the socket.
     */
    protected Socket(SocketImpl impl) throws SocketException {
        this.impl = impl;
    }

    /**
     * Checks whether the connection destination satisfies the security policy
     * and the validity of the port range.
     *
     * @param destAddr
     *            the destination host address.
     * @param dstPort
     *            the port on the destination host.
     */
    private void checkDestination(InetAddress destAddr, int dstPort) {
        if (dstPort < 0 || dstPort > 65535) {
            throw new IllegalArgumentException("Port out of range: " + dstPort);
        }
    }

    /**
     * Closes the socket. It is not possible to reconnect or rebind to this
     * socket thereafter which means a new socket instance has to be created.
     *
     * @throws IOException
     *             if an error occurs while closing the socket.
     */
    public synchronized void close() throws IOException {
        isClosed = true;
        // RI compatibility: the RI returns the any address (but the original local port) after close.
        localAddress = Inet4Address.ANY;
        impl.close();
    }

    /**
     * Returns the IP address of the target host this socket is connected to, or null if this
     * socket is not yet connected.
     */
    public InetAddress getInetAddress() {
        if (!isConnected()) {
            return null;
        }
        return impl.getInetAddress();
    }

    /**
     * Returns an input stream to read data from this socket.
     *
     * @return the byte-oriented input stream.
     * @throws IOException
     *             if an error occurs while creating the input stream or the
     *             socket is in an invalid state.
     */
    public InputStream getInputStream() throws IOException {
        checkOpenAndCreate(false);
        if (isInputShutdown()) {
            throw new SocketException("Socket input is shutdown");
        }
        return impl.getInputStream();
    }

    /**
     * Returns this socket's {@link SocketOptions#SO_KEEPALIVE} setting.
     */
    public boolean getKeepAlive() throws SocketException {
        checkOpenAndCreate(true);
        return (Boolean) impl.getOption(SocketOptions.SO_KEEPALIVE);
    }

    /**
     * Returns the local IP address this socket is bound to, or {@code InetAddress.ANY} if
     * the socket is unbound.
     */
    public InetAddress getLocalAddress() {
        return localAddress;
    }

    /**
     * Returns the local port this socket is bound to, or -1 if the socket is unbound.
     */
    public int getLocalPort() {
        if (!isBound()) {
            return -1;
        }
        return impl.getLocalPort();
    }

    /**
     * Returns an output stream to write data into this socket.
     *
     * @return the byte-oriented output stream.
     * @throws IOException
     *             if an error occurs while creating the output stream or the
     *             socket is in an invalid state.
     */
    public OutputStream getOutputStream() throws IOException {
        checkOpenAndCreate(false);
        if (isOutputShutdown()) {
            throw new SocketException("Socket output is shutdown");
        }
        return this.impl.getOutputStream();
    }

    /**
     * Returns the port number of the target host this socket is connected to, or 0 if this socket
     * is not yet connected.
     */
    public int getPort() {
        if (!isConnected()) {
            return 0;
        }
        return impl.getPort();
    }

    /**
     * Returns this socket's {@link SocketOptions#SO_LINGER linger} timeout in seconds, or -1
     * for no linger (i.e. {@code close} will return immediately).
     */
    public int getSoLinger() throws SocketException {
        checkOpenAndCreate(true);
        // The RI explicitly guarantees this idiocy in the SocketOptions.setOption documentation.
        Object value = impl.getOption(SocketOptions.SO_LINGER);
        if (value instanceof Integer) {
            return (Integer) value;
        } else {
            return -1;
        }
    }

    /**
     * Returns this socket's {@link SocketOptions#SO_RCVBUF receive buffer size}.
     */
    public synchronized int getReceiveBufferSize() throws SocketException {
        checkOpenAndCreate(true);
        return (Integer) impl.getOption(SocketOptions.SO_RCVBUF);
    }

    /**
     * Returns this socket's {@link SocketOptions#SO_SNDBUF send buffer size}.
     */
    public synchronized int getSendBufferSize() throws SocketException {
        checkOpenAndCreate(true);
        return (Integer) impl.getOption(SocketOptions.SO_SNDBUF);
    }

    /**
     * Returns this socket's {@link SocketOptions#SO_TIMEOUT receive timeout}.
     */
    public synchronized int getSoTimeout() throws SocketException {
        checkOpenAndCreate(true);
        return (Integer) impl.getOption(SocketOptions.SO_TIMEOUT);
    }

    /**
     * Returns this socket's {@code SocketOptions#TCP_NODELAY} setting.
     */
    public boolean getTcpNoDelay() throws SocketException {
        checkOpenAndCreate(true);
        return (Boolean) impl.getOption(SocketOptions.TCP_NODELAY);
    }

    /**
     * Sets this socket's {@link SocketOptions#SO_KEEPALIVE} option.
     */
    public void setKeepAlive(boolean keepAlive) throws SocketException {
    	int kalive = keepAlive ? 1:0;
        if (impl != null) {
            checkOpenAndCreate(true);
            impl.setOption(SocketOptions.SO_KEEPALIVE, kalive);
        }
    }


    /**
     * Sets this socket's {@link SocketOptions#SO_SNDBUF send buffer size}.
     */
    public synchronized void setSendBufferSize(int size) throws SocketException {
        checkOpenAndCreate(true);
        if (size < 1) {
            throw new IllegalArgumentException("size < 1");
        }
        impl.setOption(SocketOptions.SO_SNDBUF, size);
    }

    /**
     * Sets this socket's {@link SocketOptions#SO_SNDBUF receive buffer size}.
     */
    public synchronized void setReceiveBufferSize(int size) throws SocketException {
        checkOpenAndCreate(true);
        if (size < 1) {
            throw new IllegalArgumentException("size < 1");
        }
        impl.setOption(SocketOptions.SO_RCVBUF, size);
    }

    /**
     * Sets this socket's {@link SocketOptions#SO_LINGER linger} timeout in seconds.
     * If {@code on} is false, {@code timeout} is irrelevant.
     */
    public void setSoLinger(boolean on, int timeout) throws SocketException {
        checkOpenAndCreate(true);
        // The RI explicitly guarantees this idiocy in the SocketOptions.setOption documentation.
        if (on && timeout < 0) {
            throw new IllegalArgumentException("timeout < 0");
        }
        if (on) {
            impl.setOption(SocketOptions.SO_LINGER, timeout);
        } else {
            impl.setOption(SocketOptions.SO_LINGER, 0);
        }
    }

    /**
     * Sets this socket's {@link SocketOptions#SO_TIMEOUT read timeout} in milliseconds.
     * Use 0 for no timeout.
     * To take effect, this option must be set before the blocking method was called.
     */
    public synchronized void setSoTimeout(int timeout) throws SocketException {
        checkOpenAndCreate(true);
        if (timeout < 0) {
            throw new IllegalArgumentException("timeout < 0");
        }
        impl.setOption(SocketOptions.SO_TIMEOUT, timeout);
    }

    /**
     * Sets this socket's {@link SocketOptions#TCP_NODELAY} option.
     */
    public void setTcpNoDelay(boolean on) throws SocketException {
    	int ion = on ? 1:0;
        checkOpenAndCreate(true);
        impl.setOption(SocketOptions.TCP_NODELAY, ion);
    }

    /**
     * Creates a stream socket, binds it to the nominated local address/port,
     * then connects it to the nominated destination address/port.
     *
     * @param dstAddress
     *            the destination host address.
     * @param dstPort
     *            the port on the destination host.
     * @param localAddress
     *            the address on the local machine to bind.
     * @param localPort
     *            the port on the local machine to bind.
     * @throws IOException
     *             thrown if an error occurs during the bind or connect
     *             operations.
     */
    private void startupSocket(InetAddress dstAddress, int dstPort,
            InetAddress localAddress, int localPort)
            throws IOException {

        if (localPort < 0 || localPort > 65535) {
            throw new IllegalArgumentException("Local port out of range: " + localPort);
        }

        InetAddress addr = localAddress == null ? Inet4Address.ANY : localAddress;
        synchronized (this) 
        {
            impl.create();
            isCreated = true;
            try {
                isBound = true;
                impl.connect(dstAddress, dstPort);
                isConnected = true;
            } catch (IOException e) {
                impl.close();
                throw e;
            }
        }
    }



    /**
     * Returns a {@code String} containing a concise, human-readable description of the
     * socket.
     *
     * @return the textual representation of this socket.
     */
    public String toString() {
        if (!isConnected()) {
            return "Socket[unconnected]";
        }
        return impl.toString();
    }

    /**
     * Closes the input stream of this socket. Any further data sent to this
     * socket will be discarded. Reading from this socket after this method has
     * been called will return the value {@code EOF}.
     *
     * @throws IOException
     *             if an error occurs while closing the socket input stream.
     * @throws SocketException
     *             if the input stream is already closed.
     */
    public void shutdownInput() throws IOException {
        if (isInputShutdown()) {
            throw new SocketException("Socket input is shutdown");
        }
        checkOpenAndCreate(false);
        impl.shutdownInput();
        isInputShutdown = true;
    }

    /**
     * Closes the output stream of this socket. All buffered data will be sent
     * followed by the termination sequence. Writing to the closed output stream
     * will cause an {@code IOException}.
     *
     * @throws IOException
     *             if an error occurs while closing the socket output stream.
     * @throws SocketException
     *             if the output stream is already closed.
     */
    public void shutdownOutput() throws IOException {
        if (isOutputShutdown()) {
            throw new SocketException("Socket output is shutdown");
        }
        checkOpenAndCreate(false);
        impl.shutdownOutput();
        isOutputShutdown = true;
    }

    /**
     * Checks whether the socket is closed, and throws an exception. Otherwise
     * creates the underlying SocketImpl.
     *
     * @throws SocketException
     *             if the socket is closed.
     */
    private void checkOpenAndCreate(boolean create) throws SocketException {
        if (isClosed()) {
            throw new SocketException("Socket is closed");
        }
        if (!create) {
            if (!isConnected()) {
                throw new SocketException("Socket is not connected");
                // a connected socket must be created
            }

            /*
             * return directly to fix a possible bug, if !create, should return
             * here
             */
            return;
        }
        if (isCreated) {
            return;
        }
        synchronized (this) {
            if (isCreated) {
                return;
            }
            try {
                impl.create();
            } catch (SocketException e) {
                throw e;
            } catch (IOException e) {
                throw new SocketException(e.toString());
            }
            isCreated = true;
        }
    }

    /**
     * Returns the local address and port of this socket as a SocketAddress or
     * null if the socket is unbound. This is useful on multihomed
     * hosts.
     */
    public SocketAddress getLocalSocketAddress() {
        if (!isBound()) {
            return null;
        }
        return new InetSocketAddress(getLocalAddress(), getLocalPort());
    }

    /**
     * Returns the remote address and port of this socket as a {@code
     * SocketAddress} or null if the socket is not connected.
     *
     * @return the remote socket address and port.
     */
    public SocketAddress getRemoteSocketAddress() {
        if (!isConnected()) {
            return null;
        }
        return new InetSocketAddress(getInetAddress(), getPort());
    }

    /**
     * Returns whether this socket is bound to a local address and port.
     *
     * @return {@code true} if the socket is bound to a local address, {@code
     *         false} otherwise.
     */
    public boolean isBound() {
        return isBound;
    }

    /**
     * Returns whether this socket is connected to a remote host.
     *
     * @return {@code true} if the socket is connected, {@code false} otherwise.
     */
    public boolean isConnected() {
        return isConnected;
    }

    /**
     * Returns whether this socket is closed.
     *
     * @return {@code true} if the socket is closed, {@code false} otherwise.
     */
    public boolean isClosed() {
        return isClosed;
    }

    /**
     * Binds this socket to the given local host address and port specified by
     * the SocketAddress {@code localAddr}. If {@code localAddr} is set to
     * {@code null}, this socket will be bound to an available local address on
     * any free port.
     *
     * @param localAddr
     *            the specific address and port on the local machine to bind to.
     * @throws IllegalArgumentException
     *             if the given SocketAddress is invalid or not supported.
     * @throws IOException
     *             if the socket is already bound or an error occurs while
     *             binding.
     */
    public void bind(SocketAddress localAddr) throws IOException {
        checkOpenAndCreate(true);
        if (isBound()) {
            throw new IOException("Socket is already bound");
        }

        int port = 0;
        InetAddress addr = Inet4Address.ANY;
        if (localAddr != null) {
            if (!(localAddr instanceof InetSocketAddress)) {
                throw new IllegalArgumentException("Local address not an InetSocketAddress: " +
                        localAddr.getClass());
            }
            InetSocketAddress inetAddr = (InetSocketAddress) localAddr;
            if ((addr = inetAddr.getAddress()) == null) {
                throw new UnknownHostException("Host is unresolved: " /*+ inetAddr.getHostName()*/);
            }
            port = inetAddr.getPort();
        }

        synchronized (this) {
            try {
                impl.bind(addr, port);
                isBound = true;
            } catch (IOException e) {
                impl.close();
                throw e;
            }
        }
    }

    /**
     * Connects this socket to the given remote host address and port specified
     * by the SocketAddress {@code remoteAddr}.
     *
     * @param remoteAddr
     *            the address and port of the remote host to connect to.
     * @throws IllegalArgumentException
     *             if the given SocketAddress is invalid or not supported.
     * @throws IOException
     *             if the socket is already connected or an error occurs while
     *             connecting.
     */
    public void connect(SocketAddress remoteAddr) throws IOException {
        connect(remoteAddr, 0);
    }

    /**
     * Connects this socket to the given remote host address and port specified
     * by the SocketAddress {@code remoteAddr} with the specified timeout. The
     * connecting method will block until the connection is established or an
     * error occurred.
     *
     * @param remoteAddr
     *            the address and port of the remote host to connect to.
     * @param timeout
     *            the timeout value in milliseconds or {@code 0} for an infinite
     *            timeout.
     * @throws IllegalArgumentException
     *             if the given SocketAddress is invalid or not supported or the
     *             timeout value is negative.
     * @throws IOException
     *             if the socket is already connected or an error occurs while
     *             connecting.
     */
    public void connect(SocketAddress remoteAddr, int timeout) throws IOException {
        checkOpenAndCreate(true);
        if (timeout < 0) {
            throw new IllegalArgumentException("timeout < 0");
        }
        if (isConnected()) {
            throw new SocketException("Already connected");
        }
        if (remoteAddr == null) {
            throw new IllegalArgumentException("remoteAddr == null");
        }

        if (!(remoteAddr instanceof InetSocketAddress)) {
            throw new IllegalArgumentException("Remote address not an InetSocketAddress: " +
                    remoteAddr.getClass());
        }
        InetSocketAddress inetAddr = (InetSocketAddress) remoteAddr;
        InetAddress addr;
        if ((addr = inetAddr.getAddress()) == null) {
            throw new UnknownHostException("Host is unresolved: " /*+ inetAddr.getHostName()*/);
        }
        int port = inetAddr.getPort();

        checkDestination(addr, port);
        synchronized (connectLock) {
            try {
                if (!isBound()) {
                    // socket already created at this point by earlier call or
                    // checkOpenAndCreate this caused us to lose socket
                    // options on create
                    // impl.create(true);
                    //if (!usingSocks()) {
                        impl.bind(Inet4Address.ANY, 0);
                    //}
                    isBound = true;
                }
                impl.connect(remoteAddr, timeout);
                isConnected = true;
            } catch (IOException e) {
                impl.close();
                throw e;
            }
        }
    }

    /**
     * Returns whether the incoming channel of the socket has already been
     * closed.
     *
     * @return {@code true} if reading from this socket is not possible anymore,
     *         {@code false} otherwise.
     */
    public boolean isInputShutdown() {
        return isInputShutdown;
    }

    /**
     * Returns whether the outgoing channel of the socket has already been
     * closed.
     *
     * @return {@code true} if writing to this socket is not possible anymore,
     *         {@code false} otherwise.
     */
    public boolean isOutputShutdown() {
        return isOutputShutdown;
    }

    /**
     * Sets this socket's {@link SocketOptions#SO_REUSEADDR} option.
     */
    public void setReuseAddress(boolean reuse) throws SocketException {
    	int iReuse = reuse ? 1: 0;
        checkOpenAndCreate(true);
        impl.setOption(SocketOptions.SO_REUSEADDR, iReuse);
    }

    /**
     * Returns this socket's {@link SocketOptions#SO_REUSEADDR} setting.
     */
    public boolean getReuseAddress() throws SocketException {
        checkOpenAndCreate(true);
        return (Boolean) impl.getOption(SocketOptions.SO_REUSEADDR);
    }

    /**
     * Sets this socket's {@link SocketOptions#SO_OOBINLINE} option.
     */
    public void setOOBInline(boolean oobinline) throws SocketException {
    	int obline = oobinline ? 1: 0;
        checkOpenAndCreate(true);
        impl.setOption(SocketOptions.SO_OOBINLINE, obline);
    }

    /**
     * Returns this socket's {@link SocketOptions#SO_OOBINLINE} setting.
     */
    public boolean getOOBInline() throws SocketException {
        checkOpenAndCreate(true);
        return (Boolean) impl.getOption(SocketOptions.SO_OOBINLINE);
    }

    /**
     * Sets this socket's {@link SocketOptions#IP_TOS} value for every packet sent by this socket.
     */
    public void setTrafficClass(int value) throws SocketException {
        checkOpenAndCreate(true);
        if (value < 0 || value > 255) {
            throw new IllegalArgumentException();
        }
        impl.setOption(SocketOptions.IP_TOS, value);
    }

    /**
     * Returns this socket's {@see SocketOptions#IP_TOS} setting.
     */
    public int getTrafficClass() throws SocketException {
        checkOpenAndCreate(true);
        return (Integer) impl.getOption(SocketOptions.IP_TOS);
    }


    /**
     * Set the appropriate flags for a socket created by {@code
     * ServerSocket.accept()}.
     *
     * @see ServerSocket#implAccept
     */
    void accepted() {
        isCreated = isBound = isConnected = true;
    }


    /**
     * Sets performance preferences for connectionTime, latency and bandwidth.
     *
     * <p>This method does currently nothing.
     *
     * @param connectionTime
     *            the value representing the importance of a short connecting
     *            time.
     * @param latency
     *            the value representing the importance of low latency.
     * @param bandwidth
     *            the value representing the importance of high bandwidth.
     */
    public void setPerformancePreferences(int connectionTime, int latency, int bandwidth) {
        // Our socket implementation only provide one protocol: TCP/IP, so
        // we do nothing for this method
    }
}