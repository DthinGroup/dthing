package jp.co.cmcc.event;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;

public class Event {

	int module_id;
	int evt_type;
	byte[] evt_context;
	
	public Event(int moduleid, int type, byte[] context) {
		module_id = moduleid;
		evt_type  = type;
		evt_context = context;
	}

	
	public int get_moduleid() {
		return module_id;
	}
	
	public int getType() {
		return evt_type;
	}
	
	public EventInputStream getInputStream() {
		if (evt_context == null) {
			throw new NullPointerException();
		}
		ByteArrayInputStream bais = new ByteArrayInputStream(evt_context);
		DataInputStream dis = new DataInputStream(bais);
		return new EventInputStream(dis);
	}
	
}
