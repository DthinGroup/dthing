package java.net.ota;

import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.MalformedURLException;
import java.net.http.HttpURLConnection;
import java.net.http.URL;

import com.yarlungsoft.util.Log;

public class OTADownload {
    private static final String TAG = "OTADownload";

    private String OTALink;
    private OTAListener resultListener = null;

    public static void OTA(String[] args) {
        String url;
        if (args.length > 0) {
            url = args[0];
        } else {
            Log.amsLog(TAG, "Ota - nothing to do");
            return;
        }
        Log.amsLog(TAG, "Ota - " + url);
        OTADownload ota = new OTADownload(url);
        ota.OTAStart();
    }

    public OTADownload(String url) {
        OTALink = url;
    }

    public void setListener(OTAListener listener) {
        resultListener = listener;
    }

    public void OTAStart() {
        try {
            new Thread(new OTAThread()).start();
        } catch (Exception e) {
            notifyOTAResult(OTAConfig.OTA_TASK_FAIL);
        }
    }

    private native String getAppInstalledDir();

    private native void notifyOTAResult0(int result);

    public String getAppDir() {
        return getAppInstalledDir();// "D:\\nix.long\\ReDvmAll\\dvm\\appdb\\";
    }

    public String getStorageFilename() {
        return getAppDir() + OTALink.substring(OTALink.lastIndexOf('/') + 1);
    }

    public void notifyOTAResult(int result) {
        switch (result) {
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
        Log.amsLog(TAG, "Result:" + result);
        if (resultListener != null) {
            resultListener.onResult(result);
        }
        notifyOTAResult0(result);
    }

    class OTAThread implements Runnable {

        // @Override
        public void run() {
            URL url;
            try {
                url = new URL(OTALink);
            } catch (MalformedURLException e) {
                e.printStackTrace();
                Log.amsLog(TAG, "Exception: new URL(" + OTALink + ")");
                notifyOTAResult(OTAConfig.OTA_INVALID_URL);
                return;
            }

            HttpURLConnection uConn = null;
            try {
                uConn = (HttpURLConnection) url.openConnection();
            } catch (Exception e) {
                e.printStackTrace();
                Log.amsLog(TAG, "Exception:open connection fail");
                notifyOTAResult(OTAConfig.OTA_NET_ERROR);
                return;
            }

            uConn.setRequestProperty("Accept", "*/*");
            uConn.setRequestProperty("Connection", "Keep-Alive");
            uConn.setDoOutput(false);

            try {
                InputStream in = uConn.getInputStream();

                int byteread = 0, bytesum = 0;
                int leng = uConn.getContentLength();
                FileOutputStream fs = null;

                Log.amsLog(TAG, "Start downlowd, content length:" + leng);

                String file = getStorageFilename();
                Log.amsLog(TAG, "file name:" + file);

                fs = new FileOutputStream(file, true);
                Log.amsLog(TAG, "Start read");
                byte[] buffer = new byte[3072];
                while ((byteread = in.read(buffer)) > 0) {
                    bytesum += byteread;
                    Log.amsLog(TAG, "Read count:" + bytesum);
                    fs.write(buffer, 0, byteread);
                    if (bytesum >= leng) {
                        break;
                    }
                }
                fs.flush();
                fs.close();
            } catch (FileNotFoundException e) {
                notifyOTAResult(OTAConfig.OTA_FILE_ERROR);
                uConn.disconnect();
                return;
            } catch (IOException e) {
                e.printStackTrace();
                notifyOTAResult(OTAConfig.OTA_IO_ERROR);
                uConn.disconnect();
                return;
            }
            uConn.disconnect();
            Log.amsLog(TAG, "downlowd over");
            notifyOTAResult(OTAConfig.OTA_SUCCESS);
        }
    }
}
