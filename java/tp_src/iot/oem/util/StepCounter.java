package iot.oem.util;

/**
 * This class is used for calculating steps through acceleration data,
 * which supports both two axis accelerometer and three axis accelerometer.
 * According to the constructor of StepCounter, there are below tow ways of using StepCounter class:
 *   1) StepCounter counter = new StepCounter(xInAc, yInAc, zInAc);
 *      counter.getStepCount(); //get result.
 *   2) StepCounter counter = new StepCounter();
 *      counter.caculateSteps(xInAc, yInAc, zInAc); //get result.
 */
public class StepCounter {
    private boolean DEBUG = true;
    private boolean isBadStep = false;

    private static final double UNDEFINED_VALUE = -10.0;
    private static final int X_AXIS_MAX = 0;
    private static final int Y_AXIS_MAX = 1;
    private static final int Z_AXIS_MAX = 2;
    private static final double MIN_VALID_STEP_GAP = 0.3;
    private static final double MIN_STEP_DURATION = 0.2;
    private static final double MAX_STEP_DURATION = 2;
    private double minXAc = UNDEFINED_VALUE;
    private double minYAc = UNDEFINED_VALUE;
    private double minZAc = UNDEFINED_VALUE;
    private double maxXAc = UNDEFINED_VALUE;
    private double maxYAc = UNDEFINED_VALUE;
    private double maxZAc = UNDEFINED_VALUE;

    /** Whose axis has max data. */
    private int maxAxis = 0;
    /** The frequency of data output. */
    private final double DATA_OUTPUT_FREQ = 0.05;
    /** Caculate threshold dynamically every 10 output data. */
    private final int SAMPLING_NUMBER = 10;
    /** sampling counter, reset to 0 every SAMPLING_NUMBER times. */
    private int samCounter = 0;
    /** x axis dynamic threshold. */
    private double xDcThred = UNDEFINED_VALUE;
    /** y axis dynamic threshold. */
    private double yDcThred = UNDEFINED_VALUE;
    /** z axis dynamic threshold. */
    private double zDcThred = UNDEFINED_VALUE;
    /** The step count. */
    private int stepCount = 0;
    /** Is three axis accelerometer. */
    private boolean isThreeAxisAcc = false;

    /** The state of calculating step. */
    private static final int STEP_INITIALIZING = 0;
    private static final int STEP_SEARCHING = 1;
    private static final int STEP_WORKING = 2;

    /** The test acceleration data of x, y and z axis.
    private static double xInAc[] = {-0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49 };
    private static double yInAc[] = {-1.00, -0.80, -0.63, -4.65, -2.64, -0.18, -0.28, -0.25, -0.20, -0.23,
                                   -0.32, -0.40, -0.58, -0.63, -0.60, -0.50, -0.40, -0.25, -0.12, -0.10,
                                   -0.15, -0.22, -0.31, -0.49, -0.60, -0.51, -0.40, -0.31, -0.21, -0.16,
                                   -0.22, -0.38, -0.45, -0.50, -0.62, -0.56, -0.43, -0.32, -0.19, -0.18,
                                   -0.15, -0.22, -0.31, -0.49, -0.60, -0.51, -0.40, -0.31, -0.21, -0.16,
                                   -0.22, -0.38, -0.45, -0.50, -0.62, -0.56, -0.43, -0.32, -0.19, -0.18,
                                   -0.15, -0.22, -0.31, -0.49, -0.60, -0.51, -0.40, -0.31, -0.21, -0.16,
                                   -0.22, -0.38, -0.45, -0.50, -0.62, -0.56, -0.43, -0.32, -0.19, -0.18,
                                   -0.15, -0.22, -0.31, -0.49, -0.60, -0.51, -0.40, -0.31, -0.21, -0.16,
                                   -0.22, -0.38, -0.45, -0.50, -0.62, -0.56, -0.43, -0.32, -0.19, -0.18 };
    private static double zInAc[] = {-0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49,
                                   -0.50, -0.49, -0.52, -0.47, -0.50, -0.49, -0.52, -0.47, -0.50, -0.49 }; */
    /**
     * The constructor of StepCounter with no argument.
     * In this way, you need to speccify the data of axises acceleration at caculateSteps.
     */
    public StepCounter() {
        initAcData();
    }

    /**
     * The constructor of StepCounter with the data of axises acceleration.
     * When you specify zInAc as non-null value, it means three axis accelerometer.
     * When you specify zInAc as non-null value, it means three axis accelerometer.
     */
    public StepCounter(double xInAc[], double yInAc[], double zInAc[]) {
        initAcData();
        caculateSteps(xInAc, yInAc, zInAc);
    }

    /** Initialize data. */
    private void initAcData() {
        samCounter = 0;
        stepCount = 0;
        minYAc = UNDEFINED_VALUE;
        minXAc = UNDEFINED_VALUE;
        minZAc = UNDEFINED_VALUE;
        maxXAc = UNDEFINED_VALUE;
        maxYAc = UNDEFINED_VALUE;
        maxZAc = UNDEFINED_VALUE;
        xDcThred = UNDEFINED_VALUE;
        yDcThred = UNDEFINED_VALUE;
        zDcThred = UNDEFINED_VALUE;
    }

    /** Reset data for another cycle of sampling. */
    private void resetSamplingData() {
        samCounter = 0;
        /** Caculate dynamic threshold value. */
        xDcThred = (maxXAc+minXAc)/2;
        yDcThred = (maxYAc+minYAc)/2;
        if (isThreeAxisAcc) {
            zDcThred = (maxZAc+minZAc)/2;
        }

        if (maxXAc-minXAc >= MIN_VALID_STEP_GAP
            || maxYAc-minYAc >= MIN_VALID_STEP_GAP
            || (isThreeAxisAcc && maxZAc-minZAc >= MIN_VALID_STEP_GAP)) {
            isBadStep = false;
            /** Find the axis whose acceleration changes is the largest. */
            if (isThreeAxisAcc) {
                if (maxXAc-minXAc > maxYAc-minYAc) {
                    if (maxXAc-minXAc > maxZAc-minZAc) {
                        maxAxis = X_AXIS_MAX;
                    } else {
                        maxAxis = Z_AXIS_MAX;
                    }
                } else if (maxYAc-minYAc > maxZAc-minZAc) {
                    maxAxis = Y_AXIS_MAX;
                } else {
                    maxAxis = Z_AXIS_MAX;
                }
            } else {
                maxAxis = (maxXAc-minXAc > maxYAc-minYAc) ? X_AXIS_MAX : Y_AXIS_MAX;
            }
        } else {
            isBadStep = true;
        }

        log("maxXAc = " + maxXAc + ",minXAc = " + minXAc
            + " maxYAc = " + maxYAc + ", minYAc = " + minYAc
            + (isThreeAxisAcc ? (" maxZAc = " + maxZAc + ", minZAc = " + minZAc) : ""));
        log("xDcThred = " + xDcThred + ", yDcThred = " + yDcThred
            + (isThreeAxisAcc ? (", zDcThred = " + zDcThred) : "")
            + ", isBadStep = " + isBadStep + ", maxAxis = " + maxAxis);

        minXAc = UNDEFINED_VALUE;
        minYAc = UNDEFINED_VALUE;
        minZAc = UNDEFINED_VALUE;
        maxXAc = UNDEFINED_VALUE;
        maxYAc = UNDEFINED_VALUE;
        maxZAc = UNDEFINED_VALUE;
    }

    /**
     * Start Caculating steps with the acceleration data.
     * When you specify zInAc as non-null value, it means three axis accelerometer.
     * When you specify zInAc as non-null value, it means three axis accelerometer.
     */
    public int caculateSteps(double xAc[], double yAc[], double zAc[]) {
        int i = 0;
        int lastStepPoint = 0;
        int stepState = STEP_INITIALIZING;
        int dataLen = 0;

        if (zAc != null && zAc.length > 0) {
            log("This is a three axis accelerometer!");
            isThreeAxisAcc = true;
        } else { // only two axis accelerometer
            log("This is a two axis accelerometer!");
            isThreeAxisAcc = false;
        }

        if (xAc.length > yAc.length) {
            dataLen = !isThreeAxisAcc ? xAc.length : ((zAc.length > yAc.length) ? yAc.length : zAc.length);
        } else {
            dataLen = !isThreeAxisAcc ? yAc.length : ((zAc.length > xAc.length) ? xAc.length : zAc.length);
        }

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
            if (isThreeAxisAcc && (maxZAc == UNDEFINED_VALUE || zAc[i] > maxZAc)) {
                maxZAc = zAc[i];
            }
            if (isThreeAxisAcc && (minZAc == UNDEFINED_VALUE || zAc[i] < minZAc)) {
                minZAc = zAc[i];
            }

            samCounter++;
            /** Caculate threshold value and reset sampling data. */
            if (samCounter == SAMPLING_NUMBER) {
                resetSamplingData();
            }

            if (isBadStep || i == 0) {
                continue;
            }

            /** Calutlating steps.  */
            if ((maxAxis==X_AXIS_MAX && xAc[i-1]>xDcThred && xDcThred>xAc[i])
                || (maxAxis==Y_AXIS_MAX && yAc[i-1]>yDcThred && yDcThred>yAc[i])
                || (isThreeAxisAcc && maxAxis == Z_AXIS_MAX && zAc[i-1]>zDcThred && zDcThred>zAc[i])) {
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
                        stepCount = stepCount + 2;
                    } else {
                        stepCount = stepCount + 1;
                    }

                    lastStepPoint = i;
                }
            }
        }

        log("You complete " + stepCount + " steps.");

        return stepCount;
    }

    public int getStepCount() {
        return stepCount;
    }

    private void log(String logtxt)
    {
        if(DEBUG)
        {
            System.out.println("StepCounter: "+logtxt+"\n");
        }
    }

    /*public static void main(String[] args) {
        StepCounter counter = new StepCounter(xInAc, yInAc, zInAc);
        System.out.println("The step count is " + counter.getStepCount());
        //StepCounter counter = new StepCounter();
        //System.out.println("The step count is " + counter.caculateSteps(xInAc, yInAc, zInAc));
    }*/
}
