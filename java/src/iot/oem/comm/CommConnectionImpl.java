package iot.oem.comm;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class CommConnectionImpl extends Object {
    public static final int READ = 0;
    public static final int WRITE = 1;
    public static final int READ_WRITE = 2;
    private static final int COM0 = 0;  //GPS_COM: 0
    private static final int COM1 = 1;

    private static boolean isConnected = false;
    private static CommConnectionImpl connection = null;
    private static int port = COM0;

    public CommConnectionImpl()
    {
        //TODO:
    }

    /**
     * get com instance and try to connect com device
     *
     * @return instance that connected to com device, or null for failure
     */
    public static CommConnectionImpl getCom1Instance()
    {
        if (CommConnectionImpl.connection == null)
        {
            CommConnectionImpl.connection = new CommConnectionImpl();
        }

        if (!CommConnectionImpl.isConnected)
        {
            CommConnectionImpl.isConnected = (open0(CommConnectionImpl.port) < 0)? false : true;
        }

        return CommConnectionImpl.connection;
    }

    private static native int open0(int port);

    /**
     * get baud rate of com
     *
     * @return baud rate when success, or negative for failure
     * @throws IOException when failed to get baud rate
     */
    public int getBaudRate() throws IOException
    {
        int rate = -1;

        if (!CommConnectionImpl.isConnected)
        {
            throw new IOException("Com device is not connected");
        }

        rate = getBaudRate0(CommConnectionImpl.port);
        return rate;
    }

    private static native int getBaudRate0(int port);

    /**
     * set baud rate of com
     *
     * @param bps int baud rate to set
     * @return worked baud rate, or negative for failure
     * @throws IOException when failed to set baud rate
     * @throws IllegalArgumentException when bps is illegal
     */
    public int setBaudRate(int bps) throws IOException
    {
        int rate = -1;

        if (!CommConnectionImpl.isConnected)
        {
            throw new IOException("Com device is not connected");
        }

        if (bps < 0)
        {
            throw new IllegalArgumentException("bps is illegal");
        }

        if (setBaudRate0(CommConnectionImpl.port, bps) >= 0)
        {
            rate = bps;
        }
        return rate;
    }

    private static native int setBaudRate0(int port, int bps);

    /**
     * open data input stream for com device
     *
     * @return data input stream when success or null otherwise
     * @throws IOException when exception happens
     */
    public DataInputStream openDataInputStream() throws IOException
    {
        if (!CommConnectionImpl.isConnected)
        {
            throw new IOException("Com device is not connected");
        }

        //TODO: Check the best way to manage input/output stream
        DataInputStream is = new DataInputStream(null);

        if (is == null)
        {
            throw new IOException("Failed to create data input stream");
        }

        return null;
    }

    /**
     * open data output stream for com device
     *
     * @return data output stream when success or null otherwise
     * @throws IOException when exception happens
     */
    public DataOutputStream openDataOutputStream() throws IOException
    {
        if (!CommConnectionImpl.isConnected)
        {
            throw new IOException("Com device is not connected");
        }

        //TODO:
        return null;
    }

    /**
     * open input stream for com device
     *
     * @return input stream when success or null otherwise
     * @throws IOException when exception happens
     */
    public InputStream openInputStream() throws IOException
    {
        if (!CommConnectionImpl.isConnected)
        {
            throw new IOException("Com device is not connected");
        }

        //TODO:
        return null;
    }

    /**
     * open output stream for com device
     *
     * @return output stream when success
     * @throws IOException when exception happens
     */
    public OutputStream openOutputStream() throws IOException
    {
        if (!CommConnectionImpl.isConnected)
        {
            throw new IOException("Com device is not connected");
        }

        //TODO:
        return null;
    }

    /**
     * close connection to com device
     *
     * @return void
     * @throws IOException when exception happens
     */
    public void close() throws IOException
    {
        if (!CommConnectionImpl.isConnected)
        {
            throw new IOException("Com device is not connected");
        }

        if (close0(CommConnectionImpl.port) < 0)
        {
            throw new IOException("Failed to close com connection");
        }

        CommConnectionImpl.isConnected = false;
    }

    private static native int close0(int port);
}
