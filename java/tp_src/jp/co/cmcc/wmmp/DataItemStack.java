package jp.co.cmcc.wmmp;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;

public class DataItemStack
{
    private int curCmd;
    private byte[] rawData;
    private WMMPDataItem[] dataStack;
    private int[] Tags;

    private void praseDataItem()
    {
        if(rawData ==null || rawData.length ==0)
            return;

        ByteArrayInputStream bArrIs = new ByteArrayInputStream(rawData,0,rawData.length);
        DataInputStream dataIs      = new DataInputStream(bArrIs);

        int tag =0;
        int len =0,tlen=0,cnt=0;
        while(tlen < rawData.length)
        {
            try {
                tag = dataIs.readChar();
                len = dataIs.readChar();
                System.out.println("DataItemStack I:tag=" + tag +",len=" +len);
                dataIs.skip(len);
                tlen += 4 + len;
                if(tag != 0xE021)//delete e021
                    cnt++;
            } catch (IOException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
                break;
            }
        }
        try {
            bArrIs.close();
            dataIs.close();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        dataStack = new WMMPDataItem[cnt];
        Tags = new int[cnt];

        bArrIs = new ByteArrayInputStream(rawData,0,rawData.length);
        dataIs = new DataInputStream(bArrIs);

        int i=0;
        tlen = 0;
        while(tlen < rawData.length)
        {
            System.out.println("DataItemStack II:");
            try {
                tag = dataIs.readChar();
                len = dataIs.readChar();
                System.out.println("DataItemStack II:tag=" + tag +",len=" +len);
                if(tag != 0xE021)
                {
                    byte[] d = new byte[len];
                    dataIs.read(d, 0, len);
                    Tags[i] = tag;
                    dataStack[i] = new WMMPDataItem(tag,d);
                    i++;
                }
                tlen += 4 + len;
            } catch (IOException e) {
                // TODO Auto-generated catch block
                System.out.println("DataItemStack II: fuck");
                e.printStackTrace();
                break;
            }
        }

        try {
            bArrIs.close();
            dataIs.close();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

    }
    //data[] must be TLV data
    public DataItemStack(int cmd,byte[] data)
    {
        curCmd = cmd;
        rawData = new byte[data.length];
        for(int i=0;i<data.length;i++)
        {
            rawData[i] = data[i];
            //System.arrayCopy is rubbish!!
        }

        praseDataItem();
    }

    public byte[] getRawData()
    {
        return rawData;
    }

    public WMMPDataItem getDataItem(int tag)
    {
        if(dataStack ==null)
            return null;
        for(int i=0;i<dataStack.length;i++)
        {
            if(dataStack[i].getTag() == tag)
            {
                System.out.println("WMMPDataItem,find tag:"+tag);
                return dataStack[i];
            }
        }
        return null;
    }

    public int getCurStackCmd()
    {
        return curCmd;
    }

    public int[] getTagsSet()
    {
        return Tags;
    }

    public WMMPDataItem[] getAllDataItem()
    {
        return dataStack;
    }
}
