
package com.yarlungsoft.util;

import java.io.UnsupportedEncodingException;
import java.lang.IllegalArgumentException;

/**
 * A few functions which are handy for use with strings.
 * Feel free to add whatever else you see fit.
 *
 * @author mb
 * @version  1.0
 */
public class StringOps
{
  public static final String uppercase="ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  public static final String lowercase="abcdefghijklmnopqrstuvwxyz";
  public static final String digits="0123456789";
  public static final String hexdigits="0123456789ABCDEFabcdef";
  public static final String octdigits="01234567";
  public static final String alpha=uppercase+lowercase;
  public static final String alphanum=alpha+digits;
  public static final String whitespace="\t\n\r ";
  private static final String STANDARD_STRING_ENCODING="ISO8859_1";

  public static String[] split(String st,char sep)
  {
    return split(st,sep,false);
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
  public static String[] split(String st,char sep,boolean multiple)
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
  /**
   * Like split(), but calls String.trim() on each resulting element.
   */
  public static String[] splitAndTrim(String st, char sep, boolean multiple)
  {
    String[] ret=split(st,sep,multiple);
    for (int i=0;i<ret.length;i++) ret[i]=ret[i].trim();
    return ret;
  }
  /**
   * Splits a string into sections according to a separating string.
   *
   */
  public static String[] split(String st,String sep)
  {
    int i=0;
    int start=0;
    int pos=st.indexOf(sep,start);
    String res[]=new String[10];
    while(pos>=0)
    {
      res[i++]=st.substring(start,pos);
      start=pos+sep.length();
      if(i==res.length)
      {
        String r2[]=new String[res.length+10];
        System.arraycopy(res,0,r2,0,res.length);
        res=r2;
      }
      pos=st.indexOf(sep,start);
    }
    res[i++]=st.substring(start,st.length());

    return trimto(res,i);
  }

    public static String[] split(String str, int numOfFields, String delimiter)
    {
        int len = str.length();
        int offset = 0;
        String[] fields = new String[numOfFields];

        int i;
        for (i = 0; i < numOfFields && offset < len; i++) {

            int start = offset;

            if (i == numOfFields - 1) {
                // keep all leftover in the last field
                offset = len;
            } else {
                offset = str.indexOf(delimiter, offset);
                if (offset < 0)
                    offset = len;
            }

            fields[i] = str.substring(start, offset);

            offset += delimiter.length();
        }

        if (offset == len) {
            fields[i] = "";
        }

        return fields;
    }

  public static String join(String sep, String[] strings) {
    StringBuffer ret = new StringBuffer();
    boolean first = true;
    for (int i=0; i < strings.length; ++i) {
      if (strings[i] == null)
        continue;

      if (first)
        first = false;
      else
        ret.append(sep);

      ret.append(strings[i]);
    }
    return ret.toString();
  }

  public static String strip(String in,String chars,boolean valid)
  {
    int inLen = in.length();
    char res[] = new char[inLen];
    int resLen=0;

    for (int i=0; i<inLen; i++) {
      char c = in.charAt(i);
      int t = chars.indexOf(c);
      if ((valid && t>=0) || (!valid && t<0)) {
        res[resLen]=c;
        resLen++;
      }
    }
    return resLen == inLen? in : new String(res,0,resLen);
  }

  private static String[] trimto(String s[],int i)
  {
    if(s.length==i) return s;

    String res[]=new String[i];
    for(i--;i>=0;i--) res[i]=s[i];
    return res;
  }

  public static String padleft(int i,int len,char pad)
  {
    return padleft(Integer.toString(i),len,pad);
  }

  public static String padright(int i,int len,char pad)
  {
    return padright(Integer.toString(i),len,pad);
  }

  public static String padleft(String s,int l,char pad)
  {
    if(s.length()<l)
    {
      StringBuffer sb=new StringBuffer(l);
      l-=s.length();
      while(l>0) { sb.append(pad); l--; }
      sb.append(s);
      s=sb.toString();
    }
    return s;
  }

  public static String padright(String s,int l,char pad)
  {
    if(s.length()<l)
    {
      StringBuffer sb=new StringBuffer(l);
      sb.append(s);
      l-=s.length();
      while(l>0) { sb.append(pad); l--; }
      s=sb.toString();
    }
    return s;
  }

  public static byte []getASCIIZ(String s,boolean allowNull)
  {
    byte res[]=null;
    if(s==null)
    {
      if(allowNull)
      {
        res=new byte[1];
        res[0]=0;
      }
    }
    else
    {
      int i;
      byte b[] = null;
      try {
        b = s.getBytes(STANDARD_STRING_ENCODING);
      } catch (UnsupportedEncodingException e) {
        // Cannot happen, since standard encoding is always supported.
      }
      res=new byte[b.length+1];
      for(i=0;i<b.length;i++) res[i]=b[i];
      res[i]=0;
    }
    return res;
  }

  public static byte []getUTF8(String s,boolean allowNull)
  {
    byte res[]=null;
    if(s==null)
    {
      if(allowNull)
      {
        res=new byte[1];
        res[0]=0;
      }
    }
    else
      res = UnicodeToUTF8Z(s, true);
    return res;
  }

  public static boolean equalsIgnoreAsciiCase(String a,String b)
  {
    int l=a.length();
    char ca;
    char cb;
    if(l!=b.length()) return false;
    for(int i=0;i<l;i++)
    {
      ca=a.charAt(i);
      cb=b.charAt(i);
      if(ca!=cb)
      {
        if(ca>='a' && ca<='z')  ca-=32;
        if(cb>='a' && cb<='z')  cb-=32;
        if(ca!=cb) return false;
      }
    }
    return true;
  }

  public static int evalInt(String s)
  {
    s=strip(s,whitespace,false);

    if (s.startsWith("0x")) {
      return Integer.valueOf(s.substring(2), 16).intValue();
    }

    return Integer.valueOf(s).intValue();
  }

  public static String normalizePath(String s,int offset[]) throws IllegalArgumentException
  {
    int in,out;
    char str[]=s.toCharArray();
    char res[]=new char[str.length];

    boolean normalEntry=false;
    boolean dotDot=false;
    int entryChars=0;

    // If we're given a path that starts with
    int stepBack=0;

    for(in=0,out=0;in<str.length;in++)
    {
      if(str[in]=='/' || str[in]=='\\')
      { // We got a path separator.
        if(out==0)
        {
          res[out]='/';
          out++;
        }
        else
        {
          if(res[out-1]!='/')
          {
            if(normalEntry)
            {
              res[out]='/';
              out++;
            }
            else if(dotDot)
            { // We got a '..' path entry.
              // If we can remove a path entry, do so.
              // Otherwise, if this is a relative path, increment stepBack.
              // If this is an absolute path and we're at the root, barf.
              out-=2;
              if(out==0 || (out==1 && res[0]=='/')) stepBack++;
              else
              {
                // Remove one path entry from the end.
                do { out--; } while(out>0 && res[out-1]!='/');
              }
            }
            else
            { // This was a '.' path entry.
              // We should just ignore it.
              out--;
            }
          }
        }
        entryChars=0;
        normalEntry=false;
        dotDot=false;
      }
      else
      {
        if(!normalEntry)
        {
          entryChars++;
          if(str[in]!='.' || entryChars>2) normalEntry=true;

          if(entryChars==2) dotDot=true;
        }
        res[out++]=str[in];
      }
    }
    if(offset==null)
    {
      if(stepBack!=0) throw new IllegalArgumentException();
    }
    else offset[0]=stepBack;
    return new String(res,0,out);
  }

  /**
   * UTF-8 encoded string, see http://www.ietf.org/rfc/rfc2279.txt:
   * 0x0000 - 0x007F 0xxxxxxx binary
   * 0x0080 - 0x07FF 110xxxxx 10xxxxxx binary
   * 0x0800 - 0xFFFF 1110xxxx 10xxxxxx 10xxxxxx binary, but surrogate pairs 0xD800 - 0xDFFF not handled
   */
  private static byte[] UnicodeToUTF8Z (String s, boolean addZero) {
    /* determine utf8 string length */
    final int sl = s.length();
    char c;
    int j = 0;
    for (int i = 0; i < sl; i++) {
      c = s.charAt(i);
      if (c < 0x80) {
        j++;
      } else if (c < 0x800) {
        j += 2;
      } else {
        j += 3;
      }
    }
    if (addZero) j++;
    byte[] u = new byte[j];
    j = 0;
    for (int i = 0; i < sl; i++) {
      c = s.charAt(i);
      if (c < 0x80) {
        u[j] = (byte) c; j++;
      } else if (c < 0x800) {
        u[j] = (byte) (c >>> 6 | 0xC0); j++;
        u[j] = (byte) (c & 0x3F | 0x80); j++;
      } else {
        u[j] = (byte) (c >>> 12  | 0xE0); j++;
        u[j] = (byte) ((c >>> 6) & 0x3F | 0x80); j++;
        u[j] = (byte) (c & 0x3F | 0x80); j++;
      }
    }
    if (addZero) u[j] = 0;
    return u;
  }

  /**
   * Deprecated.  Convert a String to UTF8.
   *
   * @param s A string to convert.
   * @return The UTF8-encoded string (not null-terminated).
   *
   * @see getUTF8 for a version with null-termination.
   *
   * @deprecated Use String.getBytes("UTF8") instead.
   */
  public static byte[] UnicodeToUTF8 (String s) {
    return UnicodeToUTF8Z(s, false);
  }

  /**
   * Convert a UTF8 byte[] array (not null-terminated) to a String.
   *
   * @param u The UTF8 data.
   * @param off The offset to start the conversion.
   * @param len The number of bytes to convert (do not include null terminator).
   * @return The converted string.
   *
   * @see getStringFromUTF8 for a version that uses null-termination.
   *
   * See also new String(u, off, len, "UTF8").
   */
  public static String UTF8ToUnicode (byte[] u, int off, int len) throws IllegalArgumentException{

    /* 'len' is upper limit on the number of characters required.
     * String constructor will copy the array anyway, so allocating a few
     * extra now saves an extra pass to make an exact count.
     */
    char[] s = new char[len];

    len += off;  // convert relative to absolute
    int i = off; // input iterator
    int j = 0;   // output iterator
    while (i < len) {
      int b = u[i];
      if ((b & 0x80) == 0) {
        s[j] = (char) b; i++;
      } else if ((b & 0xe0) == 0xc0) {
        s[j] = (char) (((b & 0x1F) << 6) + (u[i + 1] & 0x3F)); i += 2;
      } else if ((b & 0xf0) == 0xe0) {
        s[j] = (char) (((b & 0xF) << 12) + ((u[i + 1] & 0x3F) << 6) + (u[i + 2] & 0x3F)); i += 3;
      } else {
        throw new IllegalArgumentException("Bad UTF-8 code");
      }
      j++;
    }

    /* *FIXME* consider using PackageAccess to give internal access to
     * fdac private String(int offset, int count, char value[]) which
     * uses the array directly rather than copying it.
     */
    return new String(s, 0, j);
  }

  /**
   * Convert a String[] to String by printing the content.
   *
   * @param arr the String[] to convert
   * @return the content of arr as one String.
   */
  public static String toString(String[] arr) {

    if (arr == null)
      return "null";

    if (arr.length == 0)
      return "{0}";

    StringBuffer buf = new StringBuffer("{");
    buf.append(arr.length);
    buf.append(':');
    buf.append(arr[0]);
    for (int i = 1; i < arr.length; ++i) {
      buf.append(",");
      buf.append(arr[i]);
    }
    buf.append("}");
    return buf.toString();
  }

  /**
   * Convert '%' and two following hex characters to equivalent byte value.
   * @param s String encoded string.
   * @return String Decoded string.
   */
  public static String decode(String s)
  {
      StringBuffer result = new StringBuffer(s.length());
      for (int i = 0; i < s.length();) {
          char c = s.charAt(i);
          if (c == '%') {
              do {
                  if (i + 2 >= s.length())
                      throw new IllegalArgumentException("Incomplete URI escape sequence in ");
                      int d1 = Character.digit(s.charAt(i+1), 16);
                      int d2 = Character.digit(s.charAt(i+2), 16);
                      if (d1 == -1 || d2 == -1) {
                          throw new IllegalArgumentException(s);
                      }
                      result.append((char) ((d1 << 4) + d2));
                  i += 3;
              } while (i < s.length() && s.charAt(i) == '%');
              continue;
          }
          result.append(c);
          i++;
      }
      return result.toString();
  }

  /**
   * Check if the given string is a number.
   *
   * @param s the given string
   * @return true if the given string is a number, false otherwise.
   */
  public static boolean isNumber(String s)
  {
      try {
          Integer.parseInt(s);
          return true;
      }
      catch (NumberFormatException nfe) {
          return false;
      }
  }

    /**
     * Check whether the <code>str</code> could be matched by the <code>ptn
     * </code>.
     * In the <code>ptn</code>, an asterisk may appear by itself, or if
     * immediately preceded by a "." may appear at the end of the name,
     * to signify a wildcard match.
     *
     * @param ptn the pattern to match with
     * @param str the string to be matched
     * @return true if the <code>str</code> could be matched with the <code>
     * ptn</code>, false otherwise
     */
    public static boolean match(String ptn, String str) {
        if (ptn.equals(str) || ptn.equals("*"))
            return true;

        int pIdx, sIdx;
        int pLen = ptn.length();
        int sLen = str.length();
        int pPos = 0;
        int sPos = 0;

        while (pPos < pLen) {
            String tmp;
            pIdx = ptn.indexOf('.', pPos);
            if (pIdx == -1) {
                tmp = ptn.substring(pPos);

                return tmp.equals("*") || tmp.equals(str.substring(sPos));
            }

            tmp = ptn.substring(pPos, pIdx);
            if (tmp.equals("*") && pPos+1 == pLen)
                return true;

            sIdx = str.indexOf('.', sPos);

            if (sIdx == -1 || !tmp.equals(str.substring(sPos, sIdx)))
                return false;

            pPos = pIdx + 1;
            sPos = sIdx + 1;
        }

        return pPos == pLen && sPos == sLen;
    }

}
