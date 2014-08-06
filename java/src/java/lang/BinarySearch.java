package java.lang;

public class BinarySearch {

	/**
	 * Search the sorted characters in the string and return an exact index or
	 * -1.
	 *
	 * @param data
	 *            the String to search
	 * @param value
	 *            the character to search for
	 * @return the matching index, or -1
	 */
	public static int binarySearch(String data, char value) {
		int low = 0, high = data.length() - 1;
		while (low <= high) {
			int mid = (low + high) >> 1;
			char target = data.charAt(mid);
			if (value == target)
				return mid;
			else if (value < target)
				high = mid - 1;
			else
				low = mid + 1;
		}
		return -1;
	}

	/**
	 * Search the sorted characters in the string and return the nearest index.
	 *
	 * @param data
	 *            the String to search
	 * @param c
	 *            the character to search for
	 * @return the nearest index
	 */
	public static int binarySearchRange(String data, char c) {
		char value = 0;
		int low = 0, mid = -1, high = data.length() - 1;
		while (low <= high) {
			mid = (low + high) >> 1;
			value = data.charAt(mid);
			if (c > value)
				low = mid + 1;
			else if (c == value)
				return mid;
			else
				high = mid - 1;
		}
		return mid - (c < value ? 1 : 0);
	}
}
