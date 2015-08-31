
package java.io;

/**
 * A representation of file and directory pathnames.
 * <p>
 * User interfaces and operating systems use system-dependent <em>pathname strings</em> to name
 * files and directories. This class presents a system-independent view of hierarchical pathnames.
 * <p>
 * Each name part of a pathname string is separated from the next by a single copy of the default
 * <em>separator character</em>. The default name-separator character is defined by the system
 * property <code>file.separator</code>, and is made available in the public static fields
 * <code>{@link #separator}</code> and <code>{@link #separatorChar}</code> of this class.
 * <p>
 * A pathname must be <em>absolute</em>. An absolute pathname is complete in that no other
 * information is required in order to locate the file that it denotes.
 * <p>
 * Instances of the <code>File</code> class are immutable; that is, once created, the pathname
 * represented by a <code>File</code> object will never change.
 */

public class File implements Serializable {

    /** use serialVersionUID from JDK 1.0.2 for interoperability */
    private static final long serialVersionUID = 301077366599181567L;

    /**
     * This absolute pathname's normalized pathname string. A normalized pathname string uses the
     * default name-separator character and does not contain any duplicate or redundant separators.
     *
     * @serial
     */
    private String path;

    /* -- Constructors -- */

    /**
     * Creates a new <code>File</code> instance with the given absolute pathname string. If the
     * given string is the empty string, then the result is the empty absolute pathname.
     *
     * @param pathname A pathname string
     * @throws NullPointerException If the <code>pathname</code> argument is <code>null</code>
     */
    public File(String pathname) {
        if (pathname == null) {
            throw new NullPointerException();
        }
        path = pathname;
    }

    /**
     * Atomically creates a new, empty file named by this absolute pathname if and only if a file
     * with this name does not yet exist. The check for the existence of the file and the creation
     * of the file if it does not exist are a single operation that is atomic with respect to all
     * other filesystem activities that might affect the file.
     *
     * @return <code>true</code> if the named file does not exist and was successfully created;
     * <code>false</code> if the named file already exists
     * @throws IOException If an I/O error occurred
     */
    public boolean createNewFile() throws IOException {
        if (exists()) {
            return false;
        }
        return createFile0(path);
    }

    private static native boolean createFile0(String path);

    /**
     * Deletes the file or directory denoted by this absolute pathname. If this pathname denotes a
     * directory, then the directory must be empty in order to be deleted.
     *
     * @return <code>true</code> if and only if the file or directory is successfully deleted;
     * <code>false</code> otherwise
     */
    public boolean delete() {
        if (!exists()) {
            return false;
        }
        return delete0(path);
    }

    private static native boolean delete0(String path);

    /**
     * Tests whether the file or directory denoted by this absolute pathname exists.
     *
     * @return <code>true</code> if and only if the file or directory denoted by this absolute
     * pathname exists; <code>false</code> otherwise
     */
    public boolean exists() {
        return exists0(path);
    }

    private static native boolean exists0(String path);

    /**
     * @return The string form of this absolute pathname
     */
    public String getPath() {
        return path;
    }

    /**
     * Tests whether the file denoted by this absolute pathname is a directory.
     *
     * @return <code>true</code> if and only if the file denoted by this absolute pathname exists
     * <em>and</em> is a directory; <code>false</code> otherwise
     */
    public boolean isDirectory() {
        return isDirectory0(path);
    }

    private static native boolean isDirectory0(String path);

    /**
     * Tests whether the file denoted by this absolute pathname is a normal file. A file is
     * <em>normal</em> if it is not a directory and, in addition, satisfies other system-dependent
     * criteria. Any non-directory file created by a Java application is guaranteed to be a normal
     * file.
     *
     * @return <code>true</code> if and only if the file denoted by this absolute pathname exists
     * <em>and</em> is a normal file; <code>false</code> otherwise
     */
    public boolean isFile() {
        return isFile0(path);
    }

    private static native boolean isFile0(String path);

    /**
     * Returns the time that the file denoted by this absolute pathname was last modified.
     *
     * @return A <code>long</code> value representing the time the file was last modified, measured
     * in milliseconds since the epoch (00:00:00 GMT, January 1, 1970), or <code>0L</code> if the
     * file does not exist or if an I/O error occurs
     */
    public long lastModified() {
        return lastModified0(path);
    }

    private static native long lastModified0(String path);

    /**
     * Returns the length of the file denoted by this absolute pathname. The return value is
     * unspecified if this pathname denotes a directory.
     *
     * @return The length, in bytes, of the file denoted by this absolute pathname, or
     * <code>0L</code> if the file does not exist. Some operating systems may return <code>0L</code>
     * for pathnames denoting system-dependent entities such as devices or pipes.
     */
    public long length() {
        if (isDirectory()) {
            return 0;
        }
        return length0(path);
    }

    private static native long length0(String path);

    /**
     * Returns an array of absolute pathnames denoting the files in the directory denoted by this
     * absolute pathname.
     * <p>
     * If this absolute pathname does not denote a directory, then this method returns {@code null}.
     * Otherwise an array of {@code File} objects is returned, one for each file or directory in the
     * directory. Pathnames denoting the directory itself and the directory's parent directory are
     * not included in the result.
     * <p>
     * There is no guarantee that the name strings in the resulting array will appear in any
     * specific order; they are not, in particular, guaranteed to appear in alphabetical order.
     *
     * @return An array of absolute pathnames denoting the files and directories in the directory
     * denoted by this absolute pathname. The array will be empty if the directory is empty. Returns
     * {@code null} if this absolute pathname does not denote a directory, or if an I/O error
     * occurs.
     */
    public File[] listFiles() {
        if (!isDirectory()) {
            return null;
        }
        String[] ss = list0(path);
        if (ss == null) {
            return null;
        }
        int n = ss.length;
        File[] fs = new File[n];
        for (int i = 0; i < n; i++) {
            fs[i] = new File(ss[i]);
        }
        return fs;
    }

    private static native String[] list0(String path);

    public long totalSize() {
        return totalSize0(path);
	}

    private static native long totalSize0(String path);

    public long availableSize() {
		return availableSize0(path);
    }

    private static native long availableSize0(String path);

    /**
     * Returns the pathname string of this absolute pathname. This is just the string returned by
     * the <code>{@link #getPath}</code> method.
     *
     * @return The string form of this absolute pathname
     */
    public String toString() {
        return getPath();
    }
}
