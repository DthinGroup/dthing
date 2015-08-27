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

	private String formatlatitude(String value)
	{
		//获取本地纬度数据后，先保留符号位数据，剩余数据处理后交formateDegreeString格式化
		int signPos = value.indexOf('-');
		String sign = (signPos < 0)? "" : "-";
		/**
		 * 无法保证本地纬度数据一定按照ddmm.mmmm的字符格式上传
		 * 对于度未补0的情况, 例如dmm.mmmm或者mm.mmmm格式，也要能正常处理
		 * 为避免formatDegreeString里面做额外的判定，增加代码量，我们为纬度数据
		 * 手动补0, 再经formatDegreeString格式化。
		 */
		String target = "00" + value.substring(signPos + 1); //Avoid dmm.mmmm or mm.mmmm
		//返回服务器的数据为符号位+格式化后的纬度数据
		return sign + formatDegreeString(target, "36010203", 2);
	}

	private String formatlongitude(String value)
	{
		//获取本地经度数据后，先保留符号位数据，剩余数据处理后交formateDegreeString格式化
		int signPos = value.indexOf('-');
		String sign = (signPos < 0)? "" : "-";
		/**
		 * 无法保证本地经度数据一定按照dddmm.mmmm的字符格式上传
		 * 对于度未补0的情况, 例如ddmm.mmmm或者dmm.mmmm或者mm.mmmm格式，也要能正常处理
		 * 为避免formatDegreeString里面做额外的判定，增加代码量，我们为经度数据
		 * 手动补0, 再经formatDegreeString格式化。
		 */
		String target = "000" + value.substring(signPos + 1); //Avoid dmm.mmmm or mm.mmmm
		//返回服务器的数据为符号位+格式化后的经度数据
		return sign + formatDegreeString(target, "100999897", 3);
	}

	/**
	 * @description format gps data like longitude or latitude
	 *     longitude: dddmm.mmmm -> (ddd + mm.mmmm / 60) * 1000000
	 *     latitude: ddmm.mmmm -> (dd + mm.mmmm / 60) * 1000000
	 * @param value String gps data with degree-minute format
	 * @param defaultValue String default value when failed to format
	 * @param degLen int length of degree
	 * @return formatted degree string without sign
	 */
	private String formatDegreeString(String value, String defaultValue, int degLen)
	{
		//先找到".", 即可确认度分字符串的位置
		int separator = value.indexOf('.');

		//异常数据检查
		if (separator < (2 + degLen))
		{
			if (separator < 0)
			{
				//比0小表明数据不包含".", 可能是ddmm或dddmm格式, 故分隔符位置在最后
				//ddmm or dddmm
				separator = value.length();
			}
			else
			{
				//其他异常情况, 返回默认值
				//other illegal data
				return defaultValue;
			}
		}

		//度的数据为分数据前degLen位到分数据之间的字符串
		String deg = value.substring(separator - 2 - degLen, separator - 2);
		//分的数据为分隔符前两位到分隔符之间的字符串
		String min = value.substring(separator - 2);
		//数据化分字符串数据并保留小数点后6位
		double dmin = Double.valueOf(min).doubleValue() * 1000000D / 60D;
		min = new Double(dmin).toString().substring(0, 6);
		//合并度数据和分数据并返回
		return deg + min;
	}

    public String getTimeInfo() {
        return (gpsTime != null)? gpsTime : "0";
    }

    public String getDateInfo() {
        return (gpsDate != null)? gpsDate : "0";
    }

    public String getLatiInfo() {
        return (gpsLati != null)? formatlatitude(gpsLati) : "39902678";
    }

    public String getLongtiInfo() {
        return (gpsLongti != null)? formatlongitude(gpsLongti) : "116357822";
    }

    public String getAltiInfo() {
        return (gpsAlti != null)? gpsAlti : "0";
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
