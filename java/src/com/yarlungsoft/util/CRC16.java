package com.yarlungsoft.util;

public class CRC16 {
    private byte[] CRC=new byte[2];
    
    byte data;
    byte[] reg=new byte[2];
    byte[] ploy=new byte[2];
    byte Op;
    
    public CRC16(byte[] source)
    {
        //初始化多项式
        int temp=0xA001;
        ploy=InttoByteArray(temp,2);
        //初始化寄存器
        temp=0xFFFF;
        reg=InttoByteArray(temp,2);
        
        for(int i=0;i<source.length;i++)
        {
            //获取数据
            data=source[i];
            //与寄存器中数据进行异或操作
            reg[1]=(byte) (reg[1]^data);
            //移动数据
            for(int j=0;j<8;j++)
            {
                //获取数据的最后一位，即被移动出的数据判断是否与多项式异或
                Op=reg[0];
                //右移一位
                reg=InttoByteArray(ByteArraytoInt(reg)>>1,2);
                //如果移出数据为1
                if((Op&0x01)==1)
                {
                    //与多项式进行异或操作
                    reg[0]=(byte) (reg[0]^ploy[0]);
                    reg[1]=(byte) (reg[1]^ploy[1]);
                }                
            }
        }
        CRC=reg;        
    }

    public byte[] getCRC() 
    {
        return CRC;
    }
//格式化辅助函数
    private static byte[] InttoByteArray(int iSource, int iArrayLen) 
    {
        byte[] bLocalArr = new byte[iArrayLen];
        for ( int i = 0; (i < 4) && (i < iArrayLen); i++) 
        {
            bLocalArr[i] = (byte)( iSource>>8*i & 0xFF );          
        }
        return bLocalArr;
    }
    
    private static int ByteArraytoInt(byte[] bRefArr) 
    {
        int iOutcome = 0;
        byte bLoop;
        
        for ( int i =0; i<bRefArr.length ; i++) 
        {
            bLoop = bRefArr[i];
            iOutcome+= (bLoop & 0xFF) << (8 * i);          
        }         
        return iOutcome;
    }    
}