
package iot.oem.micophone;

import java.io.IOException;

public class Micophone extends Object {

    public static final int CI_MIC = 0;
    public static final int CI_HEADSET = 1;

    public static final int CG_MUTE = 0;
    public static final int CG_CLASS0 = 1;
    public static final int CG_CLASS1 = 2;
    public static final int CG_CLASS2 = 3;
    public static final int CG_CLASS3 = 4;
    public static final int CG_CLASS4 = 5;
    public static final int CG_CLASS5 = 6;
    public static final int CG_CLASS6 = 7;
    public static final int CG_MAX = 8;

    private static Micophone me = null;
    private static boolean isOpened = false;

    /**
     * 打开设备的麦克风功能并获取 Microphone 唯一的对象
     *
     * @return Microphone 对象，打开设备的麦克风功能失败，则返回{@code null}。
     */
    public static Micophone getInstance() {
        if (me == null) {
            me = new Micophone();

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
     * 获取当前已经打开的麦克风的状态
     *
     * @return 如果是{@code true}，表示当前麦克风可以使用；如果是{@code false}，表示当前麦克风禁止使用
     * @throws IOException 操作失败时抛出
     */
    public boolean getStatus() throws IOException {
        int status = getStatus0();
        if (status < 0) {
            throw new IOException("Failed to get micophone status");
        }
        return status > 0;
    }

    private static native int getStatus0();

    /**
     * 使能或者禁止已经打开的麦克风
     *
     * @param status 如果是 {@code true}，表示可以使用麦克风；如果是{@code false}，表示禁止使用麦克风
     * @throws IOException 设置参数失败时抛出
     */
    public void setStatus(boolean status) throws IOException {
        if (setStatus0(status) < 0) {
            throw new IOException("Failed to set micophone status");
        }
    }

    private static native int setStatus0(boolean status);

    /**
     * 获取当前打开的麦克风的语音的输入路径的设置
     *
     * @return 麦克风语音的输入路径，只能是下列值之一：{@link #CI_MIC}， {@link #CI_HEADSET}
     * @throws IOException 操作失败时抛出
     */
    public int getInPath() throws IOException {
        int inPath = getInPath0();
        if (!isValidInPath(inPath)) {
            throw new IOException("Failed to get micophone inpath");
        }
        return inPath;
    }

    private static native int getInPath0();

    /**
     * 针对打开的麦克风作语音的输入路径的设置
     *
     * @param inPath 麦克风语音的输入路径，只能取下列值之一：{@link #CI_MIC}， {@link #CI_HEADSET}
     * @throws IllegalArgumentException 参数非法时抛出
     * @throws IOException 设置参数失败时抛出
     */
    public void setInPath(int inPath) throws IllegalArgumentException, IOException {
        if (!isValidInPath(inPath)) {
            throw new IllegalArgumentException("Illegal argument");
        }

        if (setInPath0(inPath) < 0) {
            throw new IOException("Failed to set micophone inpath");
        }
    }

    private static native int setInPath0(int inPath);

    /**
     * 获取当前打开的麦克风的输入增益的设置
     *
     * @return 麦克风语音的输入增益，只能是下列值之一：{@link #CG_MUTE}，{@link #CG_CLASS0}，{@link #CG_CLASS1}，
     * {@link #CG_CLASS2}，{@link #CG_CLASS3}，{@link #CG_CLASS4}，{@link #CG_CLASS5}，
     * {@link #CG_CLASS6}，{@link #CG_MAX}.
     * @throws IOException 操作失败时抛出
     */
    public int getGain() throws IOException {
        int gain = getGain0();
        if (!isValidGain(gain)) {
            throw new IOException("Failed to get micophone gain");
        }
        return gain;
    }

    private static native int getGain0();

    /**
     * 针对打开的麦克风作输入增益的设置
     *
     * @param gain 麦克风语音的输入增益，只能取下列值之一：{@link #CG_MUTE}，{@link #CG_CLASS0}，{@link #CG_CLASS1}，
     * {@link #CG_CLASS2}，{@link #CG_CLASS3}，{@link #CG_CLASS4}，{@link #CG_CLASS5}，
     * {@link #CG_CLASS6}，{@link #CG_MAX}.
     * @throws IllegalArgumentException 参数非法时抛出
     * @throws IOException 设置参数失败时抛出
     */
    public void setGain(int gain) throws IllegalArgumentException, IOException {
        if (!isValidGain(gain)) {
            throw new IllegalArgumentException("Illegal argument");
        }

        if (setGain0(gain) < 0) {
            throw new IOException("Failed to set micophone gain");
        }
    }

    private static native int setGain0(int gain);

    /**
     * 关闭设备的麦克风功能并释放相关资源
     *
     * @throws IOException
     */
    public void close() throws IOException {
        if (close0() < 0) {
            throw new IOException("Failed to close micophone");
        }
    }

    private static native int close0();

    private boolean isValidInPath(int inPath) {
        return inPath == CI_MIC || inPath == CI_HEADSET;
    }

    private boolean isValidGain(int gain) {
        return (gain >= CG_CLASS0 && gain <= CG_MAX) || gain == CG_MUTE;
    }
}
