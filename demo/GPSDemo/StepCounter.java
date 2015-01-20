/** This class is used for calculating steps through acceleration data.*/

public class StepCounter {
    private static boolean DEBUG = true;
    private static boolean isXAxisMax = false;
    private static boolean isBadStep = false;

    private static final double UNDEFINED_VALUE = -10.0;
    private static double maxXAc = UNDEFINED_VALUE;
    private static double minXAc = UNDEFINED_VALUE;
    private static double maxYAc = UNDEFINED_VALUE;
    private static double minYAc = UNDEFINED_VALUE;
    private static final double MIN_VALID_STEP_GAP = 0.3;
    private static final double MIN_STEP_DURATION = 0.2;
    private static final double MAX_STEP_DURATION = 2;
    /** The frequency of data output. */
    private static final double DATA_OUTPUT_FREQ = 0.05;
    /** Caculate threshold dynamically every 10 output data. */
    private static final int SAMPLING_NUMBER = 10;
    /** sampling counter, reset to 0 every SAMPLING_NUMBER times. */
    private static int samCounter = 0;
    /** x axis dynamic threshold. */
    private static double xDcThred = UNDEFINED_VALUE;
    /** y axis dynamic threshold. */
    private static double yDcThred = UNDEFINED_VALUE;

    /** The state of calculating step. */
    private static final int STEP_INITIALIZING = 0;
    private static final int STEP_SEARCHING = 1;
    private static final int STEP_WORKING = 2;

    /** The test acceleration data of x and y axis. */
    private static double xAc[] = {-0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49 };
    private static double yAc[] = {-1.00, -0.80, -0.63, -4.65, -2.64, -0.18, -0.28, -0.25, -0.20, -0.23,
                                   -0.32, -0.40, -0.58, -0.63, -0.60, -0.50, -0.40, -0.25, -0.12, -0.10,
                                   -0.15, -0.22, -0.31, -0.49, -0.60, -0.51, -0.40, -0.31, -0.21, -0.16,
                                   -0.22, -0.38, -0.45, -0.50, -0.62, -0.56, -0.43, -0.32, -0.19, -0.18,
                                   -0.15, -0.22, -0.31, -0.49, -0.60, -0.51, -0.40, -0.31, -0.21, -0.16,
                                   -0.22, -0.38, -0.45, -0.50, -0.62, -0.56, -0.43, -0.32, -0.19, -0.18,
                                   -0.15, -0.22, -0.31, -0.49, -0.60, -0.51, -0.40, -0.31, -0.21, -0.16,
                                   -0.22, -0.38, -0.45, -0.50, -0.62, -0.56, -0.43, -0.32, -0.19, -0.18,
                                   -0.15, -0.22, -0.31, -0.49, -0.60, -0.51, -0.40, -0.31, -0.21, -0.16,
                                   -0.22, -0.38, -0.45, -0.50, -0.62, -0.56, -0.43, -0.32, -0.19, -0.18 };


    StepCounter() {
        initAcData();
    }

    /** Initialize data. */
    private static void initAcData() {
        samCounter = 0;
        maxXAc = UNDEFINED_VALUE;
        minXAc = UNDEFINED_VALUE;
        maxYAc = UNDEFINED_VALUE;
        minYAc = UNDEFINED_VALUE;
        xDcThred = UNDEFINED_VALUE;
        yDcThred = UNDEFINED_VALUE;
    }

    /** Reset data for another cycle of sampling. */
    private static void resetSamplingData() {
        samCounter = 0;
        /** Caculate dynamic threshold value. */
        xDcThred = (maxXAc+minXAc)/2;
        yDcThred = (maxYAc+minYAc)/2;

        if (maxXAc-minXAc >= MIN_VALID_STEP_GAP ||
            maxYAc-minYAc >= MIN_VALID_STEP_GAP) {
            isBadStep = false;
            /** Find the axis whose acceleration changes is the largest. */
            if (maxXAc-minXAc > maxYAc-minYAc) {
                isXAxisMax = true;
            } else {
                isXAxisMax = false;
            }
        } else {
            isBadStep = true;
        }

        log("maxXAc = " + maxXAc + ",minXAc = " + minXAc +
            " maxYAc = " + maxYAc + ", minYAc = " + minYAc);
        log("xDcThred = " + xDcThred + ", yDcThred = " + yDcThred + ", isBadStep = " + isBadStep);

        maxXAc = UNDEFINED_VALUE;
        minXAc = UNDEFINED_VALUE;
        maxYAc = UNDEFINED_VALUE;
        minYAc = UNDEFINED_VALUE;
    }

    /** Caculate steps with the acceleration data. */
    public static int caculateSteps() {
        int stepCounter = 0;
        int i = 0;
        int lastStepPoint = 0;
        int stepState = STEP_INITIALIZING;
        int dataLen = (xAc.length > yAc.length) ? xAc.length : yAc.length;

        log("dataLen = " + dataLen);

        initAcData();
        for (i=0; i<dataLen; i++) {
            /** Find the max and min value in axis. */
            if (maxXAc == UNDEFINED_VALUE || xAc[i] > maxXAc) {
                maxXAc = xAc[i];
            }
            if (minXAc == UNDEFINED_VALUE || xAc[i] < minXAc) {
                minXAc = xAc[i];
            }
            if (maxYAc == UNDEFINED_VALUE || yAc[i] > maxYAc) {
                maxYAc = yAc[i];
            }
            if (minYAc == UNDEFINED_VALUE || yAc[i] < minYAc) {
                minYAc = yAc[i];
            }

            samCounter++;
            if (samCounter == SAMPLING_NUMBER) {
                resetSamplingData();
            }

            if (isBadStep || i == 0) {
                continue;
            }

            if ((isXAxisMax && xAc[i-1]>xDcThred && xDcThred>xAc[i])
                || (!isXAxisMax && yAc[i-1]>yDcThred && yDcThred>yAc[i])) {
                if ((i-lastStepPoint) < (int)(MIN_STEP_DURATION/DATA_OUTPUT_FREQ)) {
                    log("ignore: lastStepPoint = " + lastStepPoint + ", i = " + i);
                    continue; // Ignore because of step gap is too short.
                } else if ((i-lastStepPoint) > (int)(MAX_STEP_DURATION/DATA_OUTPUT_FREQ)) {
                    stepState = STEP_SEARCHING;
                    lastStepPoint = i;
                } else {
                    if (stepState == STEP_INITIALIZING) {
                        stepState = STEP_SEARCHING;
                    } else if (stepState == STEP_SEARCHING) {
                        stepState = STEP_WORKING;
                        stepCounter = stepCounter + 2;
                    } else {
                        stepCounter = stepCounter + 1;
                    }

                    lastStepPoint = i;
                }
            }
        }

        log("You complete " + stepCounter + " steps.");

        return stepCounter;
    }

    private static void log(String logtxt)
    {
        if(DEBUG)
        {
            System.out.println("StepCounter: "+logtxt+"\n");
        }
    }
}
