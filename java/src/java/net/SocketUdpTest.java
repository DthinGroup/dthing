
package java.net;
/*
import java.net;
import os.thread.*;
*/
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class SocketUdpTest
{
    
    public static void main(String args[])
    {
    	byte[] arr = {0x31,0x32,0x33,0x34,0x35,0x36};
    	
    	//int ret = 0;
    	//ret = NetNativeBridge.startUpNetwork(arr);
    	
    	int port = 6000;
    	byte[] ip = new byte[] {127,0,0,1};
    	InetAddress adr = new Inet4Address(ip);
    	DatagramSocket sock = null;
    	DatagramPacket dpack =null;
    	
    	
    	
    	InetSocketAddress isAddr = new InetSocketAddress(adr,port);
    	
    	try {
			sock = new DatagramSocket();
		} catch (SocketException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
    	
    	try {
			dpack = new DatagramPacket(arr,arr.length,isAddr);
		} catch (SocketException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		try {
			sock.send(dpack);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		/*
		byte[] recvb = new byte[200];
		try {
			dpack = new DatagramPacket(recvb,recvb.length,isAddr);
		} catch (SocketException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		
		try {
			sock.receive(dpack);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		*/
		//byte[] recd = dpack.getData();
		
		//System.out.println("Udp socket recv:" + new String(recd));
    }
}