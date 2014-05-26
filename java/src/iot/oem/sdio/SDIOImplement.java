
package iot.oem.sdio;

import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class SDIOImplement extends Object {

    private boolean isActive = false;
    private String filename = null;
    private FileInputStream fis = null;
    private FileOutputStream fos = null;

    public SDIOImplement(String fname) throws IOException {
        this.filename = getFilePath(fname);
        isActive = true;
    }

    public InputStream getInputStream() throws IOException {
        if (!isActive) {
            throw new IOException("SDIO Connection is not active");
        }

        if (fis == null) {
            fis = new FileInputStream(filename);
        }
        return fis;
    }

    public OutputStream getOutputStream() throws IOException {
        if (!isActive) {
            throw new IOException("SDIO Connection is not active");
        }

        if (fos == null) {
            fos = new FileOutputStream(filename);
        }
        return fos;
    }

    public void close() throws IOException {
        if (!isActive) {
            throw new IOException("SDIO Connection is not active");
        }
        if (fis != null) {
            fis.close();
        }
        if (fos != null) {
            fos.close();
        }
        isActive = false;
    }

    private String getFilePath(String filename) {
        // TODO: if file is not exist on SD, create it, else return path
        return filename;
    }
}
