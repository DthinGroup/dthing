
package iot.oem.spi;

import java.io.IOException;

public class SPIManager extends Object {

    private int handle = -1;
    private int busId = -1;

    /**
     * 构造一个SPIManager对象，并打开总线标识参数指定的SPI总线，同时设置总线速率。
     *
     * @param busId SPI总线标识
     * @param rate 数据传输速率，以KHz为单位
     * @throws IllegalArgumentException 参数非法时抛出
     * @throws IOException 构造SPIManager失败时抛出
     */
    public SPIManager(int busId, int rate) throws IllegalArgumentException, IOException {
        if (!isValidBusId(busId) || !isValidRate(rate)) {
            throw new IllegalArgumentException("Illegal arguments");
        }

        handle = open0(busId, rate);
        if (handle < 0) {
            throw new IOException("Failed to open busId " + busId + " with rate " + rate);
        }

        this.busId = busId;
    }

    private static native int open0(int busId, int rate);

    /**
     * 获得数据传输速率，以KHz为单位
     *
     * @return 数据传输速率
     * @throws IOException 获取数据传输速率失败时抛出
     */
    public int getRate() throws IOException {
        int result = getRate0(busId);
        if (result < 0) {
            throw new IOException("Failed to get rate");
        }
        return result / 1024;
    }

    private static native int getRate0(int busId);

    /**
     * 设置数据传输速率， 以KHz为单位
     *
     * @param rate 数据传输速率
     * @throws IllegalArgumentException 参数非法时抛出
     * @throws IOException 设置数据传输速率失败时抛出
     */
    public void setRate(int rate) throws IllegalArgumentException, IOException {
        if (!isValidRate(rate)) {
            throw new IllegalArgumentException("Illegal argument");
        }

        if (setRate0(busId, rate * 1024) < 0) {
            throw new IOException("Failed to set rate " + rate + " KHz");
        }
    }

    private static native int setRate0(int busId, int rate);

    /**
     * 关闭SPIManager对象所打开的总线
     *
     * @throws IOException 关闭总线失败时抛出
     */
    public void destroy() throws IOException {
        if (close0(busId) < 0) {
            throw new IOException("Failed to close SPI manager");
        }

        handle = -1;
        busId = -1;
    }

    private static native int close0(int busId);

    /**
     * 读取来自目标设备的数据
     *
     * @param address 目的设备地址
     * @param buff 接收数据的数组
     * @return 实际读取的数据长度
     * @throws NullPointerException 参数buff为空时抛出
     * @throws IOException 读取数据失败时抛出
     */
    public int receive(int address, byte[] buff) throws NullPointerException, IOException {
        // TODO: Check illegal address

        if (buff == null) {
            throw new NullPointerException("Null buff pointer");
        }

        int readLength = read0(this.handle, address, buff, buff.length);
        if (readLength < 0) {
            throw new IOException("failed to read out data");
        }
        return readLength;
    }

    private static native int read0(int busId, int addr, byte[] buffer, int len);

    /**
     * 向目标设备写入数据
     *
     * @param address 目的设备地址
     * @param data 将被写入的数据
     * @throws NullPointerException 参数data为空时抛出
     * @throws IOException 写入数据失败时抛出
     */
    public void send(int address, byte[] data) throws NullPointerException, IOException {
        // TODO: Check illegal address

        if (data == null) {
            throw new NullPointerException("Null buff pointer");
        }

        if (write0(this.handle, address, data, data.length) < 0) {
            throw new IOException("failed to read out data");
        }
    }

    private static native int write0(int busId, int addr, byte[] data, int len);

    private boolean isValidRate(int rate) {
        // TODO:
        return (rate >= 0);
    }

    private boolean isValidBusId(int busId) {
        // TODO:
        return (busId >= 0);
    }
}
