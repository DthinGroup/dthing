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
import java.net.*;

import com.yarlungsoft.util.EmptyArray;


/**
 * @hide used in java.nio.
 */
public class PlainDatagramSocketImpl extends DatagramSocketImpl {

    private volatile int isNativeConnected;

    /**
     * used to keep address to which the socket was connected to at the native
     * level
     */
    private InetAddress connectedAddress;

    private int connectedPort = -1;

    public PlainDatagramSocketImpl(int sock, int localPort) {
        this.sockHandle = sock;
        this.localPort  = localPort;
    }

    public PlainDatagramSocketImpl() {
        this.sockHandle = 0;
    }

    public void bind(int port, InetAddress address) throws SocketException {
        NetNativeBridge.bind(sockHandle, address, port);
        if (port != 0) {
            localPort = port;
        } else {
            localPort = NetNativeBridge.getSocketLocalPort(sockHandle);
        }
        try {
            setOption(SocketOptions.SO_BROADCAST, 1);
        } catch (IOException ignored) {
        }
    }

    public synchronized void close() {
        NetNativeBridge.closeSocket(sockHandle);
    }

    
    public void create() throws SocketException {
        this.sockHandle = NetNativeBridge.socket(false);
    }

    protected void finalize() throws Throwable {
        try {
            close();
        } finally {
            super.finalize();
        }
    }

    public Object getOption(int option) throws SocketException {
        return NetNativeBridge.getSocketOption(sockHandle, option);
    }

    protected int peek(InetAddress sender) throws IOException {
        // We don't actually want the data: we just want the DatagramPacket's filled-in address.
        DatagramPacket packet = new DatagramPacket(EmptyArray.BYTE, 0);
        int result = peekData(packet);
        // Note: evil side-effect on InetAddress! This method should have returned InetSocketAddress!
        sender.ipaddress = packet.getAddress().getAddress();
        return result;
    }

    private void doRecv(DatagramPacket pack, int flags) throws IOException {
        NetNativeBridge.recvfrom(sockHandle, pack.getData(), pack.getOffset(), pack.getLength(), flags, pack, isNativeConnected);
        if (isNativeConnected !=0) {
            updatePacketRecvAddress(pack);
        }
    }

    public void receive(DatagramPacket pack) throws IOException {
        doRecv(pack, 0);
    }

    public int peekData(DatagramPacket pack) throws IOException {
        doRecv(pack, NetConstants.MSG_PEEK);
        return pack.getPort();
    }

    public void send(DatagramPacket packet) throws IOException {
        int port = (isNativeConnected==1) ? 0 : packet.getPort();
        InetAddress address = (isNativeConnected==1) ? null : packet.getAddress();
        NetNativeBridge.sendto(sockHandle, packet.getData(), packet.getOffset(), packet.getLength(), 0, address, port);
    }

    public void setOption(int option, int value) throws SocketException {
        NetNativeBridge.setSocketOption(sockHandle, option, value);
    }

    public void connect(InetAddress inetAddr, int port) throws SocketException {
        try {
			NetNativeBridge.connect(sockHandle, inetAddr, port,0);
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		} // Throws on failure.
        try {
            connectedAddress = InetAddress.getByAddress(inetAddr.getAddress());
        } catch (UnknownHostException e) {
            // this is never expected to happen as we should not have gotten
            // here if the address is not resolvable
            throw new SocketException("Host is unresolved: " + inetAddr.getAddressToInt());
        }
        connectedPort = port;
        isNativeConnected = 1;
    }

    public void disconnect() {
        try {
            NetNativeBridge.connect(sockHandle, InetAddress.UNSPECIFIED, 0,0);
        } catch (IOException errnoException) {
        	errnoException.printStackTrace();
        }
        connectedPort = -1;
        connectedAddress = null;
        isNativeConnected = 0;
    }

    /**
     * Set the received address and port in the packet. We do this when the
     * Datagram socket is connected at the native level and the
     * recvConnnectedDatagramImpl does not update the packet with address from
     * which the packet was received
     *
     * @param packet
     *            the packet to be updated
     */
    private void updatePacketRecvAddress(DatagramPacket packet) {
        packet.setAddress(connectedAddress);
        packet.setPort(connectedPort);
    }
}
