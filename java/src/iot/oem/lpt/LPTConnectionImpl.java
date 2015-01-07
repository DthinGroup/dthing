package iot.oem.lpt;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

public class LPTConnectionImpl {

    /**
     * Access mode READ.
     */
    public static final int READ = 1;

    /**
     * Access mode WRITE.
     */
    public static final int WRITE = 2;

    /**
     * Access mode READ_WRITE.
     */
    public static final int READ_WRITE = (READ | WRITE);

    private int baudRate;

    public LPTConnectionImpl() {
    }

    public void close() throws IOException {
        // FIXME: invoke native method to convert
        throw new IOException();
    }

    public static LPTConnectionImpl getLPT1Instance() {
        // FIXME: invoke native method to convert
        return null;
    }

    public DataInputStream openDataInputStream() throws IOException {
        // FIXME: invoke native method to convert
        throw new IOException();
    }

    public DataOutputStream openDataOutputStream() throws IOException {
        // FIXME: invoke native method to convert
        throw new IOException();
    }

    public InputStream openInputStream() throws IOException {
        // FIXME: invoke native method to convert
        throw new IOException();
    }

    public OutputStream openOutputStream() throws IOException {
        // FIXME: invoke native method to convert
        throw new IOException();
    }

    public int getBaudRate() {
        return baudRate;
    }

    public int setBaudRate(int bps) {
        baudRate = bps;
        return baudRate;
    }
}
