package java.net.ota;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.MalformedURLException;
import java.net.http.HttpURLConnection;
import java.net.http.URL;

import com.yarlungsoft.ams.AmsConfig;

public class OTADownload{
	private String OTALink;
	public OTADownload(String url)
	{
		OTALink = url;
	}
	
	public void OTAStart()
	{
		try{
			new Thread(new OTAThread()).start();
		}catch(Exception e){
			notifyOTAResult(OTAConfig.OTA_TASK_FAIL);
		}
	}
	
	private native String getAppInstalledDir();
	
	private native void notifyOTAResult0(int result);
	
	private String getAppDir()
	{
		return getAppInstalledDir();//"D:\\nix.long\\ReDvmAll\\dvm\\appdb\\";
	}
	
	public void notifyOTAResult(int result)
	{
		switch(result){
		case OTAConfig.OTA_SUCCESS:
			break;
		case OTAConfig.OTA_FILE_ERROR:
			break;
		case OTAConfig.OTA_INVALID_URL:
			break;
		case OTAConfig.OTA_IO_ERROR:
			break;
		case OTAConfig.OTA_NET_ERROR:
			break;
		case OTAConfig.OTA_TASK_FAIL:
			break;
		}
		Log("OTA Result:" + result);
		notifyOTAResult0(result);
	}
	
	private static void Log(String log)
	{
		if(AmsConfig.debug()){
			System.out.println(log);
		}
	}
	
	class OTAThread implements Runnable{

		//@Override
		public void run() {
			URL url;
			try {
				url = new URL(OTALink);
			} catch (MalformedURLException e) {
				e.printStackTrace();
				Log("Exception: new URL("+OTALink+")");
				notifyOTAResult(OTAConfig.OTA_INVALID_URL);
				return;
			} 
			
			HttpURLConnection uConn =null;
			try {
				uConn = (HttpURLConnection) url.openConnection();
			} catch (Exception e) {
				e.printStackTrace();
				Log("Exception:open connection fail");
				notifyOTAResult(OTAConfig.OTA_NET_ERROR);
				return;
			}
			
			uConn.setRequestProperty("Accept", "text/html");
			uConn.setRequestProperty("Content-Length", "10");				
			uConn.setRequestProperty("Connection", "keep-alive");		
			uConn.setDoOutput(false);
			
			try {
				InputStream in = uConn.getInputStream();
				
				int byteread =0,bytesum=0;
				int leng = uConn.getContentLength();
				FileOutputStream fs = null;
				
				Log("OTA: Start downlowd");	
				
				String file = getAppDir()+ OTALink.substring(OTALink.lastIndexOf('/')+1);
				Log("OTA file name:"+ file);	
				
				fs = new FileOutputStream(file,true);
				System.out.println("Start read");
				byte[] buffer = new byte[100];  					
				while ((byteread = in.read(buffer)) >0) 
				{  
					bytesum += byteread;  
					System.out.println("Read count:" + bytesum);  
					fs.write(buffer, 0, byteread);  
					if(bytesum >= leng)
						break;
				}  		   
				fs.flush();
				fs.close();
			}catch(FileNotFoundException e){
				notifyOTAResult(OTAConfig.OTA_FILE_ERROR);
				return;
			}catch (IOException e1) {
				notifyOTAResult(OTAConfig.OTA_IO_ERROR);
				return;
			}
			Log("OTA downlowd over");
			notifyOTAResult(OTAConfig.OTA_SUCCESS);
		}
		
	}

}
