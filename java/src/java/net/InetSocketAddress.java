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
import com.yarlungsoft.util.TextUtils;

/**
 * This class represents a socket endpoint described by a IP address and a port number. It is a
 * concrete implementation of {@link SocketAddress} for IP.
 * <p>
 * It can also be a pair (hostname + port number), in which case an attempt will be made to resolve
 * the hostname. If resolution fails then the address is said to be <I>unresolved</I>.
 * <p>
 * The <i>wildcard</i> is a special local IP address. It usually means "any" and can only be used
 * for {@code bind} operations.
 */
public class InetSocketAddress extends SocketAddress {

    private static final long serialVersionUID = 5076001401234631237L;

    private static final String TAG = "InetSocketAddress";

    private InetAddress addr;
    private String hostname;
    private int port = -1;

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
     * Creates a socket endpoint with the given {@code address} and port number {@code port}. The
     * range for valid port numbers is between 0 and 65535 inclusive. If {@code address} is
     * {@code null} this socket is bound to the IPv4 wildcard address.
     *
     * @param address the specified address to which this socket is bound.
     * @param port the specified port number to which this socket is bound.
     */
    public InetSocketAddress(InetAddress address, int port) {
        if (port < 0 || port > 65535) {
            throw new IllegalArgumentException("port=" + port);
        }
        addr = (address == null) ? Inet4Address.ANY : address;
        hostname = null;
        this.port = port;
    }

    /**
     * Creates a socket endpoint with the given port number {@code port} and the hostname
     * {@code host}. The hostname is tried to be resolved and cannot be {@code null} or empty. The
     * range for valid port numbers is between 0 and 65535 inclusive.
     *
     * @param host the specified hostname to which this socket is bound.
     * @param port the specified port number to which this socket is bound.
     */
    public InetSocketAddress(String host, int port) {
        Log.netLog(TAG, "constructor host=" + host + ", port=" + port);

        if (TextUtils.isEmpty(host) || port < 0 || port > 65535) {
            throw new IllegalArgumentException("host=" + host + ", port=" + port);
        }

        try {
            addr = InetAddress.getByName(host);
        } catch (UnknownHostException e) {
            Log.netLog(TAG, "failed to get addr from host:" + e);
        }
        hostname = host;
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
        return ((addr != null) ? addr.toString() : hostname) + ", port:" + port;
    }

    /**
     * Compares two socket endpoints and returns true if they are equal. Two socket endpoints are
     * equal if the IP address or the hostname of both are equal and they are bound to the same
     * port.
     *
     * @param obj the object to be tested for equality.
     * @return {@code true} if this socket and the given socket object {@code socketAddr} are equal,
     *         {@code false} otherwise.
     */
    // @Override
    public final boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (obj == null || !(obj instanceof InetSocketAddress)) {
            return false;
        }

        InetSocketAddress sockAddr = (InetSocketAddress) obj;

        // check the ports as we always need to do this
        if (port != sockAddr.port) {
            return false;
        }

        if (addr != sockAddr.addr) {
            return false;
        }

        return TextUtils.equals(hostname, sockAddr.hostname);
    }

    // @Override
    public final int hashCode() {
        if (addr != null) {
            return addr.hashCode() + port;
        }
        if (hostname != null) {
            return hostname.hashCode() + port;
        }
        return port;
    }
}
