package b2dqm.io;

import b2daq.core.Reader;
import b2daq.core.Writer;

public class StreamHeader {
	
	public static final StreamHeader REQUEST = new StreamHeader((byte)0xDC, (byte)0xF8);
	public static final StreamHeader UPDATE = new StreamHeader((byte)0xDC, (byte)0xFC);
	public static final StreamHeader CONFIG = new StreamHeader((byte)0xDC, (byte)0xFD);
	public static final StreamHeader CLOSE = new StreamHeader((byte)0xDC, (byte)0xFE);
	public static final StreamHeader STOP = new StreamHeader((byte)0xDC, (byte)0xFB);
	public static final StreamHeader CONTENTS = new StreamHeader((byte)0xDC, (byte)0xFA);
	public static final StreamHeader DELETE = new StreamHeader((byte)0xDC, (byte)0xF9);
	public static final StreamHeader STATE = new StreamHeader((byte)0xDC, (byte)0xBB);
	public static final StreamHeader CONNECTION = new StreamHeader((byte)0xDC, (byte)0xCC);
	public static final StreamHeader NEW_RUN = new StreamHeader((byte)0xDC, (byte)0xAA);
	public static final StreamHeader TERMINATE = new StreamHeader((byte)0xDC, (byte)0x99);

	public static final StreamHeader REQUEST_RESPONSE = new StreamHeader((byte)0xCD, (byte)0xF8);
	public static final StreamHeader UPDATE_RESPONSE = new StreamHeader((byte)0xCD, (byte)0xFC);
	public static final StreamHeader CONFIG_RESPONSE = new StreamHeader((byte)0xCD, (byte)0xFD);
	public static final StreamHeader CLOSE_RESPONSE = new StreamHeader((byte)0xCD, (byte)0xFE);
	public static final StreamHeader STOP_RESPONSE = new StreamHeader((byte)0xCD, (byte)0xFB);
	public static final StreamHeader CONTENTS_RESPONSE = new StreamHeader((byte)0xCD, (byte)0xFA);
	public static final StreamHeader DELETE_RESPONSE = new StreamHeader((byte)0xCD, (byte)0xF9);
	public static final StreamHeader STATE_RESPONSE = new StreamHeader((byte)0xCD, (byte)0xBB);
	public static final StreamHeader CONNECTION_RESPONSE = new StreamHeader((byte)0xCD, (byte)0xCC);
	public static final StreamHeader NEW_RUN_RESPONSE = new StreamHeader((byte)0xCD, (byte)0xAA);

	private byte _flag, _option;

	public StreamHeader() {
		_flag = 0;
		_option = 0;
	}

	private StreamHeader(int flag, int option) {
		_flag = (byte)flag;
		_option = (byte)option;
	}

	public boolean matches(StreamHeader header) {
		return (_flag == header._flag && _option == header._option);
	}

	public void write(Writer writer) throws Exception {
		writer.writeChar((char)_flag);
		writer.writeChar((char)_option);
	}

	public void read(Reader reader) throws Exception {
		_flag = (byte)reader.readChar();
		_option = (byte)reader.readChar();
	}
}
