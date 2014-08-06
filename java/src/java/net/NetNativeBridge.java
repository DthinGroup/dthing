
package java.net;


import java.io.IOException;

import com.yarlungsoft.util.Log;


/**
 * Utility to access native networks.
 */
public final class NetNativeBridge {

    private static final String TAG = "NetNativeBridge";

    private NetNativeBridge() {
    }

    /**
     * Check whether the network is initialized or not.
     *
     * @return {@code true} indicates the network is initialized, {@code false} indicates not
     * initialized.
     */
    private static native boolean isNetworkInited();

    /**
     * Use to activate network (PDP).
     *
     * @param test test data
     * @return {@link NetConstants.NET_OP_SUCCESS} indicates success, other value indicates failure.
     */
    public static native int startUpNetwork(byte[] test);

    private static native int socket0(boolean streaming);

    private static native int connect0(int sock, int ip, int port, int timeOuts);

    private static native int recv0(int sock, byte[] arr, int offset, int count);

    private static native int send0(int sock, byte[] bytes, int offset, int count);

    private static native int recvfrom0(int sock, byte[] buff, int offset, int count, int flags,
            int ip, int port, int connectted);

    private static native int sendto0(int sock, byte[] buff, int offset, int count, int flag,
            int ip, int port);

    private static native int closeSocket0(int sock);

    /**
     * Create socket.
     *
     * @param streaming {@code true} to create TCP socket, {@code false} to create UDP socket.
     * @return socket handle if success
     * @throws SocketException
     */
    public static int socket(boolean streaming) throws SocketException {
        int res = NetConstants.NET_OP_ERROR;
        byte[] arr = {1, 2, 3, 4, 5};
        Log.log(TAG, "create socket,stream:" + (streaming ? "tcp" : "udp"));
        if (!isNetworkInited()) {
            while (AsyncIO.loop()) {
                res = startUpNetwork(arr);
            }

            Log.log(TAG, "startUpNetwork :" + res);
            if (res != NetConstants.NET_OP_SUCCESS) {
                throw new SocketException("in socket cons,start up network fail!");
            }
        }

        int sock = socket0(streaming);
        if (sock == NetConstants.NET_OP_ERROR) {
            throw new SocketException("create socket fail!");
        }

        Log.log(TAG, "create sock handle:" + sock);
        return sock;
    }

    public static void closeSocket(int sockHandle) {
        closeSocket0(sockHandle);
    }

    public static int available(int sockHandle) {
        return 0;
    }

    public static void bind(int sockHandle, InetAddress address, int port) {
        int ip = address.getAddressToInt();
        int family = address.getFamily();

        // not need to support!!
    }

    public static void connect(int sockHandle, InetAddress address, int port, int timeoutMs)
            throws IOException {
        int ip = address.getAddressToInt();
        int family = address.getFamily();

        int ret = 0;
        while (AsyncIO.loop()) {
            ret = connect0(sockHandle, ip, port, timeoutMs);
        }
        if (ret != NetConstants.NET_OP_SUCCESS) {
            throw new IOException("connect to server:" + ip + ":" + port + " fail!");
        }

    }

    public static void setSocketOption(int sockHandle, int opt, int val) {
    }

    public static int getSocketOption(int sockHandle, int opt) {
        return 0;
    }

    public static int getSocketLocalPort(int sockHandle) {
        return 0;
    }

    public static int recv(int sockHandle, byte[] bytes, int offset, int count, int flags)
            throws IOException {
        int cnt = 0;

        if (offset + count > bytes.length) {
            throw new ArrayIndexOutOfBoundsException("array index out of bound!");
        }

        while (AsyncIO.loop()) {
            cnt = recv0(sockHandle, bytes, offset, count);
        }
        return cnt;
    }

    public static int recvfrom(int sockHandle, byte[] bytes, int offset, int count, int flags,
            DatagramPacket pack, int connected) throws IOException {
        int cnt = 0;
        int ip = 0;
        int port = 0;

        if (connected == 0 && pack == null) {
            throw new NullPointerException("datagram packet is null!");
        }

        if (pack != null) {
            ip = pack.getAddress().getAddressToInt();
            port = pack.getPort();
        }

        if (connected == 0) {
            if (ip == 0 || port == 0) {
                throw new SocketException("not specefied remote ip/port address");
            }
        }

        if (offset + count > bytes.length) {
            throw new ArrayIndexOutOfBoundsException("array index out of bound!");
        }

        while (AsyncIO.loop()) {
            cnt = recvfrom0(sockHandle, bytes, offset, count, flags, ip, port, connected);
        }

        if (cnt < 0) {
            throw new IOException("recvfrom exception occurs!,return code is " + cnt);
        }

        return cnt;
    }

    public static int send(int sockHandle, byte[] bytes, int offset, int count, int flags)
            throws IOException {
        int cnt = 0;

        if (offset + count > bytes.length) {
            throw new ArrayIndexOutOfBoundsException("array index out of bound!");
        }

        while (AsyncIO.loop()) {
            cnt = send0(sockHandle, bytes, offset, count);
        }
        return cnt;
    }

    public static int sendto(int sockHandle, byte[] bytes, int offset, int count, int flags,
            InetAddress address, int port) throws IOException {
        int cnt = 0;
        int ip = 0;

        if (address != null) {
            ip = address.getAddressToInt();
        }

        if (offset + count > bytes.length) {
            throw new ArrayIndexOutOfBoundsException("array index out of bound!");
        }

        while (AsyncIO.loop()) {
            cnt = sendto0(sockHandle, bytes, offset, count, flags, ip, port);
        }
        return cnt;
    }
}
