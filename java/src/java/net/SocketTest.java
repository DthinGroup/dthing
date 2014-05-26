
package java.net;
/*
import java.net;
import os.thread.*;
*/
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class SocketTest
{
    
    public static void main(String args[])
    {
    	//byte[] arr = {1,2,3,4,5,6};
    	
    	//int ret = 0;
    	//ret = NetNativeBridge.startUpNetwork(arr);
    	
    	int port = 6000;
    	byte[] ip = new byte[] {127,0,0,1};
    	InetAddress adr = new Inet4Address(ip);
    	Socket s = null;
    	
    	InetSocketAddress isAddr = new InetSocketAddress(adr,port);
    	
    	InputStream in =null;
    	OutputStream out =null;
    	/*
    	try
    	{
        	s = new Socket(adr,port);
        }
        catch(IOException e)
        {
        	e.printStackTrace();
        	System.out.println("new socket fail!!");
        }
        */
        s = new Socket();
        try {
			s.connect(isAddr,0);
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		
        
        try {
			in = s.getInputStream();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
        byte[] buff = new byte[20];
		try {
			in.read(buff, 0, 20);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		System.out.println("recv:" + new String(buff));
		/*
		try {
			out = s.getOutputStream();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		
		if(out ==null)
		{
			byte[] arr = {1,2,3,4,5,6};
	    	
	    	int ret = 0;
	    	ret = NetNativeBridge.startUpNetwork(arr);
			System.out.println("out is null");
		}
		else
			
		{
			byte[] arr = {10,12,13,14,15,16};
			try {
				out.write("hello".getBytes(), 0, 5);
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
		*/
    }
}