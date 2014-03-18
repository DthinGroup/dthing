/*
 * Record:
 * 1.delete API: @Override public boolean isMCGlobal()
 */
 
package java.net;

//import java.io.ObjectStreamException;
//import static libcore.io.OsConstants.*;

/**
 * An IPv4 address. See {@link InetAddress}.
 */
public final class Inet4Address extends InetAddress {

    private static final long serialVersionUID = 3286316764910316507L;

    /**
     * @hide
     */
    public static final InetAddress ANY =
            new Inet4Address(new byte[] { 0, 0, 0, 0 });

    /**
     * @hide
     */
    public static final InetAddress ALL =
            new Inet4Address(new byte[] { (byte) 255, (byte) 255,
                                          (byte) 255, (byte) 255 });

    /**
     * @hide
     */
    public static final InetAddress LOOPBACK =
            new Inet4Address(new byte[] { 127, 0, 0, 1 });

    Inet4Address(byte[] ipaddress) {
        super(NetConstants.AF_INET, ipaddress, null);
    }

    //@Override 
    public boolean isAnyLocalAddress() {
        return ipaddress[0] == 0 && ipaddress[1] == 0 && ipaddress[2] == 0 && ipaddress[3] == 0; // 0.0.0.0
    }

    //@Override 
    public boolean isLinkLocalAddress() {
        // The RI does not return true for loopback addresses even though RFC 3484 says to do so.
        return ((ipaddress[0] & 0xff) == 169) && ((ipaddress[1] & 0xff) == 254); // 169.254/16
    }

    //@Override 
    public boolean isLoopbackAddress() {
        return ((ipaddress[0] & 0xff) == 127); // 127/8
    }

    //@Override 
    public boolean isMCGlobal() 
    {    
        return false;
    }

    //@Override 
    public boolean isMCLinkLocal() {
        return ((ipaddress[0] & 0xff) == 224) && (ipaddress[1] == 0) && (ipaddress[2] == 0); // 224.0.0/24
    }

    //@Override 
    public boolean isMCNodeLocal() {
        return false;
    }

    //@Override 
    public boolean isMCOrgLocal() {
        return ((ipaddress[0] & 0xff) == 239) && ((ipaddress[1] & 0xfc) == 192); // 239.192/14
    }

    //@Override 
    public boolean isMCSiteLocal() {
        return ((ipaddress[0] & 0xff) == 239) && ((ipaddress[1] & 0xff) == 255); // 239.255/16
    }

    //@Override 
    public boolean isMulticastAddress() {
        return (ipaddress[0] & 0xf0) == 224; // 224/4
    }

    //@Override 
    public boolean isSiteLocalAddress() {
        if ((ipaddress[0] & 0xff) == 10) { // 10/8
            return true;
        } else if (((ipaddress[0] & 0xff) == 172) && ((ipaddress[1] & 0xf0) == 16)) { // 172.16/12
            return true;
        } else if (((ipaddress[0] & 0xff) == 192) && ((ipaddress[1] & 0xff) == 168)) { // 192.168/16
            return true;
        }
        return false;
    }
/*
    private Object writeReplace() throws ObjectStreamException {
        return new Inet4Address(ipaddress);
    }
    */
}
