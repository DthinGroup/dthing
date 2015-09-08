public class GPSParser {
    private int mLatitude = 0;
    private int mLongitude = 0;

    private String gpsTime = null;
    private String gpsDate = null;
    private String gpsAlti = null;
    private String gpsLongitudeSign = "";
    private String gpsLatitudeSign = "";

    private static boolean DEBUG = false;
    private static boolean waitingMode = false; //When not get enough data to parse
    private static byte[] rawBuf = new byte[256];
    private static int rawBufLen = 0;

    public GPSParser () {
    }

    public void parseGPRMC() {
        int lastSeparator = 0;
        int currentSeparator = 0;
        boolean parseMode = false; //When start with GPRMC, can be parsed
        boolean available = false; //When item2 is A, available for longitude and latitude
        int itemCount = 0;

        for (int i = 0; i < rawBufLen; i++) {
            if (rawBuf[i] == '$') {
                if ((rawBuf[i + 1] == 'G') && (rawBuf[i + 2] == 'P') && (rawBuf[i + 3] == 'R')
                    && (rawBuf[i + 4] == 'M') && (rawBuf[i + 5] == 'C')) {
                    lastSeparator = i;
                    currentSeparator = i;
                    itemCount = 0;
                    available = false;
                    parseMode = true;
                    continue;
                }
            }

            if (!parseMode) continue;

            if (rawBuf[i] == ',') {
                lastSeparator = currentSeparator;
                currentSeparator = i;
                switch(itemCount) {
                case 1: //time
                    if (currentSeparator > (lastSeparator + 9)) {
                        gpsTime = "" + digit2int(rawBuf[lastSeparator + 1]) + digit2int(rawBuf[lastSeparator + 2])
                            + digit2int(rawBuf[lastSeparator + 3]) + digit2int(rawBuf[lastSeparator + 4])
                            + digit2int(rawBuf[lastSeparator + 5]) + digit2int(rawBuf[lastSeparator + 6]);
                    }
                    log(lastSeparator + ":" + currentSeparator + "@time:" +gpsTime);
                    break;
                case 2: //is gps data available
                    if (currentSeparator > (lastSeparator + 1)) {
                        if (rawBuf[i - 1] == 'A') {
                            available = true;
                        } else {
                            available = false;
                        }
                        log(lastSeparator + ":" + currentSeparator + "@A:" +available);
                    }
                    break;
                case 3: //latitude
                    if (!available) continue;
                    if (currentSeparator > (lastSeparator + 9)) {
                        int deg = (digit2int(rawBuf[lastSeparator + 1]) * 10 + digit2int(rawBuf[lastSeparator + 2])) * 1000000;
                        int min = (digit2int(rawBuf[lastSeparator + 3]) * 10 + digit2int(rawBuf[lastSeparator + 4])) * 1000000;
                        int sec = digit2int(rawBuf[lastSeparator + 6]) * 100000 + digit2int(rawBuf[lastSeparator + 7]) * 10000
                            + digit2int(rawBuf[lastSeparator + 8]) * 100000 + digit2int(rawBuf[lastSeparator + 9]) * 100000;
                        mLatitude = deg + (min + sec) / 60;
                    }
                    log(lastSeparator + ":" + currentSeparator + "@latitude:" + mLatitude);
                    break;
                case 4: //latitude direction
                    if (!available) continue;
                    if (currentSeparator > (lastSeparator + 1)) {
                        if (rawBuf[i - 1] == 'N') {
                            gpsLatitudeSign = "";
                        } else {
                            gpsLatitudeSign = "-";
                        }
                    }
                    log(lastSeparator + ":" + currentSeparator + "@gpsLatitudeSign:" + gpsLatitudeSign);
                    break;
                case 5: //longitude
                    if (!available) continue;
                    if (currentSeparator > (lastSeparator + 10)) {
                        int deg = (digit2int(rawBuf[lastSeparator + 1]) * 100 + digit2int(rawBuf[lastSeparator + 2]) * 10
                            + digit2int(rawBuf[lastSeparator + 3])) * 1000000;
                        int min = (digit2int(rawBuf[lastSeparator + 4]) * 10 + digit2int(rawBuf[lastSeparator + 5])) * 1000000;
                        int sec = digit2int(rawBuf[lastSeparator + 7]) * 100000 + digit2int(rawBuf[lastSeparator + 8]) * 10000
                            + digit2int(rawBuf[lastSeparator + 9]) * 100000 + digit2int(rawBuf[lastSeparator + 10]) * 100000;
                        mLongitude = deg + (min + sec) / 60;
                    }
                    log(lastSeparator + ":" + currentSeparator + "@longitude:" + mLongitude);
                    break;
                case 6: //longitude direction
                    if (!available) continue;
                    if (currentSeparator > (lastSeparator + 1)) {
                        if (rawBuf[i - 1] == 'E') {
                            gpsLongitudeSign = "";
                        } else {
                            gpsLongitudeSign = "-";
                        }
                    }
                    log(lastSeparator + ":" + currentSeparator + "@gpsLongitudeSign:" + gpsLongitudeSign);
                    break;
                case 9: //date
                    if (currentSeparator > (lastSeparator + 6)) {
                        gpsDate = "" + digit2int(rawBuf[lastSeparator + 5]) + digit2int(rawBuf[lastSeparator + 6])
                            + digit2int(rawBuf[lastSeparator + 3]) + digit2int(rawBuf[lastSeparator + 4])
                            + digit2int(rawBuf[lastSeparator + 1]) + digit2int(rawBuf[lastSeparator + 2]);
                    }
                    parseMode = false;
                    log(lastSeparator + ":" + currentSeparator + "@gpsDate:" + gpsDate);
                    break;
                default:
                    break;
                }
                itemCount++;
            }
        }
    }

    private int digit2int(byte digit) {
        int result = 0;
        if ((digit >= '0') && (digit <= '9'))
        {
            result = digit - '0';
        }
        return result;
    }

    public void save(byte[] buf, int len) {
        boolean isDataReady = false;

        for (int i = 0; i < len; i++) {
            if (buf[i] == '$') {
                if (waitingMode) {
                    waitingMode = false;
                    isDataReady = true;
                    break;
                } else {
                    if ((buf[i + 1] == 'G') && (buf[i + 2] == 'P') && (buf[i + 3] == 'R')
                        && (buf[i + 4] == 'M') && (buf[i + 5] == 'C')) {
                        waitingMode = true;
                    }
                }
            }

            if (waitingMode) {
                rawBuf[rawBufLen] = buf[i];
                rawBufLen++;
            }
        }

        if (isDataReady) {
            log("start parse");
            parseGPRMC();
            isDataReady = false;
            rawBufLen = 0;
        }
    }

    private static void log(String l) {
        if (DEBUG) {
            System.out.println(l);
        }
    }

    public String getTimeInfo() {
        return (gpsTime != null)? gpsTime : "000000";
    }

    public String getDateInfo() {
        return (gpsDate != null)? ("20" + gpsDate) : "20150901";
    }

    public String getLatiInfo() {
        String result = (mLatitude != 0)? (gpsLatitudeSign + mLatitude) : "39902678";
        return result;
    }

    public String getLongtiInfo() {
        String result = (mLongitude != 0)? (gpsLongitudeSign + mLongitude) : "116357822";
        return result;
    }

    public String getAltiInfo() {
        return (gpsAlti != null)? gpsAlti : "0";
    }
}



