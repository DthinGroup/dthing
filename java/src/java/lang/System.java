/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package java.lang;

import java.io.InputStream;
import java.io.PrintStream;


/**
 * Provides access to system-related information and resources including
 * standard input and output. Enables clients to dynamically load native
 * libraries. All methods of this class are accessed in a static way and the
 * class itself can not be instantiated.
 *
 * @see Runtime
 */
public final class System {

    /**
     * Default input stream.
     */
    public static final InputStream in;

    /**
     * Default output stream.
     */
    public static final PrintStream out;

    /**
     * Default error output stream.
     */
    public static final PrintStream err;


    static {
        // TODO: all three streams are buffered in Harmony.        
        out = new PrintStream(new com.yarlungsoft.util.SystemPrintStream(true));
        err = new PrintStream(new com.yarlungsoft.util.SystemPrintStream(false));
        in = new com.yarlungsoft.util.SystemInputStream();

    }

    /**
     * Prevents this class from being instantiated.
     */
    private System() {
    }

    /**
     * Copies {@code length} elements from the array {@code src},
     * starting at offset {@code srcPos}, into the array {@code dst},
     * starting at offset {@code dstPos}.
     *
     * @param src
     *            the source array to copy the content.
     * @param srcPos
     *            the starting index of the content in {@code src}.
     * @param dst
     *            the destination array to copy the data into.
     * @param dstPos
     *            the starting index for the copied content in {@code dst}.
     * @param length
     *            the number of elements to be copied.
     */
    public static native void arraycopy(Object src, int srcPos, Object dst, int dstPos, int length);

    /**
     * Returns the current system time in milliseconds since January 1, 1970
     * 00:00:00 UTC. This method shouldn't be used for measuring timeouts or
     * other elapsed time measurements, as changing the system time can affect
     * the results.
     *
     * @return the local system time in milliseconds.
     */
    public static native long currentTimeMillis();

    /**
     * Causes the VM to stop running and the program to exit. If
     * {@link #runFinalizersOnExit(boolean)} has been previously invoked with a
     * {@code true} argument, then all objects will be properly
     * garbage-collected and finalized first.
     *
     * @param code
     *            the return code.
     */
    public static void exit(int code) {
        Runtime.getRuntime().exit(code);
    }

    /**
     * Indicates to the VM that it would be a good time to run the
     * garbage collector. Note that this is a hint only. There is no guarantee
     * that the garbage collector will actually be run.
     */
    public static void gc() {
        Runtime.getRuntime().gc();
    }

    /**
     * Returns the value of a particular system property or {@code null} if no
     * such property exists.
     *
     * <p>The following properties are always provided by the Dalvik VM:
     * <p><table BORDER="1" WIDTH="100%" CELLPADDING="3" CELLSPACING="0" SUMMARY="">
     * <tr BGCOLOR="#CCCCFF" CLASS="TableHeadingColor">
     *     <td><b>Name</b></td>        <td><b>Meaning</b></td>                    <td><b>Example</b></td></tr>
     * <tr><td>file.separator</td>     <td>{@link java.io.File#separator}</td>    <td>{@code /}</td></tr>
     *
     * <tr><td>java.class.path</td>    <td>System class path</td>                 <td>{@code .}</td></tr>
     * <tr><td>java.class.version</td> <td>(Not useful on Android)</td>           <td>{@code 50.0}</td></tr>
     * <tr><td>java.compiler</td>      <td>(Not useful on Android)</td>           <td>Empty</td></tr>
     * <tr><td>java.ext.dirs</td>      <td>(Not useful on Android)</td>           <td>Empty</td></tr>
     * <tr><td>java.home</td>          <td>Location of the VM on the file system</td> <td>{@code /system}</td></tr>
     * <tr><td>java.io.tmpdir</td>     <td>See {@link java.io.File#createTempFile}</td> <td>{@code /sdcard}</td></tr>
     * <tr><td>java.library.path</td>  <td>Search path for JNI libraries</td>     <td>{@code /system/lib}</td></tr>
     * <tr><td>java.vendor</td>        <td>Human-readable VM vendor</td>          <td>{@code The Android Project}</td></tr>
     * <tr><td>java.vendor.url</td>    <td>URL for VM vendor's web site</td>      <td>{@code http://www.android.com/}</td></tr>
     * <tr><td>java.version</td>       <td>(Not useful on Android)</td>           <td>{@code 0}</td></tr>
     *
     * <tr><td>java.specification.version</td>    <td>VM libraries version</td>        <td>{@code 0.9}</td></tr>
     * <tr><td>java.specification.vendor</td>     <td>VM libraries vendor</td>         <td>{@code The Android Project}</td></tr>
     * <tr><td>java.specification.name</td>       <td>VM libraries name</td>           <td>{@code Dalvik Core Library}</td></tr>
     * <tr><td>java.vm.version</td>               <td>VM implementation version</td>   <td>{@code 1.2.0}</td></tr>
     * <tr><td>java.vm.vendor</td>                <td>VM implementation vendor</td>    <td>{@code The Android Project}</td></tr>
     * <tr><td>java.vm.name</td>                  <td>VM implementation name</td>      <td>{@code Dalvik}</td></tr>
     * <tr><td>java.vm.specification.version</td> <td>VM specification version</td>    <td>{@code 0.9}</td></tr>
     * <tr><td>java.vm.specification.vendor</td>  <td>VM specification vendor</td>     <td>{@code The Android Project}</td></tr>
     * <tr><td>java.vm.specification.name</td>    <td>VM specification name</td>       <td>{@code Dalvik Virtual Machine Specification}</td></tr>
     *
     * <tr><td>line.separator</td>     <td>The system line separator</td>         <td>{@code \n}</td></tr>
     *
     * <tr><td>os.arch</td>            <td>OS architecture</td>                   <td>{@code armv7l}</td></tr>
     * <tr><td>os.name</td>            <td>OS (kernel) name</td>                  <td>{@code Linux}</td></tr>
     * <tr><td>os.version</td>         <td>OS (kernel) version</td>               <td>{@code 2.6.32.9-g103d848}</td></tr>
     *
     * <tr><td>path.separator</td>     <td>See {@link java.io.File#pathSeparator}</td> <td>{@code :}</td></tr>
     *
     * <tr><td>user.dir</td>           <td>Base of non-absolute paths</td>        <td>{@code /}</td></tr>
     * <tr><td>user.home</td>          <td>(Not useful on Android)</td>           <td>Empty</td></tr>
     * <tr><td>user.name</td>          <td>(Not useful on Android)</td>           <td>Empty</td></tr>
     *
     * </table>
     *
     * <p>It is a mistake to try to override any of these. Doing so will have unpredictable results.
     *
     * @param propertyName
     *            the name of the system property to look up.
     * @return the value of the specified system property or {@code null} if the
     *         property doesn't exist.
     */
    public static String getProperty(String propertyName) {
        return getProperty(propertyName, null);
    }

    /**
     * Returns the value of a particular system property. The {@code
     * defaultValue} will be returned if no such property has been found.
     *
     * @param prop
     *            the name of the system property to look up.
     * @param defaultValue
     *            the return value if the system property with the given name
     *            does not exist.
     * @return the value of the specified system property or the {@code
     *         defaultValue} if the property does not exist.
     */
    public static String getProperty(String prop, String defaultValue) {
    	//@UN-IMPLEMENT
    	return null;
    }


    /**
     * Returns an integer hash code for the parameter. The hash code returned is
     * the same one that would be returned by the method {@code
     * java.lang.Object.hashCode()}, whether or not the object's class has
     * overridden hashCode(). The hash code for {@code null} is {@code 0}.
     *
     * @param anObject
     *            the object to calculate the hash code.
     * @return the hash code for the given object.
     * @see java.lang.Object#hashCode
     */
    public static native int identityHashCode(Object anObject);
}
