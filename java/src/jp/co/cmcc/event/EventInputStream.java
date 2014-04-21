package jp.co.cmcc.event;

import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;

public class EventInputStream extends DataInputStream {

	public EventInputStream(InputStream in) {
		super(in);
	}

	public final String readString() {
		int len  = 0;
		char[] c = null;

		try {
			len  = in.available();
			len = (len++)>>1<<1;
			c = new char[len>>1];
			for (int i = 0; i < len>>1; i++) {
				c[i] = readChar();
				if (c[i] < 0) break; //EOF
			}
		} catch (IOException e) {
			e.printStackTrace();
		}
		return new String(c);
	}
}
