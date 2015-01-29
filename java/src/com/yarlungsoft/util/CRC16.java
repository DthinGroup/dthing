package com.yarlungsoft.util;

public class CRC16 {
    private byte[] CRC=new byte[2];
    
    byte data;
    byte[] reg=new byte[2];
    byte[] ploy=new byte[2];
    byte Op;
    
    public CRC16(byte[] source)
    {
        //��ʼ������ʽ
        int temp=0xA001;
        ploy=InttoByteArray(temp,2);
        //��ʼ���Ĵ���
        temp=0xFFFF;
        reg=InttoByteArray(temp,2);
        
        for(int i=0;i<source.length;i++)
        {
            //��ȡ����
            data=source[i];
            //��Ĵ��������ݽ���������
            reg[1]=(byte) (reg[1]^data);
            //�ƶ�����
            for(int j=0;j<8;j++)
            {
                //��ȡ���ݵ����һλ�������ƶ����������ж��Ƿ������ʽ���
                Op=reg[0];
                //����һλ
                reg=InttoByteArray(ByteArraytoInt(reg)>>1,2);
                //����Ƴ�����Ϊ1
                if((Op&0x01)==1)
                {
                    //�����ʽ����������
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
//��ʽ����������
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