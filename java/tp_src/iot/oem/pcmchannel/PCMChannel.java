
package iot.oem.pcmchannel;

import java.io.IOException;

public class PCMChannel extends Object {

    public static final int PCM_CLK_128k = 0;
    public static final int PCM_CLK_256k = 1;
    public static final int PCM_CLK_512k = 2;

    public static final int PCM_DATA_MSB = 0;
    public static final int PCM_DATA_LSB = 1;

    private static PCMChannel me = null;
    private static boolean isOpened = false;

    /**
     * 打开设备的PCM channel 并获取 PCMChannel 唯一的对象
     *
     * @return PCMChannel 对象，打开设备的PCM Channel失败，则返回{@code null}。
     */
    public static PCMChannel getInstance() {
        if (me == null) {
            me = new PCMChannel();

            if (open0() < 0) {
                me = null;
            } else {
                isOpened = true;
            }
        }
        return me;
    }

    private static native int open0();

    /**
     * 获取已经打开的PCM Channel的状态
     *
     * @return 如果是{@code true}，表示当前PCM Channel可以使用；如果是{@code false}，表示当前PCM Channel禁止使用
     * @throws IOException 操作失败时抛出
     * @throws IllegalArgumentException
     */
    public boolean getStatus() throws IllegalArgumentException, IOException {
        int status = getStatus0();
        if (status < 0) {
            throw new IOException("Failed to get PCM channel status");
        }
        return status > 0;
    }

    private static native int getStatus0();

    /**
     * 使能或者禁止已经打开的PCM Channel
     *
     * @param status 如果是{@code true}，表示可以使用PCM Channel；如果是{@code false}，表示禁止使用PCM Channel
     * @throws IOException 设置参数失败时抛出
     */
    public void setStatus(boolean status) throws IOException {
        if (setStatus0(status) < 0) {
            throw new IOException("Failed to set PCM channel status");
        }
    }

    private static native int setStatus0(boolean status);

    /**
     * 获取PCM Channel的频率
     *
     * @return PCM Channel 的频率，只能是下列值之一：{@link #PCM_CLK_128k}，{@link #PCM_CLK_256k}，
     * {@link #PCM_CLK_512k}
     * @throws IOException 操作失败时抛出
     */
    public int getFrequency() throws IOException {
        int freq = getFrequency0();
        if (!isValidFrequency(freq)) {
            throw new IOException("Failed to get PCM channel frequency");
        }
        return freq;
    }

    private static native int getFrequency0();

    /**
     * 针对打开的PCM Channel作频率的设置
     *
     * @param freq PCM Channel 的频率，只能取下列值之一：{@link #PCM_CLK_128k}，{@link #PCM_CLK_256k}，
     * {@link #PCM_CLK_512k}
     * @throws IllegalArgumentException 参数非法时抛出
     * @throws IOException 设置参数失败时抛出
     */
    public void setFrequency(int freq) throws IllegalArgumentException, IOException {
        if (!isValidFrequency(freq)) {
            throw new IllegalArgumentException("Illegal argument " + freq);
        }

        if (setFrequency0(freq) < 0) {
            throw new IOException("Failed to set PCM channel frequency");
        }
    }

    private static native int setFrequency0(int freq);

    /**
     * 获取PCM Channel的位优先设置
     *
     * @return PCM Channel 的位优先设置，只能是下列值之一：{@link #PCM_DATA_MSB}，{@link #PCM_DATA_LSB}
     * @throws IOException 操作失败时抛出
     */
    public int getMode() throws IOException {
        int mode = getMode0();
        if (!isValidMode(mode)) {
            throw new IOException("Failed to get PCM channel mode");
        }
        return mode;
    }

    private static native int getMode0();

    /**
     * 针对打开的PCM Channel作位优先设置
     *
     * @param mode PCM Channel 的位优先设置，只能取下列值之一：{@link #PCM_DATA_MSB}，{@link #PCM_DATA_LSB}
     * @throws IllegalArgumentException 参数非法时抛出
     * @throws IOException 设置参数失败时抛出
     */
    public void setMode(int mode) throws IllegalArgumentException, IOException {
        if (!isValidMode(mode)) {
            throw new IllegalArgumentException("Illegal argument " + mode);
        }

        if (setMode0(mode) < 0) {
            throw new IOException("Failed to set PCM channel mode");
        }
    }

    private static native int setMode0(int mode);

    /**
     * 关闭设备的PCM Channel并释放相关资源
     *
     * @throws IOException
     */
    public void close() throws IOException {
        if (close0() < 0) {
            throw new IOException("Failed to close PCM channel");
        }
    }

    private static native int close0();

    private boolean isValidFrequency(int freq) {
        return freq == PCM_CLK_128k || freq == PCM_CLK_256k || freq == PCM_CLK_512k;
    }

    private boolean isValidMode(int mode) {
        return mode == PCM_DATA_MSB || mode == PCM_DATA_LSB;
    }
}
