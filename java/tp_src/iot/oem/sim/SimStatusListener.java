
package iot.oem.sim;

public interface SimStatusListener {

    /**
     * Call the method when SIM card status changed
     *
     * @param status new status of SIM card
     */
    void onStatusChanged(int status);
}
