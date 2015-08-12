public class GPSParser {
    private String gpsTime;
    private String gpsDate;
    private String gpsLati;
    private String gpsLongti;
    private String gpsAlti;

    private static final int MIN_TIME_LEN = 7;
    private static final int MIN_DATE_LEN = 6;
    private static final int MIN_LATI_LEN = 5;
    private static final int MIN_LONGTI_LEN = 6;

    private static boolean DEBUG = true;
    private String buffer = "";

    String[] nmeaNames = new String[] {
        "$GPGSA",
        "$GPGSV",
        "$GPGGA",
        "$GPRMC",
        "$GPVTG",
        "$GPGLL",
        "$GPZDA"
    };

    public GPSParser () {
    }

    public void save(String rawInfo) {
    	buffer = buffer + rawInfo;
    	int index = buffer.indexOf("$GPRMC");
    	int nextsection = buffer.indexOf("$", index + 1);
    	
    	if ((index >= 0) && (nextsection >= 0)) {
            startParseNmea(rawInfo);
            buffer = "";
    	}
    }

    private static void log(String l) {
        if (DEBUG) {
            System.out.println(l);
        }
    }

    private void printGpsInfo() {
        log("gpsTime = " + gpsTime);
        log("gpsDate = " + gpsDate);
        log("gpsLati = " + gpsLati);
        log("gpsLongti = " + gpsLongti);
    }

    private void initGpsInfo() {
        gpsTime = null;
        gpsDate = null;
        gpsLati = null;
        gpsLongti = null;
        gpsAlti = null;
    }
    
    private static String[] trimto(String s[],int i)
    {
        if(s.length==i) return s;
        
        String res[]=new String[i];
        for(i--;i>=0;i--) res[i]=s[i];
        return res;
    }
    
    /**
     * Splits a string into sections according to a separating character.
     *
     * @param st The string to split into multiple substrings.
     * @param sep The character to use as a separation point.
     * @param multiple True if we should combine multiple <code>sep</code>
     * characters into a single separation token. If not, there will
     * be blank strings between these characters.
     *
     * If sep = ';' the string ';this;is;;a;;test;' will be split into:
     *
     * multiple=true: "this", "is", "a", "test".
     * multiple=false: "", "this", "is", "", "a", "", "test", "".
     */
    private String[] toSplit(String st,char sep,boolean multiple)
    {
        int i=0;
        int start=0;
        int pos=0;
        String res[]=new String[10];
        while(pos>=0)
        {
            if(multiple)
            {
                do{
                  pos=st.indexOf(sep,start);
                  if(pos==start) start++;
                }
                while(pos<start && pos>=0);
                
                if(pos<0 && start==st.length()) break;
            }
            else pos=st.indexOf(sep,start);
            
            if(i==res.length)
            {
                String r2[]=new String[res.length+10];
                System.arraycopy(res,0,r2,0,res.length);
                res=r2;
            }
            
            res[i++]=st.substring(start,(pos<0?st.length():pos));
            start=pos+1;
        }
        return trimto(res,i);
    }

    private NmeaInfo extractNmeaInfo(String rawInfo, String name, int start) {
        int curt = 0;
        int end = 0;
        int begin = rawInfo.indexOf(name, start);

        log("\nextract nmea items from rawInfo.");

        if (begin >= 0) {
            String extInfo = null;
            for (int i = 0; i < nmeaNames.length; i++) {
                if (!nmeaNames[i].equals(name)) {
                    curt = rawInfo.indexOf(nmeaNames[i], begin);
                    if ((curt > begin) && (end == 0 || curt < end)) {
                        end = curt;
                    }
                }
            }

            if (begin < end) {
                extInfo = rawInfo.substring(begin, end);
            } else {
                extInfo = rawInfo.substring(begin);
                end = begin + extInfo.length();
            }

            log("rawInfo(begin:" + start + ", end:" + end + ") is "  + name +
                " tag content: " + extInfo);

            return new NmeaInfo(end, toSplit(extInfo, ',', true));
        } else {
            log("rawInfo(begin:" + start + ") doesn't contain " + name + " tag \n");
            return null;
        }
    }

     public void startParseNmea(String gpsInfo){
        int start = 0;
        NmeaInfo nmeaInfo;

        log("start parsing gps information.");

        initGpsInfo();

        while ((nmeaInfo = extractNmeaInfo(gpsInfo, "$GPRMC", start)) != null) {
            String[] items = nmeaInfo.items;
            start = nmeaInfo.endPos;
            int len = items.length;

            log("items length = " + len);

            /*for (int i = 0 ; i < len; i++) {
                log(items[i]);
            }*/

            // read time info.
            if (gpsTime == null && len > 2 && items[1].length() >= MIN_TIME_LEN) {
                gpsTime = items[1];
            }

            // read latitude info.
            if (gpsLati == null && len > 4 && items[3].length() >= MIN_LATI_LEN) {
                gpsLati = items[3];
            }

            // read longitude info.
            if (gpsLongti == null && len > 6 && items[5].length() >= MIN_LONGTI_LEN) {
                gpsLongti = items[5];
            }

            // read date info.
            if (gpsDate == null && len > 10 && items[9].length() >= MIN_DATE_LEN) {
                gpsDate = items[9];
            }

            // continue reading or not?
            if (gpsTime == null && gpsLati != null &&
                gpsLongti != null && gpsDate != null) {
                break;
            }
        }

        printGpsInfo();

        start = 0;
        while ((nmeaInfo = extractNmeaInfo(gpsInfo, "$GPGGA", start)) != null) {
            String[] items = nmeaInfo.items;
            int len = items.length;
            start = nmeaInfo.endPos;
            log("items length = " + len);

            // read time info.
            if (gpsTime == null && len > 2 && items[1].length() >= MIN_TIME_LEN) {
                gpsTime = items[1];
            }

            // read latitude info.
            if (gpsLati == null && len > 3 && items[2].length() >= MIN_LATI_LEN) {
                gpsLati = items[2];
            }

            // read longitude info.
            if (gpsLongti == null && len > 5 && items[4].length() >= MIN_LONGTI_LEN) {
                gpsLongti = items[4];
            }

            // get altitude info from $GPGGA tag.
            if (len > 10 && items[9].length() >= 2) {
                gpsAlti = items[9];
            }

            log("gpsAlti = " + gpsAlti);

            // continue reading or not?
            if (gpsTime == null && gpsLati != null &&
                gpsLongti != null && gpsDate != null && gpsAlti != null) {
                break;
            }
        }
    }

    public String getTimeInfo() {
        return gpsTime;
    }

    public String getDateInfo() {
        return gpsDate;
    }

    public String getLatiInfo() {
        return gpsLati;
    }

    public String getLongtiInfo() {
        return gpsLongti;
    }

    public String getAltiInfo() {
        return gpsAlti;
    }

    class NmeaInfo{
        public int endPos;
        public String[] items;

        public NmeaInfo() {
        }

        public NmeaInfo(int end, String[] items) {
            this.endPos = end;
            this.items = items;
        }
    }
}
