
package iot.oem.usb;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class USBConnection extends Object {

    private int handle = -1;

    /**
     * USBConnection实例的构造函数.
     *
     * @param usbID USB接口的标识
     * @throws IOException 打开指定USB接口失败时抛出
     */
    public USBConnection(String usbID) throws IOException {
        handle = open0(usbID);
        if (handle < 0) {
            throw new IOException("Failed to open USB connection");
        }
    }

    private static native int open0(String usbID);

    /**
     * 得到USB通信的输入流
     *
     * @return 输入流
     * @throws IOException 得到输入流失败时抛出
     */
    public InputStream getInputStream() throws IOException {
        // TODO:
        throw new IOException("Failed to get input stream");
    }

    /**
     * 得到USB通信的输出流
     *
     * @return 输出流
     * @throws IOException 得到输出流失败时抛出
     */
    public OutputStream getOutputStream() throws IOException {
        // TODO:
        throw new IOException("Failed to get output stream");
    }

    /**
     * 关闭USB连接
     *
     * @throws IOException 关闭USB连接失败时抛出
     */
    public void close() throws IOException {
        if (close0(this.handle) < 0) {
            throw new IOException("Failed to close USB connection");
        }
        handle = -1;
    }

    private static native int close0(int handle);
}
