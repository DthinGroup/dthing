package java.net.http;


import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.MalformedURLException;
import java.io.OutputStream;
import java.util.Date;




public class Test {
	
	public static void main(String args[])
	{
		URL url =null;
		try {
			//http://203.195.192.97/wechat/xDiary.apk
			url = new URL("http://203.195.192.97/wechat/test.html");
		} catch (MalformedURLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			System.out.print("fuck exception!");
			return;
		} 
		
		HttpURLConnection uConn;
		try {
			uConn = (HttpURLConnection) url.openConnection();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			System.out.print("fuck exception II!");
			return;
		}
		
		uConn.getDefaultPort();
		System.out.println("port:"+url.getPort());
		System.out.println("default port:"+url.getDefaultPort());
		System.out.println("file:"+url.getFile());
		System.out.println("protocol:"+url.getProtocol());
		System.out.println("Authority:"+url.getAuthority());
		System.out.println("getHost:"+url.getHost());
		System.out.println("getPath:"+url.getPath());
		System.out.println("getQuery:"+url.getQuery());
		System.out.println("getRef:"+url.getRef());
		System.out.println("getUserInfo:"+url.getUserInfo());
		System.out.println("hashCode:"+url.hashCode());
		System.out.println("toExternalForm:"+url.toExternalForm());		
		System.out.println("toString:"+url.toString());

		//uConn.setRequestProperty("Date", (new Date()).toString());
		//uConn.setRequestProperty("Connection", "close");
		uConn.setRequestProperty("Accept", "text/html");
		uConn.setRequestProperty("Content-Length", "10");				
		uConn.setRequestProperty("Connection", "keep-alive");		
		uConn.setDoOutput(true);
		
		try {
			
			OutputStream out = uConn.getOutputStream();
			
			//out.write("hello12345 world".getBytes());
			//out.close();
			
			
			InputStream in = uConn.getInputStream();
			
			try{
				System.out.println("To start download");
				new DThread(new DownThread(in,uConn)).start();
			}
			catch(Exception e){
				System.out.println("thread error!");
			}
			
			/*
			String ClientMessage = "";
	        String line;

	        BufferedReader bRead = new BufferedReader(new InputStreamReader(in));
	        while(null != (line = bRead.readLine()) && !line.isEmpty()){
	        	ClientMessage += line + "\r\n";
	        }
	        System.out.println("body:"+ClientMessage);
			bRead.close();
			
			int leng = uConn.getContentLength();

			 byte[] aread = new byte[leng];
			 int a =0;
		        int length =0;
		        if(0< (length = in.read(aread))){
		        	System.out.print("" + new String(aread));
		        }
		        System.out.println();
	        
			in.close();*/
			
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		/*
		try {
			//InputStream in = uConn.getInputStream();
			System.out.println("Response code:" + uConn.getResponseCode());
			System.out.println("Response msg:" + uConn.getResponseMessage());
			System.out.println("Response 5th:" + uConn.getContentLength());
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			System.out.print("fuck exception III!");
		}
		
		uConn.disconnect();
		try {
			uConn.getInputStream();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		 */		
	}
	
	static class DownThread implements Runnable{

		InputStream dIn;
		HttpURLConnection uConn;
		
		public DownThread(InputStream in,HttpURLConnection conn){
			dIn = in;
			uConn = conn;
		}
		//@Override
		public void run() {
			int byteread =0,bytesum=0;
			int leng = uConn.getContentLength();
			FileOutputStream fs = null;
			
			System.out.println("Start downlowd");
			try {
				fs = new FileOutputStream("D:\\down.bin",true);
				System.out.println("Start read");
				byte[] buffer = new byte[100];  
				
		           while ((byteread = dIn.read(buffer)) >0) {  
		               bytesum += byteread;  
		               System.out.println("Read count:" + bytesum);  
		               fs.write(buffer, 0, byteread);  
		               if(bytesum >= leng)
		            	   break;
		           }  		           
			} catch (Exception e) {
				// TODO Auto-generated catch block
				//e.printStackTrace();
				System.out.println("http download error!");
			}  	
			System.out.println("downlowd over");
		}		
	}
}
