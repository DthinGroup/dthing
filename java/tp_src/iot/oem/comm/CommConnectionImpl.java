package iot.oem.comm;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import java.lang.AsyncIO;

public class CommConnectionImpl extends Object {
    public static final int READ = 0;
    public static final int WRITE = 1;
    public static final int READ_WRITE = 2;
    private static final int COM0 = 0;
    private static final int COM1 = 1;
    public static final int DEVICE_NORMAL = 0;
    public static final int DEVICE_GPS = 1;

    private static boolean isConnected = false;
    private static CommConnectionImpl connection = null;
    private static int hPort = COM1;

    private static OutputStream outs = null;
    private static InputStream ins = null;

    private CommConnectionImpl()
    {
        //TODO:
    }

    /**
     * get com instance and try to connect com device
     *
     * @return instance that connected to com device, or null for failure
     */
    public static CommConnectionImpl getComInstance(int port)
    {
        return getComInstance(port, 115200, DEVICE_NORMAL);
    }

    /**
     * get com instance and try to connect com device
     *
     * @return instance that connected to com device, or null for failure
     */
    public static CommConnectionImpl getComInstance(int port, int baudrate)
    {
        return getComInstance(port, baudrate, DEVICE_NORMAL);
    }

    /**
     * get com instance and try to connect com device
     *
     * @return instance that connected to com device, or null for failure
     */
    public static CommConnectionImpl getComInstance(int port, int baudrate, int device)
    {
        if (connection == null)
        {
            connection = new CommConnectionImpl();
        }

        if (!isConnected)
        {
            isConnected = (open0(port, baudrate, device) < 0)? false : true;

            if (isConnected == true)
            {
                hPort = port;
            }
        }
        else if (port != hPort)
        {
            //any port has been opened before.
            return null;
        }
        return connection;
    }

    private static native int open0(int port, int baudrate, int device);

    /**
     * get baud rate of com
     *
     * @return baud rate when success, or negative for failure
     * @throws IOException when failed to get baud rate
     */
    public int getBaudRate() throws IOException
    {
        int rate = -1;

        if (!isConnected)
        {
            throw new IOException("Com device is not connected");
        }

        rate = getBaudRate0(hPort);
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

        if (!isConnected)
        {
            throw new IOException("Com device is not connected");
        }

        if (bps < 0)
        {
            throw new IllegalArgumentException("bps is illegal");
        }

        if (setBaudRate0(hPort, bps) >= 0)
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
        if (!isConnected)
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
        if (!isConnected)
        {
            throw new IOException("Com device is not connected");
        }

        if (ins == null)
        {
            ins = new comm_PrivateInputStream(connection);
        }

        return ins;
    }

    /**
     * open output stream for com device
     *
     * @return output stream when success
     * @throws IOException when exception happens
     */
    public OutputStream openOutputStream() throws IOException
    {
        if (!isConnected)
        {
            throw new IOException("Com device is not connected");
        }

        if (outs == null)
        {
            outs = new comm_PrivateOutputStream(connection);
        }

        return outs;
    }

    /**
     * close connection to com device
     *
     * @return void
     * @throws IOException when exception happens
     */
    public void close() throws IOException
    {
        if (!isConnected)
        {
            throw new IOException("Com device is not connected");
        }

        if (close0(hPort) < 0)
        {
            throw new IOException("Failed to close com connection");
        }

        if (ins != null)
        {
            ins.close();
            ins = null;
        }

        if (outs != null)
        {
            outs.close();
            outs = null;
        }

        isConnected = false;
    }

    private static native int close0(int port);

    /**
     * Input stream for the connection
     */
    static class comm_PrivateInputStream extends InputStream
    {

        /**
         * Buffer for single char reads
         */
        byte[] buf = new byte[1];

        /**
         * Pointer to the connection
         */
        private CommConnectionImpl parent;

        /**
         * Constructor
         * @param parent to the connection object
         *
         * @throws IOException  if an I/O error occurs.
         */
        public comm_PrivateInputStream(CommConnectionImpl parent) throws IOException
        {
            this.parent = parent;
        }

        /**
         * Check the stream is open
         *
         * @throws  IOException  if the stream is not open.
         */
        void ensureOpen() throws IOException
        {
            if (!parent.isConnected) {
                throw new IOException("Stream closed");
            }
        }

        /**
         * Reads the next byte of data from the input stream.
         * <p>
         * Polling the native code is done here to allow for simple
         * asynchronous native code to be written. Not all implementations
         * work this way (they block in the native code) but the same
         * Java code works for both.
         *
         * @return     the next byte of data, or <code>-1</code>
         *             if the end of the stream is reached.
         * @throws  IOException  if an I/O error occurs.
         */
        synchronized public int read() throws IOException
        {
            if (read(buf, 0, 1) > 0) {
                return (buf[0] & 0xFF);
            }
            return -1;
        }

        /**
         * Reads up to <code>len</code> bytes of data from the input stream into
         * an array of bytes.
         * <p>
         * Polling the native code is done here to allow for simple
         * asynchronous native code to be written. Not all implementations
         * work this way (they block in the native code) but the same
         * Java code works for both.
         *
         * @param      b     the buffer into which the data is read.
         * @param      off   the start offset in array <code>b</code>
         *                   at which the data is written.
         * @param      len   the maximum number of bytes to read.
         * @return     the total number of bytes read into the buffer, or
         *             <code>-1</code> if there is no more data because the end of
         *             the stream has been reached.
         * @throws  IOException  if an I/O error occurs.
         */
        synchronized public int read(byte[] b, int off, int len) throws IOException
        {
            ensureOpen();

            // Ensure the given off and len are valid for b
            if ((off | len | (off + len) | (b.length - (off + len))) < 0)
                throw new IndexOutOfBoundsException();

            for(;;) {
                int count = read1(parent.hPort, b, off, len);
                if (count != 0)
                {
                    if (count > len) { // Is this really needed?
                        throw new IOException("Read overrun in comm:");
                    }
                    return count;
                }
            }
        }

        /*
         * read1
         */
        int read1(int port, byte[] b, int off, int len) throws IOException
        {
            int nBytes = 0;

            while (AsyncIO.loop())
            {
                nBytes = readBytes0(port, b, off, len);
            }

            if(nBytes < 0)
            {
                throw new IOException("Failed to read bytes " + nBytes);
            }

            return nBytes;
        }

        /**
         * Close the stream
         *
         * @throws  IOException  if an I/O error occurs.
         */
        public synchronized void close() throws IOException
        {
            if (parent.isConnected) {
                parent.close();
                parent = null;
            }
        }
    }

    /**
     * Output stream for the connection
     */
    static final class comm_PrivateOutputStream extends OutputStream
    {

        /**
         * Pointer to the connection
         */
        CommConnectionImpl parent;

        /**
         * Buffer for single char writes
         */
        byte[] buf = new byte[1];

        /**
         * Constructor
         *
         * @param pointer to the parent connection
         */
        public comm_PrivateOutputStream(CommConnectionImpl p)
        {
            parent = p;
        }

        /**
         * Check the stream is open
         *
         * @throws  IOException  if the stream is not open.
         */
        void ensureOpen() throws IOException
        {
            if (!parent.isConnected)
            {
                throw new IOException("Stream closed");
            }
        }

        /**
         * Writes the specified byte to this output stream.
         * <p>
         * Polling the native code is done here to allow for simple
         * asynchronous native code to be written. Not all implementations
         * work this way (they block in the native code) but the same
         * Java code works for both.
         *
         * @param      b   the <code>byte</code>.
         * @throws  IOException  if an I/O error occurs. In particular,
         *             an <code>IOException</code> may be thrown if the
         *             output stream has been closed.
         */
        synchronized public void write(int b) throws IOException
        {
            buf[0] = (byte)b;
            write(buf, 0, 1);
        }

        /**
         * Writes <code>len</code> bytes from the specified byte array
         * starting at offset <code>off</code> to this output stream.
         * <p>
         * Polling the native code is done here to allow for simple
         * asynchronous native code to be written. Not all implementations
         * work this way (they block in the native code) but the same
         * Java code works for both.
         *
         * @param      b     the data.
         * @param      off   the start offset in the data.
         * @param      len   the number of bytes to write.
         * @throws  IOException  if an I/O error occurs. In particular,
         *             an <code>IOException</code> is thrown if the output
         *             stream is closed.
         */
        synchronized public void write(byte[] b, int off, int len) throws IOException
        {
            int result=0;
            int test;

            ensureOpen();

            if (len <= 0) {
                return;
            }

            // test the parameters so we don't crash in the native code
            test = b[off] + b[off + len - 1];

            while (AsyncIO.loop())
            {
                result = writeBytes0(parent.hPort, b, off, len);
            }

            if(result < 0)
            {
                throw new IOException("Failed to write data with result " + result);
            }
        }

        /**
         * Close the stream
         *
         * @throws  IOException  if an I/O error occurs.
         */
        public synchronized void close() throws IOException
        {
            if (parent.isConnected)
            {
                parent.close();
                parent = null;
            }
        }
    }

    /**
     *
     * Reads bytes from an opened port.
     *
     * @param hPort The port's native handle.
     * @param b Array of bytes where the bytes are to be written to.
     * @param off The offset in the <code>b</code> array where the first
     * byte is to be written.
     * @param len The number of bytes to be read.
     * @return The number of bytes read if > 0, or an error code.
     */
    private static native int readBytes0(int hPort, byte[] b, int off, int len) throws IOException;

    /**
     * Write bytes to an opened port.
     *
     * @param hPort The port's native handle.
     * @param b The array that contains the bytes to be written.
     * @param off The offset of the first byte to be written from the <code>b</code> array.
     * @param len The number of bytes to be written.
     * @return 0 If successful, or an error code.
     */
    private static native int writeBytes0(int hPort, byte[] b, int off, int len) throws IOException;
}
