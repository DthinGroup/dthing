
package iot.oem.i2c;

import java.io.IOException;

public class I2CManager extends Object {

    public static final int ADDRESS_TYPE_7BIT = 0;
    public static final int ADDRESS_TYPE_10BIT = 1;

    public static final int DATA_RATE_STANDARD = 0; // I2C bus standard transfer rate: 100Kbits/s
    public static final int DATA_RATE_FAST = 1; // I2C bus standard transfer rate: 400Kbits/s
    public static final int DATA_RATE_HIGH = 2; // I2C bus standard transfer rate: 3.4Mbits/s

    private int rate = -1;
    private int busId = -1;
    private int handle = -1;

    private static boolean isEnabled = false;

    /**
     * construct I2CManager instance, open specified I2C bus and set rate
     *
     * @param I2CBusId int I2C bus id
     * @param rate int data transfer rate. DATA_RATE_STANDARD/DATA_RATE_FAST/DATA_RATE_HIGH
     * @return I2CManager instance
     * @throws IllegalArgumentException when has illegal arguments
     * @throws IOException when failed to create instance
     */
    public I2CManager(int I2CBusId, int rate, int slaveAddress, int regAddressNumber) throws IllegalArgumentException, IOException
    {
        //FIXME: As defined in tck, invalid rate or bus id should throw IOException
        if(!isValidBusId(I2CBusId))
        {
            throw new IOException("Illegal i2c busid " + I2CBusId);
        }

        if(!isValidRate(rate))
        {
          throw new IllegalArgumentException("Illegal i2c rate " + rate);
        }

        this.rate = rate;
        this.busId = I2CBusId;
        this.handle = open0(I2CBusId, rate, slaveAddress, regAddressNumber);

        if (handle < 0)
        {
            throw new IOException("Failed to open gpio pin");
        }
        this.isEnabled = true;
    }

    /**
     * construct I2CManager instance, open specified I2C bus and set rate
     *
     * @param I2CBusId int I2C bus id
     * @param rate int data transfer rate. DATA_RATE_STANDARD/DATA_RATE_FAST/DATA_RATE_HIGH
     * @return I2CManager instance
     * @throws IllegalArgumentException when has illegal arguments
     * @throws IOException when failed to create instance
     */
    public I2CManager(int I2CBusId, int rate, int slaveAddress) throws IllegalArgumentException, IOException
    {
        //FIXME: As defined in tck, invalid rate or bus id should throw IOException
        if(!isValidBusId(I2CBusId))
        {
            throw new IOException("Illegal i2c busid " + I2CBusId);
        }

        if(!isValidRate(rate))
        {
          throw new IllegalArgumentException("Illegal i2c rate " + rate);
        }

        this.rate = rate;
        this.busId = I2CBusId;
        this.handle = open0(I2CBusId, rate, slaveAddress, 0);

        if (handle < 0)
        {
            throw new IOException("Failed to open gpio pin");
        }
        this.isEnabled = true;
    }


    /**
     * construct I2CManager instance, open specified I2C bus and set rate
     *
     * @param I2CBusId int I2C bus id
     * @param rate int data transfer rate. DATA_RATE_STANDARD/DATA_RATE_FAST/DATA_RATE_HIGH
     * @return I2CManager instance
     * @throws IllegalArgumentException when has illegal arguments
     * @throws IOException when failed to create instance
     */
    public I2CManager(int I2CBusId, int rate) throws IllegalArgumentException, IOException
    {
        //FIXME: As defined in tck, invalid rate or bus id should throw IOException
        if(!isValidBusId(I2CBusId))
        {
            throw new IOException("Illegal i2c busid " + I2CBusId);
        }

        if(!isValidRate(rate))
        {
          throw new IllegalArgumentException("Illegal i2c rate " + rate);
        }

        this.rate = rate;
        this.busId = I2CBusId;
        this.handle = open0(I2CBusId, rate, 0xB8, 0);

        if (handle < 0)
        {
            throw new IOException("Failed to open gpio pin");
        }
        this.isEnabled = true;
    }

    private static native int open0(int busId, int freq, int slaveAddr, int regAddrNum);

    /**
     * Get data transfer rate
     *
     * @exception IOException when failed to get transfer rate
     * @return data transfer rate. {@link #DATA_RATE_STANDARD}, {@link #DATA_RATE_FAST}, or
     * {@link #DATA_RATE_HIGH}
     */
    public int getRate() throws IOException {
        if (!isAvailable()) {
            throw new IOException("I2CManager instance is destroyed");
        }

        int result = getRate0(handle);

        if (!isValidRate(result)) {
            throw new IOException("failed to get rate");
        }

        rate = result;
        return rate;
    }

    private static native int getRate0(int handle);

    /**
     * Set data transfer rate
     *
     * @exception IllegalArgumentException when has illegal arguments
     * @exception IOException when failed to set transfer rate
     */
    public void setRate(int rate) throws IllegalArgumentException, IOException {
        if (!isValidRate(rate)) {
            throw new IllegalArgumentException("illegal rate");
        }

        if (!isAvailable()) {
            throw new IOException("I2CManager instance is destroyed");
        }

        if (setRate0(handle, rate) < 0) {
            throw new IOException("failed to set rate");
        }
        this.rate = rate;
    }

    private static native int setRate0(int handle, int rate);

    /**
     * Close bus that opened by I2CManager instance
     *
     * @exception IOException when failed to close bus
     */
    public void destroy() throws IOException {
        if (!isAvailable()) {
            throw new IOException("I2CManager instance is destroyed");
        }

        if (close0(handle) < 0) {
            throw new IOException("failed to close bus " + this.busId);
        }
        handle = -1;
        busId = -1;
        rate = -1;
        isEnabled = false;
    }

    private static native int close0(int handle);

    /**
     * Mutex operation with read()
     *
     * @param destAddr address of target device
     * @param addressType address type of target device
     * @param subAddr not used
     * @param buff byte[] buffer to receive those read data
     * @exception IllegalArgumentException when has illegal arguments
     * @exception NullPointerException when parameter buff is null
     * @exception IOException when failed to read out data
     * @return actual read length of data
     */
    synchronized public int receive(int destAddr, int addressType, int subAddr, byte[] buff)
            throws IllegalArgumentException, NullPointerException, IOException {
        if (buff == null) {
            throw new NullPointerException("Null buff pointer");
        }

        if (destAddr < 0 || subAddr < 0) {
            throw new IOException("Not expected dest addr");
        }

        if (!isValidAddressType(addressType)) {
            throw new IllegalArgumentException("Not expected address type");
        }

        if (!isAvailable()) {
            throw new IOException("I2CManager instance is destroyed");
        }

        if (!verifyAddressBit(destAddr, addressType, subAddr)) {
            throw new IOException("error address and type");
        }

        int readLength = read0(handle, destAddr, subAddr, buff, buff.length);

        if (readLength < 0) {
            throw new IOException("failed to read out data");
        }

        return readLength;
    }

    private static native int read0(int handle, int addr, int subAddr, byte[] buffer, int len);

    /**
     * Mutex operation with receive()
     *
     * @param destAddr address of target device
     * @param addressType address type of target device
     * @param subAddr not used
     * @param data byte[] data to write in
     * @exception IllegalArgumentException when has illegal arguments
     * @exception NullPointerException when parameter data is null
     * @exception IOException when failed to write in data
     */
    synchronized public void send(int destAddr, int addressType, int subAddr, byte[] data)
            throws IllegalArgumentException, NullPointerException, IOException {
        if (data == null) {
            throw new NullPointerException("Null buff pointer");
        }

        if (destAddr < 0 || subAddr < 0) {
            throw new IOException("Not expected dest addr");
        }

        if (!isValidAddressType(addressType)) {
            throw new IllegalArgumentException("Not expected address type");
        }

        if (!isAvailable()) {
            throw new IOException("I2CManager instance is destroyed");
        }

        if (!verifyAddressBit(destAddr, addressType, subAddr)) {
            throw new IOException("error address and type");
        }

        if (write0(handle, destAddr, subAddr, data, data.length) < 0) {
            throw new IOException("failed to read out data");
        }
    }

    private static native int write0(int handle, int addr, int subAddr, byte[] data, int len);

    private boolean isValidBusId(int busId) {
        return busId >= 0;
    }

    private boolean isValidRate(int rate) {
        return rate == DATA_RATE_STANDARD || rate == DATA_RATE_FAST || rate == DATA_RATE_HIGH;
    }

    private boolean isAvailable() {
        return isEnabled;
    }

    private boolean isValidAddressType(int type) {
        return type == ADDRESS_TYPE_7BIT || type == ADDRESS_TYPE_10BIT;
    }

    private boolean verifyAddressBit(int destAddr, int addressType, int subAddr)
    {
        boolean result = true;

        //TODO: verfiy address bit

        return result;
    }
}
