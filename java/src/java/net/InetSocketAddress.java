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

import com.yarlungsoft.util.Log;

/**
 * This class represents a socket endpoint described by a IP address and a port number. It is a
 * concrete implementation of {@code SocketAddress} for IP.
 */
public class InetSocketAddress extends SocketAddress {

    private static final long serialVersionUID = 5076001401234631237L;

    private static final String TAG = "InetSocketAddress";

    // Exactly one of hostip or addr should be set.
    private final InetAddress addr;
    private final String hostip;
    private final int port;

    /**
     * @hide internal use only
     */
    public InetSocketAddress() {
        // These will be filled in the native implementation of recvfrom.
        this.addr = null;
        this.hostip = null;
        this.port = -1;
    }

    /**
     * Creates a socket endpoint with the given port number {@code port} and no specified address.
     * The range for valid port numbers is between 0 and 65535 inclusive.
     *
     * @param port the specified port number to which this socket is bound.
     */
    public InetSocketAddress(int port) {
        this((InetAddress) null, port);
    }

    /**
     * Creates a socket endpoint with the given port number {@code port} and {@code address}. The
     * range for valid port numbers is between 0 and 65535 inclusive. If {@code address} is
     * {@code null} this socket is bound to the IPv4 wildcard address.
     *
     * @param port the specified port number to which this socket is bound.
     * @param address the specified address to which this socket is bound.
     */
    public InetSocketAddress(InetAddress address, int port) {
        if (port < 0 || port > 65535) {
            throw new IllegalArgumentException("port=" + port);
        }
        this.addr = (address == null) ? Inet4Address.ANY : address;
        this.hostip = null;
        this.port = port;
    }

    private static byte[] getIP(String addr) throws IllegalArgumentException {
        byte[] ip = new byte[4];
        int i = addr.indexOf('.');
        ip[0] = (byte) Integer.parseInt(addr.substring(0, i++));
        int j = addr.indexOf('.', i);
        ip[1] = (byte) Integer.parseInt(addr.substring(i, j++));
        i = addr.indexOf('.', j);
        ip[2] = (byte) Integer.parseInt(addr.substring(j, i++));
        ip[3] = (byte) Integer.parseInt(addr.substring(i));

        Log.netLog(TAG, "ip=" + ip[0] + "." + ip[1] + "." + ip[2] + "." + ip[3]);
        return ip;
    }

    /**
     * Creates a socket endpoint with the given port number {@code port} and the hostname
     * {@code host}. The hostname is tried to be resolved and cannot be {@code null}. The range for
     * valid port numbers is between 0 and 65535 inclusive.
     *
     * @param port the specified port number to which this socket is bound.
     * @param host the specified hostname to which this socket is bound.
     */
    public InetSocketAddress(String host, int port) {
        Log.netLog(TAG, "constructor host=" + host + ", port=" + port);
        // host format is ip "xx.x.x.x"
        if (host == null || port < 0 || port > 65535) {
            throw new IllegalArgumentException("host=" + host + ", port=" + port);
        }

        byte[] ip = getIP(host);
        this.addr = new Inet4Address(ip);
        this.hostip = host;
        this.port = port;
        Log.netLog(TAG, "constructor over");
    }

    /**
     * Gets the port number of this socket.
     *
     * @return the socket endpoint port number.
     */
    public final int getPort() {
        return port;
    }

    /**
     * Gets the address of this socket.
     *
     * @return the socket endpoint address.
     */
    public final InetAddress getAddress() {
        return addr;
    }

    /**
     * Returns whether this socket address is unresolved or not.
     *
     * @return {@code true} if this socket address is unresolved, {@code false} otherwise.
     */
    public final boolean isUnresolved() {
        return addr == null;
    }

    /**
     * Gets a string representation of this socket included the address and the port number.
     *
     * @return the address and port number as a textual representation.
     */
    // @Override
    public String toString() {
        return ((addr != null) ? addr.toString() : hostip) + ":" + port;
    }

    /**
     * Compares two socket endpoints and returns true if they are equal. Two socket endpoints are
     * equal if the IP address or the hostname of both are equal and they are bound to the same
     * port.
     *
     * @param socketAddr the object to be tested for equality.
     * @return {@code true} if this socket and the given socket object {@code socketAddr} are equal,
     *         {@code false} otherwise.
     */
    // @Override
    public final boolean equals(Object socketAddr) {
        if (this == socketAddr) {
            return true;
        }
        if (!(socketAddr instanceof InetSocketAddress)) {
            return false;
        }
        InetSocketAddress iSockAddr = (InetSocketAddress) socketAddr;

        // check the ports as we always need to do this
        if (port != iSockAddr.port) {
            return false;
        }

        // we only use the hostip in the comparison if the addrs were not resolved
        if ((addr == null) && (iSockAddr.addr == null)) {
            return hostip.equals(iSockAddr.hostip);
        }

        // addrs were resolved so use them for the comparison
        if (addr == null) {
            // if we are here we know iSockAddr is not null so just return false
            return false;
        }
        return addr.equals(iSockAddr.addr);
    }

    // @Override
    public final int hashCode() {
        if (addr == null) {
            return hostip.hashCode() + port;
        }
        return addr.hashCode() + port;
    }
}
