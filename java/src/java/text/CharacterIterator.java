package java.text;

/**
 * An interface for the bidirectional iteration over a group of characters. The
 * iteration starts at the begin index in the group of characters and continues
 * to one index before the end index.
 */
public interface CharacterIterator extends Cloneable {

    /**
     * A constant which indicates that there is no character at the current
     * index.
     */
    public static final char DONE = '\uffff';

    /**
     * Returns a new {@code CharacterIterator} with the same properties.
     *
     * @return a shallow copy of this character iterator.
     *
     * @see java.lang.Cloneable
     */
    public Object clone();

    /**
     * Returns the character at the current index.
     *
     * @return the current character, or {@code DONE} if the current index is
     *         past the beginning or end of the sequence.
     */
    public char current();

    /**
     * Sets the current position to the begin index and returns the character at
     * the new position.
     *
     * @return the character at the begin index.
     */
    public char first();

    /**
     * Returns the begin index.
     *
     * @return the index of the first character of the iteration.
     */
    public int getBeginIndex();

    /**
     * Returns the end index.
     *
     * @return the index one past the last character of the iteration.
     */
    public int getEndIndex();

    /**
     * Returns the current index.
     *
     * @return the current index.
     */
    public int getIndex();

    /**
     * Sets the current position to the end index - 1 and returns the character
     * at the new position.
     *
     * @return the character before the end index.
     */
    public char last();

    /**
     * Increments the current index and returns the character at the new index.
     *
     * @return the character at the next index, or {@code DONE} if the next
     *         index would be past the end.
     */
    public char next();

    /**
     * Decrements the current index and returns the character at the new index.
     *
     * @return the character at the previous index, or {@code DONE} if the
     *         previous index would be past the beginning.
     */
    public char previous();

    /**
     * Sets the current index to a new position and returns the character at the
     * new index.
     *
     * @param location
     *            the new index that this character iterator is set to.
     * @return the character at the new index, or {@code DONE} if the index is
     *         past the end.
     * @throws IllegalArgumentException
     *         if {@code location} is less than the begin index or greater than
     *         the end index.
     */
    public char setIndex(int location);
}