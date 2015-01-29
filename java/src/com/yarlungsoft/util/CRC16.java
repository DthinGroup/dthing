package com.yarlungsoft.util;

public class CRC16 {
    private byte[] CRC=new byte[2];
    
    byte data;
    byte[] reg=new byte[2];
    byte[] ploy=new byte[2];
    byte Op;
    
    public CRC16(byte[] source)
    {
        //ԵʼۯנЮʽ
        int temp=0xA001;
        ploy=InttoByteArray(temp,2);
        //Եʼۯ݄զǷ
        temp=0xFFFF;
        reg=InttoByteArray(temp,2);
        
        for(int i=0;i<source.length;i++)
        {
            //ܱȡ˽ߝ
            data=source[i];
            //ԫ݄զǷא˽ߝ޸ѐӬܲәط
            reg[1]=(byte) (reg[1]^data);
            //ӆ֯˽ߝ
            for(int j=0;j<8;j++)
            {
                //ܱȡ˽ߝքخ۳һλìܴѻӆ֯Զք˽ߝƐ׏ˇرԫנЮʽӬܲ
                Op=reg[0];
                //Ԓӆһλ
                reg=InttoByteArray(ByteArraytoInt(reg)>>1,2);
                //ɧڻӆԶ˽ߝΪ1
                if((Op&0x01)==1)
                {
                    //ԫנЮʽ޸ѐӬܲәط
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
//ٱʽۯب׺گ˽
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