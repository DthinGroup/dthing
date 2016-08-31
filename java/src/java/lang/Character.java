/*
 *  Licensed to the Apache Software Foundation (ASF) under one or more
 *  contributor license agreements.  See the NOTICE file distributed with
 *  this work for additional information regarding copyright ownership.
 *  The ASF licenses this file to You under the Apache License, Version 2.0
 *  (the "License"); you may not use this file except in compliance with
 *  the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

package java.lang;


/**
 * The wrapper for the primitive type {@code char}. This class also provides a
 * number of utility methods for working with characters.
 *
 * <p>Character data is kept up to date as Unicode evolves.
 * See the <a href="../util/Locale.html#locale_data">Locale data</a> section of
 * the {@code Locale} documentation for details of the Unicode versions implemented by current
 * and historical Android releases.
 *
 * <p>The Unicode specification, character tables, and other information are available at
 * <a href="http://www.unicode.org/">http://www.unicode.org/</a>.
 *
 * <p>Unicode characters are referred to as <i>code points</i>. The range of valid
 * code points is U+0000 to U+10FFFF. The <i>Basic Multilingual Plane (BMP)</i>
 * is the code point range U+0000 to U+FFFF. Characters above the BMP are
 * referred to as <i>Supplementary Characters</i>. On the Java platform, UTF-16
 * encoding and {@code char} pairs are used to represent code points in the
 * supplementary range. A pair of {@code char} values that represent a
 * supplementary character are made up of a <i>high surrogate</i> with a value
 * range of 0xD800 to 0xDBFF and a <i>low surrogate</i> with a value range of
 * 0xDC00 to 0xDFFF.
 * <p>
 * On the Java platform a {@code char} value represents either a single BMP code
 * point or a UTF-16 unit that's part of a surrogate pair. The {@code int} type
 * is used to represent all Unicode code points.
 *
 * <a name="unicode_categories"><h3>Unicode categories</h3></a>
 * <p>Here's a list of the Unicode character categories and the corresponding Java constant,
 * grouped semantically to provide a convenient overview. This table is also useful in
 * conjunction with {@code \p} and {@code \P} in {@link java.util.regex.Pattern regular expressions}.
 * <span class="datatable">
 * <style type="text/css">
 * .datatable td { padding-right: 20px; }
 * </style>
 * <p><table>
 * <tr> <td> Cn </td> <td> Unassigned </td>  <td>{@link #UNASSIGNED}</td> </tr>
 * <tr> <td> Cc </td> <td> Control </td>     <td>{@link #CONTROL}</td> </tr>
 * <tr> <td> Cf </td> <td> Format </td>      <td>{@link #FORMAT}</td> </tr>
 * <tr> <td> Co </td> <td> Private use </td> <td>{@link #PRIVATE_USE}</td> </tr>
 * <tr> <td> Cs </td> <td> Surrogate </td>   <td>{@link #SURROGATE}</td> </tr>
 * <tr> <td><br></td> </tr>
 * <tr> <td> Lu </td> <td> Uppercase letter </td> <td>{@link #UPPERCASE_LETTER}</td> </tr>
 * <tr> <td> Ll </td> <td> Lowercase letter </td> <td>{@link #LOWERCASE_LETTER}</td> </tr>
 * <tr> <td> Lt </td> <td> Titlecase letter </td> <td>{@link #TITLECASE_LETTER}</td> </tr>
 * <tr> <td> Lm </td> <td> Modifier letter </td>  <td>{@link #MODIFIER_LETTER}</td> </tr>
 * <tr> <td> Lo </td> <td> Other letter </td>     <td>{@link #OTHER_LETTER}</td> </tr>
 * <tr> <td><br></td> </tr>
 * <tr> <td> Mn </td> <td> Non-spacing mark </td>       <td>{@link #NON_SPACING_MARK}</td> </tr>
 * <tr> <td> Me </td> <td> Enclosing mark </td>         <td>{@link #ENCLOSING_MARK}</td> </tr>
 * <tr> <td> Mc </td> <td> Combining spacing mark </td> <td>{@link #COMBINING_SPACING_MARK}</td> </tr>
 * <tr> <td><br></td> </tr>
 * <tr> <td> Nd </td> <td> Decimal digit number </td> <td>{@link #DECIMAL_DIGIT_NUMBER}</td> </tr>
 * <tr> <td> Nl </td> <td> Letter number </td>        <td>{@link #LETTER_NUMBER}</td> </tr>
 * <tr> <td> No </td> <td> Other number </td>         <td>{@link #OTHER_NUMBER}</td> </tr>
 * <tr> <td><br></td> </tr>
 * <tr> <td> Pd </td> <td> Dash punctuation </td>          <td>{@link #DASH_PUNCTUATION}</td> </tr>
 * <tr> <td> Ps </td> <td> Start punctuation </td>         <td>{@link #START_PUNCTUATION}</td> </tr>
 * <tr> <td> Pe </td> <td> End punctuation </td>           <td>{@link #END_PUNCTUATION}</td> </tr>
 * <tr> <td> Pc </td> <td> Connector punctuation </td>     <td>{@link #CONNECTOR_PUNCTUATION}</td> </tr>
 * <tr> <td> Pi </td> <td> Initial quote punctuation </td> <td>{@link #INITIAL_QUOTE_PUNCTUATION}</td> </tr>
 * <tr> <td> Pf </td> <td> Final quote punctuation </td>   <td>{@link #FINAL_QUOTE_PUNCTUATION}</td> </tr>
 * <tr> <td> Po </td> <td> Other punctuation </td>         <td>{@link #OTHER_PUNCTUATION}</td> </tr>
 * <tr> <td><br></td> </tr>
 * <tr> <td> Sm </td> <td> Math symbol </td>     <td>{@link #MATH_SYMBOL}</td> </tr>
 * <tr> <td> Sc </td> <td> Currency symbol </td> <td>{@link #CURRENCY_SYMBOL}</td> </tr>
 * <tr> <td> Sk </td> <td> Modifier symbol </td> <td>{@link #MODIFIER_SYMBOL}</td> </tr>
 * <tr> <td> So </td> <td> Other symbol </td>    <td>{@link #OTHER_SYMBOL}</td> </tr>
 * <tr> <td><br></td> </tr>
 * <tr> <td> Zs </td> <td> Space separator </td>     <td>{@link #SPACE_SEPARATOR}</td> </tr>
 * <tr> <td> Zl </td> <td> Line separator </td>      <td>{@link #LINE_SEPARATOR}</td> </tr>
 * <tr> <td> Zp </td> <td> Paragraph separator </td> <td>{@link #PARAGRAPH_SEPARATOR}</td> </tr>
 * </table>
 * </span>
 *
 * @since 1.0
 */
public final class Character {
   
	private static final long serialVersionUID = 3786198910865385080L;
    
    private final char value;
    
	/**
	 * Unicode category constant Cn.
	 */
	public static final byte UNASSIGNED = 0;

	/**
	 * Unicode category constant Lu.
	 */
	public static final byte UPPERCASE_LETTER = 1;
	
	public static final byte OTHER_LETTER = 5;
	
	public static final byte DECIMAL_DIGIT_NUMBER = 9;

    /**
     * The minimum {@code Character} value.
     */
    public static final char MIN_VALUE = '\u0000';

    /**
     * The maximum {@code Character} value.
     */
    public static final char MAX_VALUE = '\uffff';
    
    /**
     * The minimum radix used for conversions between characters and integers.
     */
    public static final int MIN_RADIX = 2;

    /**
     * The maximum radix used for conversions between characters and integers.
     */
    public static final int MAX_RADIX = 36;


	// Unicode 3.0.1 (same as Unicode 3.0.0)
	private static final String typeKeys = "\u0000 \"$&(*-/1:<?A[]_a{}\u007f\u00a0\u00a2\u00a6\u00a8\u00aa\u00ac\u00ae\u00b1\u00b3\u00b5\u00b7\u00b9\u00bb\u00bd\u00bf\u00c1\u00d7\u00d9\u00df\u00f7\u00f9\u0100\u0138\u0149\u0179\u017f\u0181\u0183\u0187\u018a\u018c\u018e\u0192\u0194\u0197\u0199\u019c\u019e\u01a0\u01a7\u01ab\u01af\u01b2\u01b4\u01b8\u01ba\u01bc\u01be\u01c0\u01c4\u01c6\u01c8\u01ca\u01cc\u01dd\u01f0\u01f2\u01f4\u01f7\u01f9\u0222\u0250\u02b0\u02b9\u02bb\u02c2\u02d0\u02d2\u02e0\u02e5\u02ee\u0300\u0360\u0374\u037a\u037e\u0384\u0386\u0389\u038c\u038e\u0390\u0392\u03a3\u03ac\u03d0\u03d2\u03d5\u03da\u03f0\u0400\u0430\u0460\u0482\u0484\u0488\u048c\u04c1\u04c7\u04cb\u04d0\u04f8\u0531\u0559\u055b\u0561\u0589\u0591\u05a3\u05bb\u05be\u05c2\u05d0\u05f0\u05f3\u060c\u061b\u061f\u0621\u0640\u0642\u064b\u0660\u066a\u0670\u0672\u06d4\u06d6\u06dd\u06df\u06e5\u06e7\u06e9\u06eb\u06f0\u06fa\u06fd\u0700\u070f\u0711\u0713\u0730\u0780\u07a6\u0901\u0903\u0905\u093c\u093e\u0941\u0949\u094d\u0950\u0952\u0958\u0962\u0964\u0966\u0970\u0981\u0983\u0985\u098f\u0993\u09aa\u09b2\u09b6\u09bc\u09be\u09c1\u09c7\u09cb\u09cd\u09d7\u09dc\u09df\u09e2\u09e6\u09f0\u09f2\u09f4\u09fa\u0a02\u0a05\u0a0f\u0a13\u0a2a\u0a32\u0a35\u0a38\u0a3c\u0a3e\u0a41\u0a47\u0a4b\u0a59\u0a5e\u0a66\u0a70\u0a72\u0a81\u0a83\u0a85\u0a8d\u0a8f\u0a93\u0aaa\u0ab2\u0ab5\u0abc\u0abe\u0ac1\u0ac7\u0ac9\u0acb\u0acd\u0ad0\u0ae0\u0ae6\u0b01\u0b03\u0b05\u0b0f\u0b13\u0b2a\u0b32\u0b36\u0b3c\u0b3e\u0b42\u0b47\u0b4b\u0b4d\u0b56\u0b5c\u0b5f\u0b66\u0b70\u0b82\u0b85\u0b8e\u0b92\u0b99\u0b9c\u0b9e\u0ba3\u0ba8\u0bae\u0bb7\u0bbe\u0bc0\u0bc2\u0bc6\u0bca\u0bcd\u0bd7\u0be7\u0bf0\u0c01\u0c05\u0c0e\u0c12\u0c2a\u0c35\u0c3e\u0c41\u0c46\u0c4a\u0c55\u0c60\u0c66\u0c82\u0c85\u0c8e\u0c92\u0caa\u0cb5\u0cbe\u0cc1\u0cc6\u0cc8\u0cca\u0ccc\u0cd5\u0cde\u0ce0\u0ce6\u0d02\u0d05\u0d0e\u0d12\u0d2a\u0d3e\u0d41\u0d46\u0d4a\u0d4d\u0d57\u0d60\u0d66\u0d82\u0d85\u0d9a\u0db3\u0dbd\u0dc0\u0dca\u0dcf\u0dd2\u0dd6\u0dd8\u0df2\u0df4\u0e01\u0e31\u0e33\u0e35\u0e3f\u0e41\u0e46\u0e48\u0e4f\u0e51\u0e5a\u0e81\u0e84\u0e87\u0e8a\u0e8d\u0e94\u0e99\u0ea1\u0ea5\u0ea7\u0eaa\u0ead\u0eb1\u0eb3\u0eb5\u0ebb\u0ebd\u0ec0\u0ec6\u0ec8\u0ed0\u0edc\u0f00\u0f02\u0f04\u0f13\u0f18\u0f1a\u0f20\u0f2a\u0f34\u0f3a\u0f3e\u0f40\u0f49\u0f71\u0f7f\u0f81\u0f85\u0f87\u0f89\u0f90\u0f99\u0fbe\u0fc6\u0fc8\u0fcf\u1000\u1023\u1029\u102c\u102e\u1031\u1036\u1038\u1040\u104a\u1050\u1056\u1058\u10a0\u10d0\u10fb\u1100\u115f\u11a8\u1200\u1208\u1248\u124a\u1250\u1258\u125a\u1260\u1288\u128a\u1290\u12b0\u12b2\u12b8\u12c0\u12c2\u12c8\u12d0\u12d8\u12f0\u1310\u1312\u1318\u1320\u1348\u1361\u1369\u1372\u13a0\u1401\u166d\u166f\u1680\u1682\u169b\u16a0\u16eb\u16ee\u1780\u17b4\u17b7\u17be\u17c6\u17c8\u17ca\u17d4\u17db\u17e0\u1800\u1806\u1808\u180b\u1810\u1820\u1843\u1845\u1880\u18a9\u1e00\u1e96\u1ea0\u1f00\u1f08\u1f10\u1f18\u1f20\u1f28\u1f30\u1f38\u1f40\u1f48\u1f50\u1f59\u1f5b\u1f5d\u1f5f\u1f61\u1f68\u1f70\u1f80\u1f88\u1f90\u1f98\u1fa0\u1fa8\u1fb0\u1fb6\u1fb8\u1fbc\u1fbe\u1fc0\u1fc2\u1fc6\u1fc8\u1fcc\u1fce\u1fd0\u1fd6\u1fd8\u1fdd\u1fe0\u1fe8\u1fed\u1ff2\u1ff6\u1ff8\u1ffc\u1ffe\u2000\u200c\u2010\u2016\u2018\u201a\u201c\u201e\u2020\u2028\u202a\u202f\u2031\u2039\u203b\u203f\u2041\u2044\u2046\u2048\u206a\u2070\u2074\u207a\u207d\u207f\u2081\u208a\u208d\u20a0\u20d0\u20dd\u20e1\u20e3\u2100\u2102\u2104\u2107\u2109\u210b\u210e\u2110\u2113\u2115\u2117\u2119\u211e\u2124\u212b\u212e\u2130\u2132\u2134\u2136\u2139\u2153\u2160\u2190\u2195\u219a\u219c\u21a0\u21a2\u21a5\u21a8\u21ae\u21b0\u21ce\u21d0\u21d2\u21d6\u2200\u2300\u2308\u230c\u2320\u2322\u2329\u232b\u237d\u2400\u2440\u2460\u249c\u24ea\u2500\u25a0\u25b7\u25b9\u25c1\u25c3\u2600\u2619\u266f\u2671\u2701\u2706\u270c\u2729\u274d\u274f\u2756\u2758\u2761\u2776\u2794\u2798\u27b1\u2800\u2e80\u2e9b\u2f00\u2ff0\u3000\u3002\u3004\u3006\u3008\u3012\u3014\u301c\u301e\u3020\u3022\u302a\u3030\u3032\u3036\u3038\u303e\u3041\u3099\u309b\u309d\u30a1\u30fb\u30fd\u3105\u3131\u3190\u3192\u3196\u31a0\u3200\u3220\u322a\u3260\u327f\u3281\u328a\u32c0\u32d0\u3300\u337b\u33e0\u3400\u4e00\ua000\ua490\ua4a4\ua4b5\ua4c2\ua4c6\uac00\ud800\ue000\uf900\ufb00\ufb13\ufb1d\ufb20\ufb29\ufb2b\ufb38\ufb3e\ufb40\ufb43\ufb46\ufbd3\ufd3e\ufd50\ufd92\ufdf0\ufe20\ufe30\ufe32\ufe34\ufe36\ufe49\ufe4d\ufe50\ufe54\ufe58\ufe5a\ufe5f\ufe62\ufe65\ufe68\ufe6b\ufe70\ufe74\ufe76\ufeff\uff01\uff04\uff06\uff08\uff0a\uff0d\uff0f\uff11\uff1a\uff1c\uff1f\uff21\uff3b\uff3d\uff3f\uff41\uff5b\uff5d\uff61\uff63\uff65\uff67\uff70\uff72\uff9e\uffa0\uffc2\uffca\uffd2\uffda\uffe0\uffe2\uffe4\uffe6\uffe8\uffea\uffed\ufff9\ufffc";
	private static final char[] typeValues = new StringBuffer("\u001f\u000f!\u180c#\u0018%\u181a'\u0018)\u1615,\u1918.\u14180\u18099\t;\u0018>\u0019@\u0018Z\u0001\\\u1518^\u161b`\u171bz\u0002|\u1519~\u1619\u009f\u000f\u00a1\u180c\u00a5\u001a\u00a7\u001c\u00a9\u1c1b\u00ab\u1d02\u00ad\u1419\u00b0\u1b1c\u00b2\u190b\u00b4\u0b1b\u00b6\u021c\u00b8\u181b\u00ba\u0b02\u00bc\u1e0b\u00be\u000b\u00c0\u1801\u00d6\u0001\u00d8\u1901\u00de\u0001\u00f6\u0002\u00f8\u1902\u00ff\u0002\u0137\u0201\u0148\u0102\u0178\u0201\u017e\u0102\u0180\u0002\u0182\u0001\u0186\u0201\u0189\u0102\u018b\u0001\u018d\u0002\u0191\u0001\u0193\u0102\u0196\u0201\u0198\u0001\u019b\u0002\u019d\u0001\u019f\u0102\u01a6\u0201\u01aa\u0102\u01ae\u0201\u01b1\u0102\u01b3\u0001\u01b7\u0102\u01b9\u0201\u01bb\u0502\u01bd\u0201\u01bf\u0002\u01c3\u0005\u01c5\u0301\u01c7\u0102\u01c9\u0203\u01cb\u0301\u01dc\u0102\u01ef\u0201\u01f1\u0102\u01f3\u0203\u01f6\u0201\u01f8\u0001\u021f\u0201\u0233\u0201\u02ad\u0002\u02b8\u0004\u02ba\u001b\u02c1\u0004\u02cf\u001b\u02d1\u0004\u02df\u001b\u02e4\u0004\u02ed\u001b\u02ee\u0004\u034e\u0006\u0362\u0006\u0375\u001b\u037a\u0004\u037e\u0018\u0385\u001b\u0388\u1801\u038a\u0001\u038c\u0001\u038f\u0001\u0391\u0102\u03a1\u0001\u03ab\u0001\u03ce\u0002\u03d1\u0002\u03d4\u0001\u03d7\u0002\u03ef\u0201\u03f3\u0002\u042f\u0001\u045f\u0002\u0481\u0201\u0483\u061c\u0486\u0006\u0489\u0007\u04c0\u0201\u04c4\u0102\u04c8\u0102\u04cc\u0102\u04f5\u0201\u04f9\u0201\u0556\u0001\u055a\u0418\u055f\u0018\u0587\u0002\u058a\u1814\u05a1\u0006\u05b9\u0006\u05bd\u0006\u05c1\u0618\u05c4\u1806\u05ea\u0005\u05f2\u0005\u05f4\u0018\u060c\u0018\u061b\u1800\u061f\u1800\u063a\u0005\u0641\u0504\u064a\u0005\u0655\u0006\u0669\t\u066d\u0018\u0671\u0506\u06d3\u0005\u06d5\u0518\u06dc\u0006\u06de\u0007\u06e4\u0006\u06e6\u0004\u06e8\u0006\u06ea\u1c06\u06ed\u0006\u06f9\t\u06fc\u0005\u06fe\u001c\u070d\u0018\u0710\u1005\u0712\u0605\u072c\u0005\u074a\u0006\u07a5\u0005\u07b0\u0006\u0902\u0006\u0903\u0800\u0939\u0005\u093d\u0506\u0940\b\u0948\u0006\u094c\b\u094d\u0600\u0951\u0605\u0954\u0006\u0961\u0005\u0963\u0006\u0965\u0018\u096f\t\u0970\u0018\u0982\u0608\u0983\u0800\u098c\u0005\u0990\u0005\u09a8\u0005\u09b0\u0005\u09b2\u0005\u09b9\u0005\u09bc\u0006\u09c0\b\u09c4\u0006\u09c8\b\u09cc\b\u09cd\u0600\u09d7\u0800\u09dd\u0005\u09e1\u0005\u09e3\u0006\u09ef\t\u09f1\u0005\u09f3\u001a\u09f9\u000b\u09fa\u001c\u0a02\u0006\u0a0a\u0005\u0a10\u0005\u0a28\u0005\u0a30\u0005\u0a33\u0005\u0a36\u0005\u0a39\u0005\u0a3c\u0006\u0a40\b\u0a42\u0006\u0a48\u0006\u0a4d\u0006\u0a5c\u0005\u0a5e\u0005\u0a6f\t\u0a71\u0006\u0a74\u0005\u0a82\u0006\u0a83\u0800\u0a8b\u0005\u0a8d\u0500\u0a91\u0005\u0aa8\u0005\u0ab0\u0005\u0ab3\u0005\u0ab9\u0005\u0abd\u0506\u0ac0\b\u0ac5\u0006\u0ac8\u0006\u0ac9\u0800\u0acc\b\u0acd\u0600\u0ad0\u0005\u0ae0\u0005\u0aef\t\u0b02\u0608\u0b03\u0800\u0b0c\u0005\u0b10\u0005\u0b28\u0005\u0b30\u0005\u0b33\u0005\u0b39\u0005\u0b3d\u0506\u0b41\u0608\u0b43\u0006\u0b48\b\u0b4c\b\u0b4d\u0600\u0b57\u0806\u0b5d\u0005\u0b61\u0005\u0b6f\t\u0b70\u001c\u0b83\u0806\u0b8a\u0005\u0b90\u0005\u0b95\u0005\u0b9a\u0005\u0b9c\u0005\u0b9f\u0005\u0ba4\u0005\u0baa\u0005\u0bb5\u0005\u0bb9\u0005\u0bbf\b\u0bc1\u0806\u0bc2\b\u0bc8\b\u0bcc\b\u0bcd\u0600\u0bd7\u0800\u0bef\t\u0bf2\u000b\u0c03\b\u0c0c\u0005\u0c10\u0005\u0c28\u0005\u0c33\u0005\u0c39\u0005\u0c40\u0006\u0c44\b\u0c48\u0006\u0c4d\u0006\u0c56\u0006\u0c61\u0005\u0c6f\t\u0c83\b\u0c8c\u0005\u0c90\u0005\u0ca8\u0005\u0cb3\u0005\u0cb9\u0005\u0cc0\u0608\u0cc4\b\u0cc7\u0806\u0cc8\b\u0ccb\b\u0ccd\u0006\u0cd6\b\u0cde\u0005\u0ce1\u0005\u0cef\t\u0d03\b\u0d0c\u0005\u0d10\u0005\u0d28\u0005\u0d39\u0005\u0d40\b\u0d43\u0006\u0d48\b\u0d4c\b\u0d4d\u0600\u0d57\u0800\u0d61\u0005\u0d6f\t\u0d83\b\u0d96\u0005\u0db1\u0005\u0dbb\u0005\u0dbd\u0500\u0dc6\u0005\u0dca\u0006\u0dd1\b\u0dd4\u0006\u0dd6\u0006\u0ddf\b\u0df3\b\u0df4\u0018\u0e30\u0005\u0e32\u0605\u0e34\u0506\u0e3a\u0006\u0e40\u1a05\u0e45\u0005\u0e47\u0604\u0e4e\u0006\u0e50\u1809\u0e59\t\u0e5b\u0018\u0e82\u0005\u0e84\u0005\u0e88\u0005\u0e8a\u0005\u0e8d\u0500\u0e97\u0005\u0e9f\u0005\u0ea3\u0005\u0ea5\u0500\u0ea7\u0500\u0eab\u0005\u0eb0\u0005\u0eb2\u0605\u0eb4\u0506\u0eb9\u0006\u0ebc\u0006\u0ebd\u0500\u0ec4\u0005\u0ec6\u0004\u0ecd\u0006\u0ed9\t\u0edd\u0005\u0f01\u1c05\u0f03\u001c\u0f12\u0018\u0f17\u001c\u0f19\u0006\u0f1f\u001c\u0f29\t\u0f33\u000b\u0f39\u061c\u0f3d\u1615\u0f3f\b\u0f47\u0005\u0f6a\u0005\u0f7e\u0006\u0f80\u0806\u0f84\u0006\u0f86\u1806\u0f88\u0605\u0f8b\u0005\u0f97\u0006\u0fbc\u0006\u0fc5\u001c\u0fc7\u1c06\u0fcc\u001c\u0fcf\u1c00\u1021\u0005\u1027\u0005\u102a\u0005\u102d\u0608\u1030\u0006\u1032\u0806\u1037\u0006\u1039\u0608\u1049\t\u104f\u0018\u1055\u0005\u1057\b\u1059\u0006\u10c5\u0001\u10f6\u0005\u10fb\u1800\u1159\u0005\u11a2\u0005\u11f9\u0005\u1206\u0005\u1246\u0005\u1248\u0005\u124d\u0005\u1256\u0005\u1258\u0005\u125d\u0005\u1286\u0005\u1288\u0005\u128d\u0005\u12ae\u0005\u12b0\u0005\u12b5\u0005\u12be\u0005\u12c0\u0005\u12c5\u0005\u12ce\u0005\u12d6\u0005\u12ee\u0005\u130e\u0005\u1310\u0005\u1315\u0005\u131e\u0005\u1346\u0005\u135a\u0005\u1368\u0018\u1371\t\u137c\u000b\u13f4\u0005\u166c\u0005\u166e\u0018\u1676\u0005\u1681\u050c\u169a\u0005\u169c\u1516\u16ea\u0005\u16ed\u0018\u16f0\u000b\u17b3\u0005\u17b6\b\u17bd\u0006\u17c5\b\u17c7\u0806\u17c9\u0608\u17d3\u0006\u17da\u0018\u17dc\u1a18\u17e9\t\u1805\u0018\u1807\u1814\u180a\u0018\u180e\u0010\u1819\t\u1842\u0005\u1844\u0405\u1877\u0005\u18a8\u0005\u18a9\u0600\u1e95\u0201\u1e9b\u0002\u1ef9\u0201\u1f07\u0002\u1f0f\u0001\u1f15\u0002\u1f1d\u0001\u1f27\u0002\u1f2f\u0001\u1f37\u0002\u1f3f\u0001\u1f45\u0002\u1f4d\u0001\u1f57\u0002\u1f59\u0100\u1f5b\u0100\u1f5d\u0100\u1f60\u0102\u1f67\u0002\u1f6f\u0001\u1f7d\u0002\u1f87\u0002\u1f8f\u0003\u1f97\u0002\u1f9f\u0003\u1fa7\u0002\u1faf\u0003\u1fb4\u0002\u1fb7\u0002\u1fbb\u0001\u1fbd\u1b03\u1fbf\u1b02\u1fc1\u001b\u1fc4\u0002\u1fc7\u0002\u1fcb\u0001\u1fcd\u1b03\u1fcf\u001b\u1fd3\u0002\u1fd7\u0002\u1fdb\u0001\u1fdf\u001b\u1fe7\u0002\u1fec\u0001\u1fef\u001b\u1ff4\u0002\u1ff7\u0002\u1ffb\u0001\u1ffd\u1b03\u1ffe\u001b\u200b\f\u200f\u0010\u2015\u0014\u2017\u0018\u2019\u1e1d\u201b\u1d15\u201d\u1e1d\u201f\u1d15\u2027\u0018\u2029\u0e0d\u202e\u0010\u2030\u0c18\u2038\u0018\u203a\u1d1e\u203e\u0018\u2040\u0017\u2043\u0018\u2045\u1519\u2046\u0016\u204d\u0018\u206f\u0010\u2070\u000b\u2079\u000b\u207c\u0019\u207e\u1516\u2080\u020b\u2089\u000b\u208c\u0019\u208e\u1516\u20af\u001a\u20dc\u0006\u20e0\u0007\u20e2\u0607\u20e3\u0700\u2101\u001c\u2103\u1c01\u2106\u001c\u2108\u011c\u210a\u1c02\u210d\u0001\u210f\u0002\u2112\u0001\u2114\u021c\u2116\u011c\u2118\u001c\u211d\u0001\u2123\u001c\u212a\u1c01\u212d\u0001\u212f\u021c\u2131\u0001\u2133\u011c\u2135\u0502\u2138\u0005\u213a\u021c\u215f\u000b\u2183\n\u2194\u0019\u2199\u001c\u219b\u0019\u219f\u001c\u21a1\u1c19\u21a4\u191c\u21a7\u1c19\u21ad\u001c\u21af\u1c19\u21cd\u001c\u21cf\u0019\u21d1\u001c\u21d5\u1c19\u21f3\u001c\u22f1\u0019\u2307\u001c\u230b\u0019\u231f\u001c\u2321\u0019\u2328\u001c\u232a\u1516\u237b\u001c\u239a\u001c\u2426\u001c\u244a\u001c\u249b\u000b\u24e9\u001c\u24ea\u000b\u2595\u001c\u25b6\u001c\u25b8\u191c\u25c0\u001c\u25c2\u191c\u25f7\u001c\u2613\u001c\u266e\u001c\u2670\u191c\u2671\u1c00\u2704\u001c\u2709\u001c\u2727\u001c\u274b\u001c\u274d\u1c00\u2752\u001c\u2756\u001c\u275e\u001c\u2767\u001c\u2793\u000b\u2794\u001c\u27af\u001c\u27be\u001c\u28ff\u001c\u2e99\u001c\u2ef3\u001c\u2fd5\u001c\u2ffb\u001c\u3001\u180c\u3003\u0018\u3005\u041c\u3007\u0a05\u3011\u1615\u3013\u001c\u301b\u1615\u301d\u1514\u301f\u0016\u3021\u0a1c\u3029\n\u302f\u0006\u3031\u0414\u3035\u0004\u3037\u001c\u303a\n\u303f\u001c\u3094\u0005\u309a\u0006\u309c\u001b\u309e\u0004\u30fa\u0005\u30fc\u1704\u30fe\u0004\u312c\u0005\u318e\u0005\u3191\u001c\u3195\u000b\u319f\u001c\u31b7\u0005\u321c\u001c\u3229\u000b\u3243\u001c\u327b\u001c\u3280\u1c0b\u3289\u000b\u32b0\u001c\u32cb\u001c\u32fe\u001c\u3376\u001c\u33dd\u001c\u33fe\u001c\u4db5\u0005\u9fa5\u0005\ua48c\u0005\ua4a1\u001c\ua4b3\u001c\ua4c0\u001c\ua4c4\u001c\ua4c6\u001c\ud7a3\u0005\udfff\u0013\uf8ff\u0012\ufa2d\u0005\ufb06\u0002\ufb17\u0002\ufb1f\u0506\ufb28\u0005\ufb2a\u1905\ufb36\u0005\ufb3c\u0005\ufb3e\u0005\ufb41\u0005\ufb44\u0005\ufbb1\u0005\ufd3d\u0005\ufd3f\u1615\ufd8f\u0005\ufdc7\u0005\ufdfb\u0005\ufe23\u0006\ufe31\u1418\ufe33\u1714\ufe35\u1517\ufe44\u1516\ufe4c\u0018\ufe4f\u0017\ufe52\u0018\ufe57\u0018\ufe59\u1514\ufe5e\u1516\ufe61\u0018\ufe64\u1419\ufe66\u0019\ufe6a\u1a18\ufe6b\u1800\ufe72\u0005\ufe74\u0005\ufefc\u0005\ufeff\u1000\uff03\u0018\uff05\u181a\uff07\u0018\uff09\u1615\uff0c\u1918\uff0e\u1418\uff10\u1809\uff19\t\uff1b\u0018\uff1e\u0019\uff20\u0018\uff3a\u0001\uff3c\u1518\uff3e\u161b\uff40\u171b\uff5a\u0002\uff5c\u1519\uff5e\u1619\uff62\u1815\uff64\u1618\uff66\u1705\uff6f\u0005\uff71\u0504\uff9d\u0005\uff9f\u0004\uffbe\u0005\uffc7\u0005\uffcf\u0005\uffd7\u0005\uffdc\u0005\uffe1\u001a\uffe3\u1b19\uffe5\u1a1c\uffe6\u001a\uffe9\u191c\uffec\u0019\uffee\u001c\ufffb\u0010\ufffd\u001c").getValue();
    
    private static final int[] typeValuesCache = {
     	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
     	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
     	12, 24, 24, 24, 26, 24, 24, 24, 21, 22, 24, 25, 24, 20, 24, 24,
     	9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 24, 24, 25, 25, 25, 24,
     	24, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 21, 24, 22, 27, 23,
     	27, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
     	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 21, 25, 22, 25, 15,
     	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
     	15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
     	12, 24, 26, 26, 26, 26, 28, 28, 27, 28, 2, 29, 25, 16, 28, 27,
     	28, 25, 11, 11, 27, 2, 28, 24, 27, 11, 2, 30, 11, 11, 11, 24,
     	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     	1, 1, 1, 1, 1, 1, 1, 25, 1, 1, 1, 1, 1, 1, 1, 2,
     	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
     	2, 2, 2, 2, 2, 2, 2, 25, 2, 2, 2, 2, 2, 2, 2, 2,
     	1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
     	1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
     	1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
     	1, 2, 1, 2, 1, 2, 1, 2, 2, 1, 2, 1, 2, 1, 2, 1,
     	2, 1, 2, 1, 2, 1, 2, 1, 2, 2, 1, 2, 1, 2, 1, 2,
     	1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
     	1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
     	1, 2, 1, 2, 1, 2, 1, 2, 1, 1, 2, 1, 2, 1, 2, 2,
     	2, 1, 1, 2, 1, 2, 1, 1, 2, 1, 1, 1, 2, 2, 1, 1,
     	1, 1, 2, 1, 1, 2, 1, 1, 1, 2, 2, 2, 1, 1, 2, 1,
     	1, 2, 1, 2, 1, 2, 1, 1, 2, 1, 2, 2, 1, 2, 1, 1,
     	2, 1, 1, 1, 2, 1, 2, 1, 1, 2, 2, 5, 1, 2, 2, 2,
     	5, 5, 5, 5, 1, 3, 2, 1, 3, 2, 1, 3, 2, 1, 2, 1,
     	2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 2, 1, 2,
     	1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
     	2, 1, 3, 2, 1, 2, 1, 1, 1, 2, 1, 2, 1, 2, 1, 2,
     	1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
     	1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
     	1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,
     	1, 2, 1, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
     	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
     	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
     	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
     	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
     	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
     	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
     	4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
     	4, 4, 27, 27, 27, 27, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
     	4, 4, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
     	4, 4, 4, 4, 4, 27, 27, 27, 27, 27, 27, 27, 27, 27, 4, 27,
     	27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
     	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
     	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
     	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
     	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
     	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
     	6, 6, 6, 6, 6, 6, 6, 6, 0, 0, 0, 0, 0, 6, 6, 6,
     	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
     	0, 0, 0, 0, 27, 27, 0, 0, 0, 0, 4, 0, 0, 0, 24, 0,
     	0, 0, 0, 0, 27, 27, 1, 24, 1, 1, 1, 0, 1, 0, 1, 1,
     	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
     	1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2,
     	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
     	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 0,
     	2, 2, 1, 1, 1, 2, 2, 2, 1, 2, 1, 2, 1, 2, 1, 2,
     	1, 2, 1, 2, 1, 2, 1, 2 };
    
    /**
     * The minimum value of a high surrogate or leading surrogate unit in UTF-16
     * encoding, {@code '\uD800'}.
     *
     * @since 1.5
     */
    public static final char MIN_HIGH_SURROGATE = '\uD800';

    /**
     * The maximum value of a high surrogate or leading surrogate unit in UTF-16
     * encoding, {@code '\uDBFF'}.
     *
     * @since 1.5
     */
    public static final char MAX_HIGH_SURROGATE = '\uDBFF';
    
    /**
     * Constructs a new {@code Character} with the specified primitive char
     * value.
     *
     * @param value
     *            the primitive char value to store in the new instance.
     */
    public Character(char value) {
        this.value = value;
    }

    /**
     * Gets the primitive value of this character.
     *
     * @return this object's primitive value.
     */
    public char charValue() {
        return value;
    }
    
    /**
     * Compares this object with the specified object and indicates if they are
     * equal. In order to be equal, {@code object} must be an instance of
     * {@code Character} and have the same char value as this object.
     *
     * @param object
     *            the object to compare this double with.
     * @return {@code true} if the specified object is equal to this
     *         {@code Character}; {@code false} otherwise.
     */
    //@Override
    public boolean equals(Object object) {
        return (object instanceof Character) && (((Character) object).value == value);
    }

    //@Override
    public int hashCode() {
        return value;
    }


    /**
     * Determines if the specified character is a digit.
     *
     * @param   ch   the character to be tested.
     * @return  <code>true</code> if the character is a digit;
     *          <code>false</code> otherwise.
     * @since   JDK1.0
     */
    public static boolean isDigit(char ch) {
        // Most non-digits are > '9' so check that first
        return ch <= '9' && ch >= '0';
    }

    /**
     * Determines if the specified character is a lowercase character.
     *
     * @param   ch   the character to be tested.
     * @return  <code>true</code> if the character is lowercase;
     *          <code>false</code> otherwise.
     * @since   JDK1.0
     */
    public static boolean isLowerCase(char ch) {
        return ch >= 'a' &&
            (ch <= 'z' || (ch >= 0xDF && ch <= 0xFF && ch != 0xF7));
    }

    /**
     * Determines if the specified character is an uppercase character.
     *
     * @param   ch   the character to be tested.
     * @return  <code>true</code> if the character is uppercase;
     *          <code>false</code> otherwise.
     * @see     java.lang.Character#isLowerCase(char)
     * @see     java.lang.Character#toUpperCase(char)
     * @since   1.0
     */
    public static boolean isUpperCase(char ch) {
        return ch >= 'A' &&
            (ch <= 'Z' || (ch >= 0xC0 && ch <= 0xDE && ch != 0xD7));
    }
    
    /**
     * The given character is mapped to its lowercase equivalent; if the
     * character has no lowercase equivalent, the character itself is
     * returned.
     *
     * @param   ch   the character to be converted.
     * @return  the lowercase equivalent of the character, if any;
     *          otherwise the character itself.
     * @see     java.lang.Character#isLowerCase(char)
     * @see     java.lang.Character#isUpperCase(char)
     * @see     java.lang.Character#toUpperCase(char)
     * @since   JDK1.0
     */
    public static char toLowerCase(char ch) {
        if (ch >= 'A' &&
            (ch <= 'Z' || (ch >= 0xC0 && ch <= 0xDE && ch != 0xD7)))
        {
            ch += ('a' - 'A');
        }
        return ch;
    }

    /**
     * Converts the character argument to uppercase; if the
     * character has no lowercase equivalent, the character itself is
     * returned.
     *
     * @param   ch   the character to be converted.
     * @return  the uppercase equivalent of the character, if any;
     *          otherwise the character itself.
     * @see     java.lang.Character#isLowerCase(char)
     * @see     java.lang.Character#isUpperCase(char)
     * @see     java.lang.Character#toLowerCase(char)
     * @since   JDK1.0
     */
    public static char toUpperCase(char ch) {
        if (ch >= 'a' &&
            (ch <= 'z' || (ch >= 0xE0 && ch <= 0xFE && ch != 0xF7)))
        {
            ch -= ('a' - 'A');
        }
        return ch;
    }
    
    /**
     * Returns a String object representing this character's value.
     * Converts this <code>Character</code> object to a string. The
     * result is a string whose length is <code>1</code>. The string's
     * sole component is the primitive <code>char</code> value represented
     * by this object.
     *
     * @return  a string representation of this object.
     */
    public String toString() {
        return String.valueOf(value);
    }

    /**
     * Returns a String object representing this character's value.
     * Converts this <code>Character</code> object to a string. The
     * result is a string whose length is <code>1</code>. The string's
     * sole component is the primitive <code>char</code> value represented
     * by this object.
     *
     * @return  a string representation of this object.
     */
    public static String toString(char c) {
        return String.valueOf(c);
    }
    
    
    /**
     * Returns the numeric value of the character <code>ch</code> in the
     * specified radix.
     *
     * @param   ch      the character to be converted.
     * @param   radix   the radix.
     * @return  the numeric value represented by the character in the
     *          specified radix.
     * @see     java.lang.Character#isDigit(char)
     * @since   JDK1.0
     */
    public static int digit(char ch, int radix) {

    VALID:
        if (radix >= MIN_RADIX && radix <= MAX_RADIX) {

            int result;

            if (ch < '0')
                break VALID;
            else if (ch <= '9')
                result = ch - '0';
            else {
                result = (ch & ~0x20) - ('A' - 10);
                if (result < 10)
                    break VALID;        // only want hex digits and upwards
            }

            if (result < radix)
                return result;
        }

        // break VALID comes here

        return -1;
    }
    
    public static boolean isLetter(char c) {
        if (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
            return true;
        }
        if (c < 128) {
            return false;
        }
        int type = getType(c);
        return type >= UPPERCASE_LETTER && type <= OTHER_LETTER;
    }
    
    public static boolean isLetterOrDigit(char c) {
    	int type = getType(c);
    	return (type >= UPPERCASE_LETTER && type <= OTHER_LETTER)
    	| type == DECIMAL_DIGIT_NUMBER;
	}
    
    public static int getType(char c) {
        if(c < 1000) {
            return typeValuesCache[(int)c];
        }
        int result = BinarySearch.binarySearchRange(typeKeys, c);
        int high = typeValues[result * 2];
        if (c <= high) {
            int code = typeValues[result * 2 + 1];
            if (code < 0x100) {
                return code;
            }
            return (c & 1) == 1 ? code >> 8 : code & 0xff;
        }
        return UNASSIGNED;
    }
    
    /**
     * Indicates whether {@code ch} is a high- (or leading-) surrogate code unit
     * that is used for representing supplementary characters in UTF-16
     * encoding.
     *
     * @param ch
     *            the character to test.
     * @return {@code true} if {@code ch} is a high-surrogate code unit;
     *         {@code false} otherwise.
     * @see #isLowSurrogate(char)
     * @since 1.5
     */
    public static boolean isHighSurrogate(char ch) {
        return (MIN_HIGH_SURROGATE <= ch && MAX_HIGH_SURROGATE >= ch);
    }
}
