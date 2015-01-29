package com.yarlungsoft.util;

public class DigitalEncrypt
{
    /*-------------MD5----------------*/

    public static byte[] getMD5ByteArray(byte[] bytes) 
    {  
        return MD5.toMD5ByteArray(bytes);
    }
    
}