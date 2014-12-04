
package com.yarlungsoft.ams;

import java.io.File;
import java.io.IOException;
import java.net.ota.OTAConfig;
import java.net.ota.OTADownload;
import java.net.ota.OTAListener;

public class TCKRunner extends Thread implements OTAListener {

    /** Output debug traces, should be false when checked in */
    private static final boolean DEBUG = AmsConfig.debug();

    /** The number of tries for installing test Applets */
    private static final int TRIES_FOR_TEST_FINISHED = 3;

    /** The number of milliseconds to wait between retries */
    private static final int RETRY_DELAY = 2000;

    /** The number of milliseconds to wait before removing */
    private static final int REMOVE_DELAY = 2000;

    /** The interval in milliseconds of checking OTA status */
    private static final long CHECK_INTERVAL = 5000;

    private byte[] lock = new byte[0];
    private int otaResult = OTAConfig.OTA_UNKNOWN;
    private String otaStorageFilename = null;

    private String jadUrl = null;
    private boolean restartRunner = false;

    public TCKRunner() {
        super();
        setPriority(MAX_PRIORITY);
    }

    public TCKRunner(String jadUrl) {
        this();
        this.jadUrl = jadUrl;
    }

    /**
     * Print a string to stdout if DEBUG is true, prefixed with "TCKRunner: ".
     *
     * @param log the string to print
     */
    private static void log(String log) {
        if (DEBUG) {
            System.out.println("TCKRunner: " + log);
        }
    }

    @Override
    public void onResult(int result) {
        synchronized (lock) {
            otaResult = result;
            lock.notifyAll();
        }
    }

    private void sleep0(long time) {
        try {
            sleep(time);
        } catch (InterruptedException e) {
            // ignore
        }
    }

    private boolean downloadUrl(String url) {
        int tries = 0;
        boolean isDownloaded = false;
        OTADownload ota = null;
        otaStorageFilename = null;

        /*
         * We need to distinguish between failures that are expected during the testing, and
         * failures that should cause us to stop testing and print an error message. We print
         * error messages for everything, to make sure we have the information in the log.
         * <p>
         * We ignore all failures - just reset the VM and try again - with the following
         * exceptions. This can lead to the VM restarting in a furious tempo. If that happened
         * to you, consider adding to this list of exceptions:
         * OTA_IO_ERROR, OTA_TASK_FAIL, OTA_NET_ERROR: received when the TCK is finished
         * <p>
         * For these exceptions we re-try 3 times (TRIES_FOR_TEST_FINISHED) to avoid exiting for
         * something that was part of a normal test.
         */
        while (!isDownloaded && tries < TRIES_FOR_TEST_FINISHED) {
            log("installing from " + url + " (try " + (tries + 1) + "/" + TRIES_FOR_TEST_FINISHED
                    + ")");

            ota = new OTADownload(jadUrl);
            ota.setListener(this);

            synchronized (lock) {
                otaResult = OTAConfig.OTA_UNKNOWN;
                ota.OTAStart();

                while (otaResult == OTAConfig.OTA_UNKNOWN) {
                    try {
                        lock.wait(CHECK_INTERVAL);
                    } catch (InterruptedException ie) {
                        // ignore
                    }
                }

                isDownloaded = (otaResult == OTAConfig.OTA_SUCCESS);

                switch (otaResult) {
                case OTAConfig.OTA_SUCCESS:
                    tries = 0;
                    break;
                case OTAConfig.OTA_FILE_ERROR:
                    log("folder not found: " + ota.getAppDir());
                    tries = 0;
                    break;
                case OTAConfig.OTA_INVALID_URL:
                    log("invalid TCK url: " + jadUrl);
                    tries = 0;
                    break;

                case OTAConfig.OTA_IO_ERROR:
                case OTAConfig.OTA_TASK_FAIL:
                    log("failed to download: " + jadUrl);
                    tries++;
                    break;
                case OTAConfig.OTA_NET_ERROR:
                    log("cannot connect to: " + jadUrl);
                    tries++;
                    break;
                }
            }

            if (!isDownloaded) {
                log("waiting for " + (RETRY_DELAY / 1000) + " seconds");
                sleep0(RETRY_DELAY);
                if (tries == 0) {
                    break;
                }
            }
        }

        if (isDownloaded) {
            otaStorageFilename = ota.getStorageFilename();
            log("downloaed " + otaStorageFilename);
        } else {
            if (tries >= TRIES_FOR_TEST_FINISHED) {
                log("install() failing " + TRIES_FOR_TEST_FINISHED
                        + " times, we suppose tests are finished");
                restartRunner = false;
            } else {
                log("install() failing, we restart the vm and retry");
                restartRunner = true;
            }
        }

        return isDownloaded;
    }

    private void removeCurDownloadedFile() {
        if (otaStorageFilename == null) {
            return;
        }
        File f = new File(otaStorageFilename);
        if (f.exists()) {
            f.delete();
        }
    }

    public void run() {
        log("started!");
        boolean downloaded = false;

        for (;;) {
            downloaded = downloadUrl(jadUrl);
            if (!downloaded) {
                if (restartRunner) {
                    continue;
                } else {
                    break;
                }
            }

            JadInfo jadInfo = null;
            try {
                jadInfo = new JadParser().parseJad(otaStorageFilename);
                jadInfo.jarUrl = JadParser.checkSetJarUrl(jadUrl, jadInfo.jarUrl);
            } catch (IOException e) {
                // ignore
            }
            if (jadInfo == null || jadInfo.jarUrl == null || jadInfo.jarUrl.length() == 0
                    || jadInfo.appClass == null || jadInfo.appClass.length() == 0) {
                if (restartRunner) {
                    continue;
                } else {
                    break;
                }
            }

            // delete downloaded jad file
            removeCurDownloadedFile();

            downloaded = downloadUrl(jadInfo.jarUrl);
            if (!downloaded) {
                if (restartRunner) {
                    continue;
                } else {
                    break;
                }
            }

            Main.launchApp(jadInfo.appClass);

            /* waiting for the applet finishing */
            while (Scheduler.getCurrentRunningApp() != null) {
                sleep0(CHECK_INTERVAL);
            }

            /*
             * The sleep is to allow tracing to come out on slow machines, and to prevent spinning
             * if something goes wrong.
             */
            sleep0(REMOVE_DELAY);

            // delete downloaded jar file
            removeCurDownloadedFile();
        }
        log("ended!");
    }
}
