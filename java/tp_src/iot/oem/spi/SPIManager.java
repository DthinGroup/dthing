
package iot.oem.spi;

import java.io.IOException;

public class SPIManager extends Object {
	public static final int SPI_CPOL0_CPHA0 = 0; //sampling on rising edge, clk idle '0'
    public static final int SPI_CPOL0_CPHA1 = 1; //sampling on falling edge, clk idle '0'
    public static final int SPI_CPOL1_CPHA0 = 2; //sampling on falling edge, clk idle '1'
    public static final int SPI_CPOL1_CPHA1 = 3; //sampling on rising edge, clk idle '1'
    public static final int SPI_TX_BIT_LEN_DEFAULT = 8;
    private int handle = -1;
    private int busId = -1;

    /**
     * construct the SPIManager instance by specified SPI bus id and rate
     *
     * @param spiBusId id of SPI bus
     * @param rate value of rate by KHz
     * @return the SPIManager instance
     * @throws IllegalArgumentException when argument is illegal
     * @throws IOException when failed to construct
     */
    public SPIManager(int busId, int rate) throws IllegalArgumentException, IOException {
        if (!isValidBusId(busId) || !isValidRate(rate)) {
            throw new IllegalArgumentException("Illegal arguments");
        }

        handle = open0(busId, rate, SPI_CPOL0_CPHA0, SPI_TX_BIT_LEN_DEFAULT);
        if (handle < 0) {
            throw new IOException("Failed to open busId " + busId + " with rate " + rate);
        }

        this.busId = busId;
    }

     /**
     * construct the SPIManager instance by specified SPI bus id and rate
     *
     * @param spiBusId id of SPI bus
     * @param rate value of rate by KHz
     * @param mode CPOL and CPHA status
     * @param bitlen tx bit length
     * @return the SPIManager instance
     * @throws IllegalArgumentException when argument is illegal
     * @throws IOException when failed to construct
     */
    public SPIManager(int spiBusId, int rate, int mode, int bitlen) throws IllegalArgumentException, IOException
    {
        if (!isValidBusId(spiBusId) || !isValidRate(rate))
        {
            throw new IllegalArgumentException("Illegal arguments");
        }

        this.handle = open0(spiBusId, rate, mode, bitlen);

        if (this.handle < 0)
        {
            throw new IOException("Failed to open busId " + spiBusId + " with rate " + rate);
        }

        this.busId = spiBusId;
    }

    private static native int open0(int busId, int rate, int mode, int bitlen);

    /**
     * do get rate of SPI
     *
     * @return value of rate by KHz
     * @throws IOException when failed to get SPI rate
     */
    public int getRate() throws IOException {
        int result = getRate0(busId);
        if (result < 0) {
            throw new IOException("Failed to get rate");
        }
        return result / 1024;
    }

    private static native int getRate0(int busId);

    /**
     * do set rate of SPI
     *
     * @param rate value of rate by KHz
     * @return void
     * @throws IllegalArgumentException when argument is illegal
     * @throws IOException when failed to set SPI rate
     */
    public void setRate(int rate) throws IllegalArgumentException, IOException {
        if (!isValidRate(rate)) {
            throw new IllegalArgumentException("Illegal argument");
        }

        if (setRate0(busId, rate * 1024) < 0) {
            throw new IOException("Failed to set rate " + rate + " KHz");
        }
    }

    private static native int setRate0(int busId, int rate);

    /**
     * destroy SPI instance
     *
     * @throws IOException when failed to destroy
     */
    public void destroy() throws IOException {
        if (close0(busId) < 0) {
            throw new IOException("Failed to close SPI manager");
        }

        handle = -1;
        busId = -1;
    }

    private static native int close0(int busId);

    /**
     * receive data from target SPI device
     *
     * @param address value of target device address
     * @param buff byte array to receive data
     * @return length of data had read
     * @throws NullPointerException when data is null
     * @throws IOException when failed to read data from SPI device
     */
    public int receive(int address, byte[] buff) throws NullPointerException, IOException {
        // TODO: Check illegal address

        if (buff == null) {
            throw new NullPointerException("Null buff pointer");
        }

        int readLength = read0(this.handle, address, buff, buff.length);
        if (readLength < 0) {
            throw new IOException("failed to read out data");
        }
        return readLength;
    }

    private static native int read0(int busId, int addr, byte[] buffer, int len);

    /**
     * send data into SPI device
     *
     * @param address value of target device address
     * @param data byte array of data to sent
     * @return void
     * @throws NullPointerException when data is null
     * @throws IOException when failed to write data into SPI device
     */
    public void send(int address, byte[] data) throws NullPointerException, IOException {
        // TODO: Check illegal address

        if (data == null) {
            throw new NullPointerException("Null buff pointer");
        }

        if (write0(this.handle, address, data, data.length) < 0) {
            throw new IOException("failed to read out data");
        }
    }

    private static native int write0(int busId, int addr, byte[] data, int len);

    private boolean isValidRate(int rate) {
        // TODO:
        return (rate >= 0);
    }

    private boolean isValidBusId(int busId) {
        // TODO:
        return (busId >= 0);
    }
}
