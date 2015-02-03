package jp.co.cmcc.wmmp;

import java.io.IOException;


public class WMMPAppConnection extends Object{
    WMMPConnection wConn;
    
    public WMMPAppConnection(WMMPConnection wconn) throws NullPointerException
    {        
        if(wconn ==null)
            throw new NullPointerException();
        wConn = wconn;
    }
    
    public void open() throws IOException
    {
        if(!wConn.isAlive())
        {
            throw new IOException();
        }
    }
        
    public void send(byte[] data) throws IOException,NullPointerException
    {
        try{
            wConn.sendData(WMMPConnection.EC_TYPE, data, "unknown");
        } catch (WMMPException e) {
            throw new IOException();
        } catch (NullPointerException e) {
            throw new NullPointerException();
        }
    }
    
    public void close()
    {
        wConn = null;
    }
}
