
package java.net.ota;

public interface OTAListener {

    /**
     * Call the method when {@link OTADownload} thread reports certain result.
     *
     * @param result one of the following constrains in {@link OTAConfig}:
     * {@link OTAConfig#OTA_SUCCESS}, {@link OTAConfig#OTA_INVALID_URL},
     * {@link OTAConfig#OTA_IO_ERROR}, {@link OTAConfig#OTA_FILE_ERROR},
     * {@link OTAConfig#OTA_NET_ERROR}, {@link OTAConfig#OTA_TASK_FAIL}.
     */
    void onResult(int result);
}
