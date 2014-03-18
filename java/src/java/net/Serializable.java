
package java.net;

/**
 * An empty marker interface for classes that want to support serialization and
 * deserialization based on the {@code ObjectOutputStream} and {@code
 * ObjectInputStream} classes. Implementing this interface is enough to make
 * most classes serializable. If a class needs more fine-grained control over
 * the serialization process (for example to implement compatibility with older
 * versions of the class), it can achieve this by providing the following two
 * methods (signatures must match exactly):
 * <p>
 * {@code private void writeObject(java.io.ObjectOutputStream out) throws
 * IOException}
 * <p>
 * {@code private void readObject(java.io.ObjectInputStream in) throws
 * IOException, ClassNotFoundException}
 */
public interface Serializable {
    /* empty */
}