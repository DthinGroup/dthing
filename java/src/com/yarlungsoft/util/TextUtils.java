
package com.yarlungsoft.util;

import java.net.NetConstants;
import java.util.StringTokenizer;

/**
 * API for handling strings.
 */
public final class TextUtils {

    private TextUtils() {
        /* cannot be instantiated */
    }

    /**
     * Returns true if the string is null or 0-length.
     *
     * @param str the string to be examined
     * @return true if str is null or zero length
     */
    public static boolean isEmpty(String str) {
        return str == null || str.length() == 0;
    }

    public static boolean equals(String str1, String str2) {
        if (str1 == str2) {
            return true;
        }
        if (str1 == null || str2 == null || str1.length() != str2.length()) {
            return false;
        }

        return str1.equals(str2);
    }

    /**
     * Converts IPv4 address in its textual presentation form into its numeric binary form.
     *
     * @param src a String representing an IPv4 address in standard format
     * @return a byte array representing the IPv4 numeric address
     */
    public static byte[] parseIpV4(String host) {
        if (host.length() == 0) {
            return null;
        }

        byte[] res = new byte[NetConstants.INADDR4SZ];
        StringTokenizer st = new StringTokenizer(host, ".");
        long val;
        try {
            switch(st.countTokens()) {
            case 1:
                /*
                 * When only one part is given, the value is stored directly in
                 * the network address without any byte rearrangement.
                 */

                val = Long.parseLong(st.nextToken());
                if (val < 0 || val > 0xffffffffL) {
                    return null;
                }
                res[0] = (byte) ((val >> 24) & 0xff);
                res[1] = (byte) (((val & 0xffffff) >> 16) & 0xff);
                res[2] = (byte) (((val & 0xffff) >> 8) & 0xff);
                res[3] = (byte) (val & 0xff);
                break;

            case 2:
                /*
                 * When a two part address is supplied, the last part is
                 * interpreted as a 24-bit quantity and placed in the right
                 * most three bytes of the network address. This makes the
                 * two part address format convenient for specifying Class A
                 * network addresses as net.host.
                 */

                val = Integer.parseInt(st.nextToken());
                if (val < 0 || val > 0xff) {
                    return null;
                }
                res[0] = (byte) (val & 0xff);
                val = Integer.parseInt(st.nextToken());
                if (val < 0 || val > 0xffffff) {
                    return null;
                }
                res[1] = (byte) ((val >> 16) & 0xff);
                res[2] = (byte) (((val & 0xffff) >> 8) &0xff);
                res[3] = (byte) (val & 0xff);
                break;

            case 3:
                /*
                 * When a three part address is specified, the last part is
                 * interpreted as a 16-bit quantity and placed in the right
                 * most two bytes of the network address. This makes the
                 * three part address format convenient for specifying
                 * Class B net- work addresses as 128.net.host.
                 */
                for (int i = 0; i < 2; i++) {
                    val = Integer.parseInt(st.nextToken());
                    if (val < 0 || val > 0xff) {
                        return null;
                    }
                    res[i] = (byte) (val & 0xff);
                }
                val = Integer.parseInt(st.nextToken());
                if (val < 0 || val > 0xffff) {
                    return null;
                }
                res[2] = (byte) ((val >> 8) & 0xff);
                res[3] = (byte) (val & 0xff);
                break;

            case 4:
                /*
                 * When four parts are specified, each is interpreted as a
                 * byte of data and assigned, from left to right, to the
                 * four bytes of an IPv4 address.
                 */
                for (int i = 0; i < 4; i++) {
                    val = Integer.parseInt(st.nextToken());
                    if (val < 0 || val > 0xff) {
                        return null;
                    }
                    res[i] = (byte) (val & 0xff);
                }
                break;
            default:
                return null;
            }
        } catch(NumberFormatException e) {
            return null;
        }
        return res;
    }

    /**
     * Convert IPv6 presentation level address to network order binary form.
     * <p>
     * credit:<br>
     * Converted from C code from Solaris 8 (inet_pton)
     * <p>
     * Any component of the string following a per-cent % is ignored.
     *
     * @param src a String representing an IPv6 address in textual format
     * @return a byte array representing the IPv6 numeric address
     */
    public static byte[] parseIpV6(String host) {
        // Shortest valid string is "::", hence at least 2 chars
        if (host.length() < 2) {
            return null;
        }

        int colonp;
        char ch;
        boolean saw_xdigit;
        int val;
        char[] srcb = host.toCharArray();
        byte[] dst = new byte[NetConstants.INADDR16SZ];

        int srcb_length = srcb.length;
        int pc = host.indexOf ("%");
        if (pc == srcb_length -1) {
            return null;
        }

        if (pc != -1) {
            srcb_length = pc;
        }

        colonp = -1;
        int i = 0, j = 0;
        /* Leading :: requires some special handling. */
        if (srcb[i] == ':') {
            if (srcb[++i] != ':') {
                return null;
            }
        }

        int curtok = i;
        saw_xdigit = false;
        val = 0;
        while (i < srcb_length) {
            ch = srcb[i++];
            int chval = Character.digit(ch, 16);
            if (chval != -1) {
                val <<= 4;
                val |= chval;
                if (val > 0xffff) {
                    return null;
                }
                saw_xdigit = true;
                continue;
            }
            if (ch == ':') {
                curtok = i;
                if (!saw_xdigit) {
                    if (colonp != -1) {
                        return null;
                    }
                    colonp = j;
                    continue;
                } else if (i == srcb_length) {
                    return null;
                }
                if (j + NetConstants.INT16SZ > NetConstants.INADDR16SZ) {
                    return null;
                }
                dst[j++] = (byte) ((val >> 8) & 0xff);
                dst[j++] = (byte) (val & 0xff);
                saw_xdigit = false;
                val = 0;
                continue;
            }
            if (ch == '.' && ((j + NetConstants.INADDR4SZ) <= NetConstants.INADDR16SZ)) {
                String ia4 = host.substring(curtok, srcb_length);
                /* check this IPv4 address has 3 dots, ie. A.B.C.D */
                int dot_count = 0, index=0;
                while ((index = ia4.indexOf ('.', index)) != -1) {
                    dot_count ++;
                    index ++;
                }
                if (dot_count != 3) {
                    return null;
                }
                byte[] v4addr = parseIpV4(ia4);
                if (v4addr == null) {
                    return null;
                }
                for (int k = 0; k < NetConstants.INADDR4SZ; k++) {
                    dst[j++] = v4addr[k];
                }
                saw_xdigit = false;
                break;  /* '\0' was seen by inet_pton4(). */
            }
            return null;
        }
        if (saw_xdigit) {
            if (j + NetConstants.INT16SZ > NetConstants.INADDR16SZ) {
                return null;
            }
            dst[j++] = (byte) ((val >> 8) & 0xff);
            dst[j++] = (byte) (val & 0xff);
        }

        if (colonp != -1) {
            int n = j - colonp;

            if (j == NetConstants.INADDR16SZ) {
                return null;
            }
            for (i = 1; i <= n; i++) {
                dst[NetConstants.INADDR16SZ - i] = dst[colonp + n - i];
                dst[colonp + n - i] = 0;
            }
            j = NetConstants.INADDR16SZ;
        }
        if (j != NetConstants.INADDR16SZ) {
            return null;
        }
        byte[] newdst = convertFromIPv4MappedAddress(dst);
        if (newdst != null) {
            return newdst;
        } else {
            return dst;
        }
    }

    /**
     * Convert IPv4-Mapped address to IPv4 address. Both input and returned value are in network
     * order binary form.
     *
     * @param src a String representing an IPv4-Mapped address in textual format
     * @return a byte array representing the IPv4 numeric address
     */
    public static byte[] convertFromIPv4MappedAddress(byte[] addr) {
        if (isIPv4MappedAddress(addr)) {
            byte[] newAddr = new byte[NetConstants.INADDR4SZ];
            System.arraycopy(addr, 12, newAddr, 0, NetConstants.INADDR4SZ);
            return newAddr;
        }
        return null;
    }

    /**
     * Utility routine to check if the InetAddress is an IPv4 mapped IPv6 address.
     *
     * @return a <code>boolean</code> indicating if the InetAddress is
     *         an IPv4 mapped IPv6 address; or false if address is IPv4 address.
     */
    private static boolean isIPv4MappedAddress(byte[] addr) {
        if (addr.length < NetConstants.INADDR16SZ) {
            return false;
        }
        if ((addr[0] == 0x00) && (addr[1] == 0x00) &&
            (addr[2] == 0x00) && (addr[3] == 0x00) &&
            (addr[4] == 0x00) && (addr[5] == 0x00) &&
            (addr[6] == 0x00) && (addr[7] == 0x00) &&
            (addr[8] == 0x00) && (addr[9] == 0x00) &&
            (addr[10] == (byte)0xff) &&
            (addr[11] == (byte)0xff))  {
            return true;
        }
        return false;
    }
}
