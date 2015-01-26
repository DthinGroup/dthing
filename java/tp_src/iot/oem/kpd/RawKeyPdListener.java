/*
 * Copyright 2014 Yarlung Soft. All rights reserved.
 */

package iot.oem.kpd;

public interface RawKeyPdListener {

    public static final int KEY_0 = 0x1;
    public static final int KEY_1 = 0x2;
    public static final int KEY_2 = 0x4;
    public static final int KEY_3 = 0x8;
    public static final int KEY_4 = 0x10;
    public static final int KEY_5 = 0x20;
    public static final int KEY_6 = 0x40;
    public static final int KEY_7 = 0x80;
    public static final int KEY_8 = 0x100;
    public static final int KEY_9 = 0x200;

    public static final int KEY_LEFT  = 0x10000;
    public static final int KEY_UP    = 0x20000;
    public static final int KEY_RIGHT = 0x40000;
    public static final int KEY_DOWN  = 0x80000;

    public static final int KEY_ASTERISK = 0x400;
    public static final int KEY_POUND    = 0x800;

    public static final int KEY_SELECT = 0x100000;
    public static final int KEY_SOFT1  = 0x200000;
    public static final int KEY_SOFT2  = 0x400000;
    public static final long KEY_CLEAR = 0x100000000L;

    void keyStateChanged(long curkey);
}
