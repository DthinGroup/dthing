package jp.co.cmcc.wmmp;

import java.io.*;

/*import javax.microedition.io.Connector;
import javax.microedition.io.ContentConnection;
import javax.microedition.io.file.FileConnection;
*/


//import spec.javax.microedition.io.*;

public class WMMPFileDownloadTask extends Thread
{
    public final static int GET_TRANS_ID    = 0;
    public final static int GET_FILE_SIZE   = 1;
    public final static int GET_WMMP_VER    = 2;

    boolean beDownloadOk;

    WMMPConnection wmmpConn;
    WMMPListener   cmdListener;
    String fileName;
    String fileUrl;
    String filePath;

    private void checkParams() throws IllegalArgumentException 
    {
        //check url
        if(fileUrl == null || filePath ==null || wmmpConn==null || cmdListener==null)
        {
            throw new IllegalArgumentException();
        }
        
        if(fileUrl.length() == 0 || filePath.length() == 0)
        {
            throw new IllegalArgumentException();
        }

        if(fileUrl.startsWith("http://") ||
            fileUrl.startsWith("https://"))
        {
            int file_start =0;
            int file_end =0;
            file_start = fileUrl.lastIndexOf('/');
            if(file_start ==-1)
                throw new IllegalArgumentException("not contains file");
            file_end = fileUrl.indexOf("jar");
            if(file_end ==-1)
                throw new IllegalArgumentException("not contains file");
            file_start = file_start +1;
            file_end = file_end + 3;
            fileName = fileUrl.substring(file_start, file_end);

            if(fileName ==null)
                throw new IllegalArgumentException("not contains file");
        }
        else if(fileUrl.startsWith("wmmp://"))
        {
            //not support for now!!
        }
        else
        {
            throw new IllegalArgumentException("illegal prefix");
        }

        //file path check
        //1.check root disk,must have ':'
        if(filePath.indexOf(':') != 1 || filePath.indexOf('+') != -1 || filePath.indexOf('>') != -1 || 
           filePath.indexOf('<') != -1 ||  filePath.indexOf('|') !=-1 || filePath.indexOf('*') != -1 ||
           filePath.indexOf('-') != -1 || filePath.indexOf('%') != -1 || filePath.indexOf('&') != -1 ||
           filePath.indexOf('^') != -1 ) {
            throw new IllegalArgumentException("illegal file Path");
        }

        System.out.println("WMMPFileDownloadTask:URL  - " + fileUrl);
        System.out.println("WMMPFileDownloadTask:path - " + filePath);
        System.out.println("WMMPFileDownloadTask:name - " + fileName);
    }

    public WMMPFileDownloadTask(WMMPConnection conn,
                         WMMPListener listener,
                         String url,
                         String pathName)
                       throws IllegalArgumentException
    {
        wmmpConn = conn;
        cmdListener = listener;
        beDownloadOk = false;
        fileUrl = url;
        filePath = pathName;
        
        checkParams();
    }

    public void run()
    {
    		/*
        ContentConnection cConn = null;
        DataInputStream dis = null;
        DataOutputStream dos = null;
        FileConnection fconn =null;
        */
/*#if 0
        try
        {
            log("point 1");

            if(WMMPConnection.FAKE_DOWNLOAD)
            {
                //reason is the down url is 192.168.x.x .
                fileName = "Demo.jar"; //for test
                fileUrl = "http://42.121.18.62/Demo.jar";  //for test
            }

            String file_t = new String("file:///" + filePath + fileName);
            log("File Path:"+file_t);
            fconn = (FileConnection)javax.microedition.io.Connector.open(file_t);
            log("point 2");
            try
            {
                fconn.delete();
            }
            catch(Exception e)
            {
                log("file not exist!!!");
            }
                log("point 3.to create");
                fconn.create();

            log("point 3.1, create ok");

            if(fconn.exists())
            {
                log("file exist");
                log("test file size: " + fconn.fileSize());
            }
            else
            {
                log("file create fail!");
                return;
            }
            log("point 4");
            dos = fconn.openDataOutputStream();

            cConn = (ContentConnection)Connector.open(fileUrl);
            int len = (int)cConn.getLength();
            dis = cConn.openDataInputStream();
            log("FILE LEN:" +len);
            if (len > 0)
            {
                byte[] data = new byte[len];
                dis.readFully(data);
                dos.write(data);
            }
            else
            {
                int ch;
                while ((ch = dis.read()) != -1)
                {
                    dos.write(ch);
                }
            }
            long size = fconn.fileSize();
            log("File size is " + size + " Byte");
        }
        catch(Throwable e)
        {
            e.printStackTrace();
            log("Fuck download Exception");
            return;
        }

        try
        {
            if(dos !=null)
                dos.close();
            if(fconn !=null)
                fconn.close();
            if (dis != null)
                dis.close();
            if (cConn != null)
                cConn.close();
        }
        catch(Throwable e)
        {
            log("download fail");
            e.printStackTrace();
            wmmpConn.sendDownloadInfoAck(0x05);
            return;
        }
#endif*/

            log("download over");
            beDownloadOk = true;
            wmmpConn.sendDownloadInfoAck(0x01);
        String urla;
/*#if 0
        String prefix = "file:///";
        urla = prefix + filePath + fileName;
#else */
        if(WMMPConnection.FAKE_DOWNLOAD)
        {
            urla = "http://42.121.18.62/Demo.jar";
        }
        else
        {
            urla = fileUrl;
        }
//#endif
        WMMPDataItem[] ditem = new WMMPDataItem[1];
        ditem[0] = new WMMPDataItem(0x1001,urla.getBytes());
        log("File URL:" + urla);
        if(cmdListener != null)
            cmdListener.onDataReceived(WMMPConnection.URL_INFO,ditem);

    }

    public boolean isDownloadOk()
    {
        return beDownloadOk;
    }

    public static String getDownloadParam(String downloadUrl,int index)
    {
        String  left = null;
        String  name = null;
        String  value = null;
        int       idx = -1 ;
        if(downloadUrl == null || index < 0) {
            return null;
        }
        idx = downloadUrl.lastIndexOf('?');
        if(idx == -1) {
            return "";
        }
        left = downloadUrl.substring(idx+1);
        if(left.equals("")) {
            return "";
        }
        //TRANS_ID=0123&FILE_SIZE=128&VER_DSC=1.0
        if(left.indexOf("TRANS_ID") == -1 && left.indexOf("FILE_SIZE") == -1 && left.indexOf("VER_DSC") == -1) {
            return "";
        }
        left = left + "&";

        //http://somewhere.com/foo.txt?TRANS_ID=0123&param2=val2&param3=val3&
        int idx0 = 0;
        int idx1 = 0;
        int idx2 = 0;
        int i  = 0;
        for(i = 0; i < index+1; i++) {
            if(i>0){
                idx0 = idx2;
            }
            idx1 = left.indexOf('=',idx0);
            if(idx1<0) {
                return null;
            }
            idx2 = left.indexOf('&',idx1);
            if(idx2 == left.length()){
                break;
            }
        }
        if(i<index) {
            return null;
        }
        if(idx0 == 0){
            idx0 = -1;
        }
        name = left.substring(idx0+1,idx1);
        value = left.substring(idx1+1,idx2);
        if(name.equals("TRANS_ID") || name.equals("FILE_SIZE") || name.equals("VER_DSC") ) {
            return value;
        } else {
            return "";
        }
    }

    public static String getDownloadFileName(String downloadUrl)
    {
         int lastidx = -1;
        String filename="";
        String testname = "";
        if(downloadUrl == null) {
            return null;
        }

        lastidx = downloadUrl.lastIndexOf('/');

        if(lastidx ==-1) {
            return "";
        }

        filename = downloadUrl.substring(lastidx +1);
        //to check if the last string if is .com  .com.cn  .org .net .cc .me
        lastidx = -1;
        lastidx = filename.lastIndexOf('.');
        testname = filename.substring(lastidx+1);
        if(testname.equals("com") || testname.equals("cn") || testname.equals("org") ||
           testname.equals("net") || testname.equals("cc") || testname.equals("me")) {
            return "";
        }
        //to ignore '?' charactor endlenss string
        lastidx = -1;
        lastidx = filename.lastIndexOf('?');
        if(lastidx != -1) {
            filename = filename.substring(0,lastidx);
        }

        return filename;
    }

    public static void log(String log)
    {
        System.out.println("[WMMPFileDownloadTask] 1:"+log);
    }
}
