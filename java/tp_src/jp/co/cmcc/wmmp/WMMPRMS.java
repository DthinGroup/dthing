package jp.co.cmcc.wmmp;

import java.io.*;
import java.util.*;
//import javax.microedition.io.file.*;

public class WMMPRMS
{
    private static final boolean DEBUG = false;

    //in jbed/spreadtrum
/*
    static String RmsFilePath = AMSConfig.getRoot() + "/LocalApp/";
*/
    //in dthing/linux
//    static String RmsFilePath = System.getenv("FDVM_ROOT")+"/DthingRoot" + "/LocalApp/";
    //static String RmsFilePath = System.getProperty("File.rootdir");
    static String RmsFilePath = "C:\\dvm\\";
    static String RmsFileName = "WmmpRMS.data";
    static String RmsFile     = RmsFilePath + RmsFileName;

    private final static String RMS_REGISTERSTATE_KEY = "RegisterState:";
    private final static String RMS_TERMSEQSTR_KEY    = "TermSeq:";
    private final static String RMS_UPLINKPWD_KEY     = "UpLinkPwd:";
    private final static String RMS_DOWNLINKPWD_KEY   = "DownLinkPwd:";
    private final static String RMS_IMEI_KEY          = "Imei:";
    private final static String RMS_IMSI_KEY          = "Imsi:";
    private final static String RMS_TERMSOFTVER_KEY   = "TermSoftVer:";
    private final static String RMS_DEVIDE            = "\n";

    private static String RMS_REGISTERSTATE    ;
    private static String RMS_TERMSEQSTR       ;
    private static String RMS_UPLINKPWD        ;
    private static String RMS_DOWNLINKPWD      ;
    private static String RMS_IMEI             ;
    private static String RMS_IMSI             ;
    private static String RMS_TERMSOFTVER      ;

    /**
     * Get available roots.
     */

    static
    {
        if(WMMPConnection.ARM_VERSION)
        {
            RmsFilePath = "Phone/";
        }
        else
        {
        	RmsFilePath =System.getProperty("File.rootdir");
        }
        /*
    	log("RmsFilePath I.:" + RmsFilePath);    	
    	if(RmsFilePath ==null)
    	{
        String availableRoot = null;
        long maxSize = 0;
        File rms = new File(RmsFilePath);
        File[] rootEnum = rms.listFiles();
        //Enumeration rootEnum = FileSystemRegistry.listRoots();
        for(int i=0;i<rootEnum.length;i++)
        {
        //while (rootEnum.hasMoreElements()) {
            //String root = (String) rootEnum.nextElement();
        	String root = rootEnum[i].getPath();
            FileConnection fc;
            try {
                fc = (FileConnection) javax.microedition.io.Connector.open("file:///" + root);
                long availSize = fc.availableSize();
                if (availSize > maxSize) {
                    maxSize = availSize;
                    availableRoot = root;
                }
                fc.close();
            } catch (IOException e) {
                e.printStackTrace();
            }
        }
        RmsFilePath = availableRoot;
        }*/
        RmsFile = RmsFilePath + RmsFileName;

        log("RmsFilePath:" + RmsFilePath);
        log("RmsFile:" + RmsFile);
    }


    WMMPRMS()
    {

    }

    public synchronized static void resetRmsFile()
    {
        RMS_REGISTERSTATE = "false";
        RMS_TERMSEQSTR    = WMMPConnection.TERMSEQSTR  ;
        RMS_UPLINKPWD     = WMMPConnection.UPLINKPWD   ;
        RMS_DOWNLINKPWD   = WMMPConnection.DOWNLINKPWD ;
        RMS_IMEI          = WMMPConnection.IMEI        ;
        RMS_IMSI          = WMMPConnection.IMSI        ;
        RMS_TERMSOFTVER   = WMMPConnection.TERMSOFTVER ;

        log("resetRmsFile");
        try
        {
            //FileConnection fcdes = (FileConnection)javax.microedition.io.Connector.open("file:///" + RmsFile);
        	File file = new File(RmsFile);
        	if(!file.exists())
        	{
        		file.createNewFile();
        	}
        	/*
            if(!fcdes.exists())
            {
                log("RmsFile file not exist -- create");
                fcdes.create();
            }*/

            log("resetRmsFile ,write");
            /**
            FileOutputStream fos = new FileOutputStream(RmsFile);
            */
            //OutputStream fos = fcdes.openOutputStream();
            FileOutputStream fos = new FileOutputStream(file);

            /*format : key + value + divide*/
            String tmpStr = RMS_REGISTERSTATE_KEY + RMS_REGISTERSTATE   + RMS_DEVIDE +
                            RMS_TERMSEQSTR_KEY    + RMS_TERMSEQSTR      + RMS_DEVIDE +
                            RMS_UPLINKPWD_KEY     + RMS_UPLINKPWD       + RMS_DEVIDE +
                            RMS_DOWNLINKPWD_KEY   + RMS_DOWNLINKPWD     + RMS_DEVIDE +
                            RMS_IMEI_KEY          + RMS_IMEI            + RMS_DEVIDE +
                            RMS_IMSI_KEY          + RMS_IMSI            + RMS_DEVIDE +
                            RMS_TERMSOFTVER_KEY   + RMS_TERMSOFTVER     + RMS_DEVIDE;

            fos.write(tmpStr.getBytes());

            fos.close();
            //fcdes.close();

            log("resetRmsFile ok!");
        }
        catch (IOException ioe)
        {
            log("Reset Rms file fail!");
            ioe.printStackTrace();
        }
    }

    public synchronized static void loadRmsData()
    {
        /*first set default value*/
        RMS_REGISTERSTATE = "false";
        RMS_TERMSEQSTR    = WMMPConnection.TERMSEQSTR  ;
        RMS_UPLINKPWD     = WMMPConnection.UPLINKPWD   ;
        RMS_DOWNLINKPWD   = WMMPConnection.DOWNLINKPWD ;
        RMS_IMEI          = WMMPConnection.IMEI        ;
        RMS_IMSI          = WMMPConnection.IMSI        ;
        RMS_TERMSOFTVER   = WMMPConnection.TERMSOFTVER ;

        log("in loadRmsData");

        try
        {
            /*FileConnection fconn ;
            try
            {
                fconn = (FileConnection)javax.microedition.io.Connector.open("file:///" + RmsFile);
            }
            catch(Exception e)
            {
                e.printStackTrace();
                return;
            }
            if(fconn == null)
                return;
                */

        	/*
            // If no exception is thrown, then the URI is valid, but the file may or may not exist.
            if(!fconn.exists() || fconn.fileSize() == 0)
            {
                fconn.close();
                resetRmsFile();
            }*/
        	File file = new File(RmsFile);
        	if(!file.exists() || file.length() == 0)
        	{
        		resetRmsFile();
        	}
            else
            {
            		/*
                InputStream fis = fconn.openInputStream();
                int filesize = (int)fconn.fileSize();
                int read = 0;
                byte[] buff = new byte[filesize];

                read = fis.read(buff);
                log("Read size:" + read + ",hope size is:" + filesize);
                fis.close();
                fconn.close();
                */
            	FileInputStream fis = new FileInputStream(file);
            	int filesize = (int)file.length();
            	int read = 0;
            	byte[] buff = new byte[filesize];
            	
            	read = fis.read(buff);
                log("Read size:" + read + ",hope size is:" + filesize);
                fis.close();
                String rmsStr = new String(buff);

                String[] key =
                {
                    RMS_REGISTERSTATE_KEY,
                    RMS_TERMSEQSTR_KEY   ,
                    RMS_UPLINKPWD_KEY    ,
                    RMS_DOWNLINKPWD_KEY  ,
                    RMS_IMEI_KEY         ,
                    RMS_IMSI_KEY         ,
                    RMS_TERMSOFTVER_KEY
                };

                String[] val =
                {
                    "false",
                    WMMPConnection.TERMSEQSTR  ,
                    WMMPConnection.UPLINKPWD   ,
                    WMMPConnection.DOWNLINKPWD ,
                    WMMPConnection.IMEI        ,
                    WMMPConnection.IMSI        ,
                    WMMPConnection.TERMSOFTVER
                };

                int i=0;
                String tmp;
                int fstIdx =0,sndIdx =0;
                for(i=0;i<key.length;i++)
                {
                    tmp = new String(rmsStr);
                    fstIdx = tmp.indexOf(key[i]);
                    sndIdx = tmp.indexOf(RMS_DEVIDE);

                    if(fstIdx ==-1 || sndIdx ==-1)
                    {
                        continue;
                    }
                    fstIdx += key[i].length();
                    if(fstIdx >= sndIdx)
                    {
                        continue;
                    }
                    val[i] = tmp.substring(fstIdx,sndIdx);
                    log("Rms val[" + i + "]:" + val[i] + ",len:" + val[i].length());

                    if(i!=key.length-1)
                    {
                        rmsStr = rmsStr.substring(sndIdx+1);
                    }
                }

                RMS_REGISTERSTATE = val[0];
                RMS_TERMSEQSTR    = val[1];
                RMS_UPLINKPWD     = val[2];
                RMS_DOWNLINKPWD   = val[3];
                RMS_IMEI          = val[4];
                RMS_IMSI          = val[5];
                RMS_TERMSOFTVER   = val[6];
            }
        }
        catch (Exception ioe)
        {
            ioe.printStackTrace();
        }
    }

    public synchronized static String getRmsVal(int params)
    {
        String val = null;

        switch(params)
        {
            case WMMPConnection.PARAM_REGISTERSTATE:   { val = RMS_REGISTERSTATE;} break;
            case WMMPConnection.PARAM_TERMSEQSTR:      { val = RMS_TERMSEQSTR   ;} break;
            case WMMPConnection.PARAM_UPLINKPWD:       { val = RMS_UPLINKPWD    ;} break;
            case WMMPConnection.PARAM_DOWNLINKPWD:     { val = RMS_DOWNLINKPWD  ;} break;
            case WMMPConnection.PARAM_IMEI:            { val = RMS_IMEI         ;} break;
            case WMMPConnection.PARAM_IMSI:            { val = RMS_IMSI         ;} break;
            case WMMPConnection.PARAM_TERMSOFTVER:     { val = RMS_TERMSOFTVER  ;} break;
            default: break;
        }

        return val;
    }

    public  static boolean setRmsVal(int params,String val)
    {
        log("setRmsVal: 1");
        if(val == null)
        {
            return false;
        }
        log("setRmsVal:" + params + " - "+val);
        switch(params)
        {
            case WMMPConnection.PARAM_REGISTERSTATE:   { RMS_REGISTERSTATE = val;} break;
            case WMMPConnection.PARAM_TERMSEQSTR:      { RMS_TERMSEQSTR    = val;} break;
            case WMMPConnection.PARAM_UPLINKPWD:       { RMS_UPLINKPWD     = val;} break;
            case WMMPConnection.PARAM_DOWNLINKPWD:     { RMS_DOWNLINKPWD   = val;} break;
            case WMMPConnection.PARAM_IMEI:            { RMS_IMEI          = val;} break;
            case WMMPConnection.PARAM_IMSI:            { RMS_IMSI          = val;} break;
            case WMMPConnection.PARAM_TERMSOFTVER:     { RMS_TERMSOFTVER   = val;} break;
            default: return false;
        }

        new Thread( new SaveRMSWorker()).start();

        return true;
    }

    private static void log(String str)
    {
        if(DEBUG)
            System.out.println("WmmpRMS - " + str);
    }

    private static class SaveRMSWorker implements Runnable
    {
        SaveRMSWorker()
        {

        }
        public void run()
        {
            // TODO Auto-generated method stub
            try
            {
                //FileConnection fconn = (FileConnection)javax.microedition.io.Connector.open("file:///" + RmsFile);
            	File file = new File(RmsFile);
                //fconn.delete();
                //fconn.create();
                log("setRmsVal: 2");
                String tmpStr = RMS_REGISTERSTATE_KEY + RMS_REGISTERSTATE   + RMS_DEVIDE +
                                RMS_TERMSEQSTR_KEY    + RMS_TERMSEQSTR      + RMS_DEVIDE +
                                RMS_UPLINKPWD_KEY     + RMS_UPLINKPWD       + RMS_DEVIDE +
                                RMS_DOWNLINKPWD_KEY   + RMS_DOWNLINKPWD     + RMS_DEVIDE +
                                RMS_IMEI_KEY          + RMS_IMEI            + RMS_DEVIDE +
                                RMS_IMSI_KEY          + RMS_IMSI            + RMS_DEVIDE +
                                RMS_TERMSOFTVER_KEY   + RMS_TERMSOFTVER     + RMS_DEVIDE;

                //OutputStream fos = fconn.openOutputStream();
                FileOutputStream fos = new FileOutputStream(file);
                log("setRmsVal: 3");
                fos.write(tmpStr.getBytes());
                log("setRmsVal: 4");
                fos.close();
                //fconn.close();

                log("setRmsVal ok!");
            }
            catch(Exception e)
            {
                log("setRmsVal fail!");
                e.printStackTrace();
            }
        }
    }
}
