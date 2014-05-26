
package iot.oem.sim;

import java.io.IOException;

public class SimManager extends Object {

    public static final int SIM_STATUS_READY = 0;
    public static final int SIM_STATUS_NO_CARD = 1;
    public static final int SIM_STATUS_PIN_WAITING = 2;
    public static final int SIM_STATUS_PIN_BLOCKED = 3;
    public static final int SIM_STATUS_ERROR = 4;

    private static SimManager me = null;
    private static int status = SIM_STATUS_READY;
    private SimStatusListener simStatusListener;

    /**
     * 得到唯一的SimManager实例
     *
     * @return SimManager实例
     * @throws SimException 创建SimManager失败时抛出
     */
    public static SimManager getInstance() throws SimException {
        if (me == null) {
            me = new SimManager();
        }
        status = SIM_STATUS_READY;
        return me;
    }

    /**
     * 释放SIM卡管理器的相关资源
     *
     * @throws SimException SIM卡管理器资源释放失败时抛出
     */
    public void destroy() throws SimException {
        if (close0() < 0) {
            throw new SimException("Failed to release sim card related resource");
        }

        status = SIM_STATUS_NO_CARD;
        if (simStatusListener != null) {
            simStatusListener.onStatusChanged(status);
        }
    }

    private static native int close0();

    /**
     * 开启PIN1密码保护功能
     *
     * @param pinCode - PIN1密码
     * @throws NullPointerException 参数为空时抛出
     * @throws IllegalStateException 当SIM卡状态不处于{@link #SIM_STATUS_READY}时抛出
     * @throws SimException 开启操作失败时抛出
     * @throws IOException 如果SimManager对象已被销毁时抛出
     */
    public void enablePinLock(String pinCode) throws NullPointerException, IllegalStateException,
            SimException, IOException {
        if (me == null) {
            throw new IOException("SimManager instance is destroyed");
        }

        if (status == SIM_STATUS_NO_CARD) {
            throw new IOException("SimManager is closed or no card");
        }

        if (pinCode == null) {
            throw new NullPointerException("Pin code is null");
        }

        if (status != SIM_STATUS_READY) {
            throw new IllegalStateException("Sim card is not ready");
        }

        if (enablePinLock0(pinCode) < 0) {
            throw new SimException("Failed to enable pin lock " + pinCode);
        }

        status = SIM_STATUS_PIN_WAITING;
        if (simStatusListener != null) {
            simStatusListener.onStatusChanged(status);
        }
    }

    private static native int enablePinLock0(String pinCode);

    /**
     * 关闭PIN1密码保护功能
     *
     * @param pinCode PIN1密码
     * @throws NullPointerException 参数为空时抛出
     * @throws IllegalStateException 当SIM卡状态不处于{@link #SIM_STATUS_READY}时抛出
     * @throws SimException 关闭操作失败时抛出
     * @throws IOException 如果SimManager对象已被销毁时抛出
     */
    public void disablePinLock(String pinCode) throws NullPointerException, IllegalStateException,
            SimException, IOException {
        if (me == null) {
            throw new IOException("SimManager instance is destroyed");
        }

        if (status == SIM_STATUS_NO_CARD) {
            throw new IOException("SimManager is closed or no card");
        } else if (status != SIM_STATUS_READY) {
            throw new IllegalStateException("Sim card is not ready");
        }

        if (pinCode == null) {
            throw new NullPointerException("Pin code is null");
        }

        if (disablePinLock0(pinCode) < 0) {
            throw new SimException("Failed to disable pin lock " + pinCode);
        }
        if (simStatusListener != null) {
            simStatusListener.onStatusChanged(status);
        }
    }

    private static native int disablePinLock0(String pinCode);

    /**
     * 解密PIN1密码保护功能,直到重新启动设备
     *
     * @param pinCode PIN1密码
     * @throws IllegalStateException 当SIM卡状态不处于{@link #SIM_STATUS_PIN_WAITING}时抛出
     * @throws NullPointerException 参数为空时抛出
     * @throws SimException 解密操作时抛出
     * @throws IOException 如果SimManager对象已被销毁时抛出
     */
    public void unlockPin(String pinCode) throws NullPointerException, IllegalStateException,
            SimException, IOException {
        if (me == null) {
            throw new IOException("SimManager instance is destroyed");
        }

        if (status == SIM_STATUS_NO_CARD) {
            throw new IOException("SimManager is closed or no card");
        } else if (status != SIM_STATUS_PIN_WAITING) {
            throw new IllegalStateException("Sim card is not waiting pin");
        }

        if (pinCode == null) {
            throw new NullPointerException("Pin code is null");
        }

        if (unlockPin0(pinCode) < 0) {
            throw new SimException("Failed to unlock pin " + pinCode);
        }

        status = SIM_STATUS_READY;
        if (simStatusListener != null) {
            simStatusListener.onStatusChanged(status);
        }
    }

    private static native int unlockPin0(String pinCode);

    /**
     * 修改PIN1密码
     *
     * @param oldPin 原始密码
     * @param newPin 新密码
     * @throws IllegalStateException 当SIM卡状态不处于{@link #SIM_STATUS_READY}时抛出
     * @throws NullPointerException 参数为空时抛出
     * @throws SimException 修改PIN1密码失败时抛出
     * @throws IOException 如果SimManager对象已被销毁时抛出
     */
    public void changePin(String oldPin, String newPin) throws NullPointerException,
            IllegalStateException, SimException, IOException {
        if (me == null) {
            throw new IOException("SimManager instance is destroyed");
        }

        if (status == SIM_STATUS_NO_CARD) {
            throw new IOException("SimManager is closed or no card");
        } else if (status != SIM_STATUS_READY) {
            throw new IllegalStateException("Sim card is not ready");
        }

        if (oldPin == null || newPin == null) {
            throw new NullPointerException("Pin code is null");
        }

        if (changePin0(oldPin, newPin) < 0) {
            throw new SimException("Failed to change pin code");
        }
        status = SIM_STATUS_READY;
        if (simStatusListener != null) {
            simStatusListener.onStatusChanged(status);
        }
    }

    private static native int changePin0(String oldPin, String newPin);

    /**
     * 获得SIM卡状态
     *
     * @return SIM卡状态
     * @throws IOException 如果SimManager对象已被销毁时抛出
     */
    public int getSimStatus() throws IOException {
        if (me == null) {
            throw new IOException("SimManager instance is destroyed");
        }

        if (status == SIM_STATUS_NO_CARD) {
            throw new IOException("SimManager is closed or no card");
        }
        return status;
    }

    /**
     * 设置或注销SIM卡状态监听器 当listener不为空,表示设置新的监听器,原有监听器将被覆盖;当listener为空,表示注销已经注册的监听器
     *
     * @param listener SIM卡状态监听器
     */
    public void setStatusListener(SimStatusListener statusListener) {
        simStatusListener = statusListener;
    }
}
