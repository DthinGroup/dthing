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

import java.io.Serializable;
import java.util.Arrays;
import java.util.HashMap;

import com.yarlungsoft.util.TextUtils;

/**
 * An Internet Protocol (IP) address. This can be either an IPv4 address or an IPv6 address, and
 * in practice you'll have an instance of either {@code Inet4Address} or {@code Inet6Address} (this
 * class cannot be instantiated directly). Most code does not need to distinguish between the two
 * families, and should use {@code InetAddress}.
 * <p>
 * An {@code InetAddress} may have a hostname (accessible via {@code getHostName}), but may not,
 * depending on how the {@code InetAddress} was created.
 * <h4>IPv4 numeric address formats</h4>
 * <p>
 * The {@code getAllByName} method accepts IPv4 addresses in the "decimal-dotted-quad" form only:
 * <ul>
 * <li>{@code "1.2.3.4"} - 1.2.3.4
 * </ul>
 * <h4>IPv6 numeric address formats</h4>
 * <p>
 * The {@code getAllByName} method accepts IPv6 addresses in the following forms (this text comes
 * from <a href="http://www.ietf.org/rfc/rfc2373.txt">RFC 2373</a>, which you should consult for
 * full details of IPv6 addressing):
 * <ul>
 * <li>
 * <p>
 * The preferred form is {@code x:x:x:x:x:x:x:x}, where the 'x's are the hexadecimal values of the
 * eight 16-bit pieces of the address. Note that it is not necessary to write the leading zeros in
 * an individual field, but there must be at least one numeral in every field (except for the case
 * described in the next bullet). Examples:
 *
 * <pre>
 *     FEDC:BA98:7654:3210:FEDC:BA98:7654:3210
 *     1080:0:0:0:8:800:200C:417A
 * </pre>
 *
 * </li>
 * <li>Due to some methods of allocating certain styles of IPv6 addresses, it will be common for
 * addresses to contain long strings of zero bits. In order to make writing addresses containing
 * zero bits easier a special syntax is available to compress the zeros. The use of "::" indicates
 * multiple groups of 16-bits of zeros. The "::" can only appear once in an address. The "::" can
 * also be used to compress the leading and/or trailing zeros in an address. For example the
 * following addresses:
 *
 * <pre>
 *     1080:0:0:0:8:800:200C:417A  a unicast address
 *     FF01:0:0:0:0:0:0:101        a multicast address
 *     0:0:0:0:0:0:0:1             the loopback address
 *     0:0:0:0:0:0:0:0             the unspecified addresses
 * </pre>
 *
 * may be represented as:
 *
 * <pre>
 *     1080::8:800:200C:417A       a unicast address
 *     FF01::101                   a multicast address
 *     ::1                         the loopback address
 *     ::                          the unspecified addresses
 * </pre>
 *
 * </li>
 * <li>
 * <p>
 * An alternative form that is sometimes more convenient when dealing with a mixed environment of
 * IPv4 and IPv6 nodes is {@code x:x:x:x:x:x:d.d.d.d}, where the 'x's are the hexadecimal values of
 * the six high-order 16-bit pieces of the address, and the 'd's are the decimal values of the four
 * low-order 8-bit pieces of the address (standard IPv4 representation). Examples:
 *
 * <pre>
 *     0:0:0:0:0:0:13.1.68.3
 *     0:0:0:0:0:FFFF:129.144.52.38
 * </pre>
 *
 * or in compressed form:
 *
 * <pre>
 *     ::13.1.68.3
 *     ::FFFF:129.144.52.38
 * </pre>
 *
 * </li>
 * </ul>
 * <p>
 * Scopes are given using a trailing {@code %} followed by the scope id, as in
 * {@code 1080::8:800:200C:417A%2} or {@code 1080::8:800:200C:417A%en0}. See <a
 * href="https://www.ietf.org/rfc/rfc4007.txt">RFC 4007</a> for more on IPv6's scoped address
 * architecture.
 * <p>
 * Additionally, for backwards compatibility, IPv6 addresses may be surrounded by square brackets.
 * <h4>DNS caching</h4>
 * <p>
 * On Android, addresses are cached for 600 seconds (10 minutes) by default. Failed lookups are
 * cached for 10 seconds. The underlying C library or OS may cache for longer, but you can control
 * the Java-level caching with the usual {@code "networkaddress.cache.ttl"} and
 * {@code "networkaddress.cache.negative.ttl"} system properties. These are parsed as integer
 * numbers of seconds, where the special value 0 means "don't cache" and -1 means "cache forever".
 * <p>
 * Note also that on Android &ndash; unlike the RI &ndash; the cache is not unbounded. The current
 * implementation caches around 512 entries, removed on a least-recently-used basis. (Obviously, you
 * should not rely on these details.)
 *
 * @see Inet4Address
 * @see Inet6Address
 */
public class InetAddress implements Serializable {

    private static final long serialVersionUID = 3286316764910316507L;

    /**
     * One of {@link NetConstants#AF_UNIX}, {@link NetConstants#AF_INET},
     * {@link NetConstants#AF_INET6}, {@link NetConstants#AF_UNSPEC}.
     */
    private int family;

    protected byte[] ipAddress;

    protected String hostName;

    private static final HashMap<String, InetAddress> addressCache = new HashMap<String, InetAddress>();

    /**
     * Used by the DatagramSocket.disconnect implementation.
     *
     * @hide internal use only
     */
    public static final InetAddress UNSPECIFIED = new InetAddress(NetConstants.AF_UNSPEC, null,
                                                                  null);

    /**
     * Constructs an {@code InetAddress}.
     * Note: this constructor is for subclasses only.
     */
    // always hostName is null
    InetAddress(int family, byte[] ipAddress, String hostName) {
        this.family = family;
        this.hostName = hostName;

        if (ipAddress != null
            && ((family == NetConstants.AF_INET && ipAddress.length == NetConstants.INADDR4SZ)
                    || (family == NetConstants.AF_INET6 && ipAddress.length == NetConstants.INADDR16SZ))) {
            this.ipAddress = Arrays.copyOf(ipAddress, ipAddress.length);
        }
    }

    /**
     * Compares this {@code InetAddress} instance against the specified address in {@code obj}. Two
     * addresses are equal if their address byte arrays have the same length and if the bytes in the
     * arrays are equal.
     *
     * @param obj the object to be tested for equality.
     * @return {@code true} if both objects are equal, {@code false} otherwise.
     */
    public boolean equals(Object obj) {
        if (obj == null) {
            return false;
        }
        if (obj.getClass() != this.getClass()) {
            return false;
        }

        if (family != ((InetAddress) obj).family) {
            return false;
        }

        if (!Arrays.equals(ipAddress, ((InetAddress) obj).ipAddress)) {
            return false;
        }

        return TextUtils.equals(hostName, ((InetAddress) obj).hostName);
    }

    /**
     * Returns the IP address represented by this {@code InetAddress} instance
     * as a byte array. The elements are in network order (the highest order
     * address byte is in the zeroth element).
     *
     * @return the address in form of a byte array.
     */
    public byte[] getAddress() {
        if (ipAddress == null) {
            return null;
        }
        return Arrays.copyOf(ipAddress, ipAddress.length);
    }

    public int getAddressToInt() {
        return new Integer(ipAddress);
    }

    public int getFamily() {
        return family;
    }

    public String getHostAddress() {
        if (ipAddress != null) {
            StringBuffer sb = new StringBuffer(40);
            int i, last;
            if (ipAddress.length == NetConstants.INADDR4SZ) {
                last = ipAddress.length - 1;
                for (i = 0; i < last; i++) {
                    sb.append(ipAddress[i] & 0xff);
                    sb.append('.');
                }
                sb.append(ipAddress[i] & 0xff);
                return sb.toString();
            }
            if (ipAddress.length == NetConstants.INADDR16SZ) {
                last = ipAddress.length / NetConstants.INT16SZ - 1;
                for (i = 0; i < last; i++) {
                    sb.append(Integer.toHexString(((ipAddress[i << 1] << 8) & 0xff00)
                                                  | (ipAddress[(i << 1) + 1] & 0xff)));
                    sb.append(':');
                }
                sb.append(Integer.toHexString(((ipAddress[i << 1] << 8) & 0xff00)
                                              | (ipAddress[(i << 1) + 1] & 0xff)));
                return sb.toString();
            }
        }
        return null;
    }

    /**
     * Returns an {@code InetAddress} for the local host if possible, or the
     * loopback address otherwise. This method works by getting the hostname,
     * performing a DNS lookup, and then taking the first returned address.
     * For devices with multiple network interfaces and/or multiple addresses
     * per interface, this does not necessarily return the {@code InetAddress} you want.
     * <p>
     * Multiple interface/address configurations were relatively rare when this API was designed,
     * but multiple interfaces are the default for modern mobile devices (with separate wifi and
     * radio interfaces), and the need to support both IPv4 and IPv6 has made multiple addresses
     * commonplace. New code should thus avoid this method except where it's basically being used to
     * get a loopback address or equivalent.
     * <p>
     * There are two main ways to get a more specific answer:
     * <ul>
     * <li>If you have a connected socket, you should probably use {@link Socket#getLocalAddress}
     * instead: that will give you the address that's actually in use for that connection. (It's not
     * possible to ask the question "what local address would a connection to a given remote address
     * use?"; you have to actually make the connection and see.)</li>
     * <li>For other use cases, see {@link NetworkInterface}, which lets you enumerate all available
     * network interfaces and their addresses.</li>
     * </ul>
     * <p>
     * Note that if the host doesn't have a hostname set&nbsp;&ndash; as Android devices typically
     * don't&nbsp;&ndash; this method will effectively return the loopback address, albeit by
     * getting the name {@code localhost} and then doing a lookup to translate that to
     * {@code 127.0.0.1}.
     *
     * @return an {@code InetAddress} representing the local host, or the
     *         loopback address.
     * @throws UnknownHostException
     *         if the address lookup fails.
     */
    public static InetAddress getLocalHost() throws UnknownHostException {
        return getByName(NetConstants.LOCALHOST);
    }

    /**
     * Gets the hashcode of the represented IP address.
     *
     * @return the appropriate hashcode value.
     */
    public int hashCode() {
        return getAddressToInt();
    }

    /**
     * Removes all entries from the VM's DNS cache. This does not affect the C library's DNS
     * cache, nor any caching DNS servers between you and the canonical server.
     *
     * @hide
     */
    public static void clearDnsCache() {
    }

    /**
     * Returns a string containing a concise, human-readable description of this
     * IP address.
     *
     * @return the description, as host/address.
     */
    // @Override
    public String toString() {
        StringBuffer buf = new StringBuffer();
        buf.append("InetAddress host:");
        buf.append(hostName);

        buf.append(", ip:");
        buf.append(getHostAddress());

        buf.append(", family:");
        switch (family) {
        case NetConstants.AF_INET:
            buf.append("AF_INET");
            break;
        case NetConstants.AF_INET6:
            buf.append("AF_INET6");
            break;
        case NetConstants.AF_UNIX:
            buf.append("AF_UNIX");
            break;
        case NetConstants.AF_UNSPEC:
            buf.append("AF_UNSPEC");
            break;
        default:
            buf.append(family);
            break;
        }
        return buf.toString();
    }

    /**
     * Returns whether this is the IPv6 unspecified wildcard address {@code ::} or the IPv4 "any"
     * address, {@code 0.0.0.0}.
     */
    public boolean isAnyLocalAddress() {
        return getAddressToInt() == 0;
    }

    /**
     * Returns whether this address is a link-local address or not.
     * <p>
     * Valid IPv6 link-local addresses have the prefix {@code fe80::/10}.
     * <p>
     * <a href="http://www.ietf.org/rfc/rfc3484.txt">RFC 3484</a>
     * "Default Address Selection for Internet Protocol Version 6 (IPv6)" states that both IPv4
     * auto-configuration addresses (prefix {@code 169.254/16}) and IPv4 loopback addresses (prefix
     * {@code 127/8}) have link-local scope, but {@link Inet4Address} only considers the
     * auto-configuration addresses to have link-local scope. That is: the IPv4 loopback address
     * returns false.
     */
    public boolean isLinkLocalAddress() {
        return false;
    }

    /**
     * Returns whether this address is a loopback address or not.
     * <p>
     * Valid IPv4 loopback addresses have the prefix {@code 127/8}.
     * <p>
     * The only valid IPv6 loopback address is {@code ::1}.
     */
    public boolean isLoopbackAddress() {
        return false;
    }

    /**
     * Returns whether this address is a global multicast address or not.
     * <p>
     * Valid IPv6 global multicast addresses have the prefix {@code ffxe::/16}, where {@code x} is a
     * set of flags and the additional 112 bits make up the global multicast address space.
     * <p>
     * Valid IPv4 global multicast addresses are the range of addresses from {@code 224.0.1.0} to
     * {@code 238.255.255.255}.
     */
    public boolean isMCGlobal() {
        return false;
    }

    /**
     * Returns whether this address is a link-local multicast address or not.
     * <p>
     * Valid IPv6 link-local multicast addresses have the prefix {@code ffx2::/16}, where x is a set
     * of flags and the additional 112 bits make up the link-local multicast address space.
     * <p>
     * Valid IPv4 link-local multicast addresses have the prefix {@code 224.0.0/24}.
     */
    public boolean isMCLinkLocal() {
        return false;
    }

    /**
     * Returns whether this address is a node-local multicast address or not.
     * <p>
     * Valid IPv6 node-local multicast addresses have the prefix {@code ffx1::/16}, where x is a set
     * of flags and the additional 112 bits make up the link-local multicast address space.
     * <p>
     * There are no valid IPv4 node-local multicast addresses.
     */
    public boolean isMCNodeLocal() {
        return false;
    }

    /**
     * Returns whether this address is a organization-local multicast address or not.
     * <p>
     * Valid IPv6 organization-local multicast addresses have the prefix {@code ffx8::/16}, where x
     * is a set of flags and the additional 112 bits make up the link-local multicast address space.
     * <p>
     * Valid IPv4 organization-local multicast addresses have the prefix {@code 239.192/14}.
     */
    public boolean isMCOrgLocal() {
        return false;
    }

    /**
     * Returns whether this address is a site-local multicast address or not.
     * <p>
     * Valid IPv6 site-local multicast addresses have the prefix {@code ffx5::/16}, where x is a set
     * of flags and the additional 112 bits make up the link-local multicast address space.
     * <p>
     * Valid IPv4 site-local multicast addresses have the prefix {@code 239.255/16}.
     */
    public boolean isMCSiteLocal() {
        return false;
    }

    /**
     * Returns whether this address is a multicast address or not.
     * <p>
     * Valid IPv6 multicast addresses have the prefix {@code ff::/8}.
     * <p>
     * Valid IPv4 multicast addresses have the prefix {@code 224/4}.
     */
    public boolean isMulticastAddress() {
        return false;
    }

    /**
     * Returns whether this address is a site-local address or not.
     * <p>
     * For the purposes of this method, valid IPv6 site-local addresses have the deprecated prefix
     * {@code fec0::/10} from <a href="http://www.ietf.org/rfc/rfc1884.txt">RFC 1884</a>, <i>not</i>
     * the modern prefix {@code fc00::/7} from <a href="http://www.ietf.org/rfc/rfc4193.txt">RFC
     * 4193</a>.
     * <p>
     * <a href="http://www.ietf.org/rfc/rfc3484.txt">RFC 3484</a>
     * "Default Address Selection for Internet Protocol Version 6 (IPv6)" states that IPv4 private
     * addresses have the prefix {@code 10/8}, {@code 172.16/12}, or {@code 192.168/16}.
     *
     * @return {@code true} if this instance represents a site-local address, {@code false}
     *         otherwise.
     */
    public boolean isSiteLocalAddress() {
        return false;
    }

    /**
     * Returns an {@code InetAddress} object given the raw IP address.
     * The argument is in network byte order: the highest order byte of the address is in
     * {@code getAddress()[0]}.
     * <p>
     * This method doesn't block, i.e. no reverse name service lookup is performed.
     * <p>
     * IPv4 address byte array must be 4 bytes long and IPv6 byte array must be 16 bytes long
     *
     * @param addr the raw IP address in network byte order
     * @return an InetAddress object created from the raw IP address.
     * @exception UnknownHostException if IP address is of illegal length
     */
    public static InetAddress getByAddress(byte[] addr) throws UnknownHostException {
        if (addr == null) {
            throw new UnknownHostException("addr is null");
        }
        if (addr.length == NetConstants.INADDR4SZ) {
            return new Inet4Address(addr);
        }
        if (addr.length == NetConstants.INADDR16SZ) {
            return new InetAddress(NetConstants.AF_INET6, addr, null);
        }
        throw new UnknownHostException("addr is of illegal length");
    }

    /**
     * Determines the IP address of a host, given the host's name.
     * <p>
     * The host name can either be a machine name, such as "{@code java.sun.com}", or a textual
     * representation of its IP address. If a literal IP address is supplied, only the validity of
     * the address format is checked.
     * <p>
     * For {@code host} specified in literal IPv6 address, either the form defined in RFC 2732 or
     * the literal IPv6 address format defined in RFC 2373 is accepted. IPv6 scoped addresses are
     * also supported. See <a href="Inet6Address.html#scoped">here</a> for a description of IPv6
     * scoped addresses.
     * <p>
     * If the host is {@code null} then an {@code InetAddress} representing a IPv4 address of the
     * loopback interface is returned. See <a
     * href="http://www.ietf.org/rfc/rfc3330.txt">RFC&nbsp;3330</a> section&nbsp;2 and <a
     * href="http://www.ietf.org/rfc/rfc2373.txt">RFC&nbsp;2373</a> section&nbsp;2.5.3.
     *
     * @param host the specified host, or {@code null}.
     * @return an IP address for the given host name.
     * @exception UnknownHostException if no IP address for the {@code host} could be found, or if a
     *            scope_id was specified for a global IPv6 address.
     */
    public static InetAddress getByName(String host) throws UnknownHostException {
        if (TextUtils.isEmpty(host)) {
            return Inet4Address.LOOPBACK;
        }
        if(host.indexOf("..") != -1){
            throw new UnknownHostException(host + ": invalid address");
        }
        if(host.endsWith(".invalid")==true){
            throw new UnknownHostException(host + ": invalid address");
        }

        boolean ipv6Expected = false;
        if (host.charAt(0) == '[') {
            // This is supposed to be an IPv6 literal
            if (host.length() > 2 && host.charAt(host.length() - 1) == ']') {
                host = host.substring(1, host.length() - 1);
                ipv6Expected = true;
            } else {
                // This was supposed to be a IPv6 address, but it's not!
                throw new UnknownHostException(host + ": invalid IPv6 address");
            }
        }

        InetAddress ret;

        // if host is an IP address, we won't do further lookup
        if (Character.digit(host.charAt(0), 16) != -1 || (host.charAt(0) == ':')) {
            byte[] addr = null;
            String ifname = null;
            // see if it is IPv4 address

            addr = TextUtils.parseIpV4(host);
            if (addr == null) {
                // This is supposed to be an IPv6 literal
                // Check if a numeric or string zone id is present
                int pos;
                if ((pos = host.indexOf('%')) != -1) {
                    if (checkNumericZone(host) == -1) { /* remainder of string must be an ifname */
                        ifname = host.substring(pos + 1);
                    }
                }
                if ((addr = TextUtils.parseIpV6(host)) == null && host.indexOf(':') != -1) {
                    throw new UnknownHostException(host + ": invalid IPv6 address");
                }
            } else if (ipv6Expected) {
                // Means an IPv4 literal between brackets!
                throw new UnknownHostException("[" + host + "]");
            }
            if (addr != null) {
                if (addr.length == NetConstants.INADDR4SZ) {
                    ret = new Inet4Address(addr);
                } else {
                    ret = new InetAddress(NetConstants.AF_INET6, addr, ifname);
                }

                cacheAddress(host, ret);
                return ret;
            }
        } else if (ipv6Expected) {
            // We were expecting an IPv6 Litteral, but got something else
            throw new UnknownHostException("[" + host + "]");
        }

        // Lookup hostname in cache
        ret = getCachedAddress(host);
        if (ret != null) {
            return ret;
        }

        if (host.equalsIgnoreCase(NetConstants.LOCALHOST)) {
            ret = Inet4Address.LOOPBACK;
            cacheAddress(host, ret);
            return ret;
        }

        byte[] addr = NetNativeBridge.getHostByName(host);
        ret = getByAddress(addr);
        ret.hostName = host;
        cacheAddress(host, ret);
        return ret;
    }

    private static void cacheAddress(String host, InetAddress addr) {
        synchronized (addressCache) {
            addressCache.put(host.toLowerCase(), addr);
        }
    }

    private static InetAddress getCachedAddress(String host) {
        synchronized (addressCache) {
            return addressCache.get(host.toLowerCase());
        }
    }

    /**
     * check if the literal address string has %nn appended returns -1 if not, or the numeric value
     * otherwise.
     * <p>
     * %nn may also be a string that represents the displayName of a currently available
     * NetworkInterface.
     */
    private static int checkNumericZone(String s) {
        int percent = s.indexOf('%');
        int slen = s.length();
        int digit, zone = 0;
        if (percent == -1) {
            return -1;
        }
        for (int i = percent + 1; i < slen; i++) {
            char c = s.charAt(i);
            if (c == ']') {
                if (i == percent + 1) {
                    /* empty per-cent field */
                    return -1;
                }
                break;
            }
            if ((digit = Character.digit(c, 10)) < 0) {
                return -1;
            }
            zone = (zone * 10) + digit;
        }
        return zone;
    }

    /**
     * Returns an {@code InetAddress} corresponding to the given network-order bytes
     * {@code ipAddress} and {@code scopeId}.
     * <p>
     * For an IPv4 address, the byte array must be of length 4. For IPv6, the byte array must be of
     * length 16. Any other length will cause an {@code UnknownHostException}.
     * <p>
     * No reverse lookup is performed. The given {@code hostName} (which may be null) is associated
     * with the new {@code InetAddress} with no validation done.
     * <p>
     * (Note that numeric addresses such as {@code "127.0.0.1"} are names for the purposes of this
     * API. Most callers probably want {@link #getAllByName} instead.)
     *
     * @throws UnknownHostException if {@code ipAddress} is null or the wrong length.
     */
    public static InetAddress getByAddress(String hostName, byte[] ipAddress)
            throws UnknownHostException {
        return getByAddress(hostName, ipAddress, 0);
    }

    private static InetAddress getByAddress(String hostName, byte[] ipAddress, int scopeId)
            throws UnknownHostException {
        if (ipAddress == null) {
            throw new UnknownHostException("ipAddress == null");
        }
        if (ipAddress.length == 4) {
            return new Inet4Address(ipAddress/* .clone() */);
        }
        throw badAddressLength(ipAddress);
    }

    private static UnknownHostException badAddressLength(byte[] bytes) throws UnknownHostException {
        throw new UnknownHostException("Address is neither 4 or 16 bytes: " + new String(bytes));
    }
}
