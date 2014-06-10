package b2daq.core;

import java.io.IOException;

public abstract class Reader {
	
	abstract public boolean readBoolean() throws IOException;
	abstract public char readChar() throws IOException;
	abstract public byte readByte() throws IOException;
	abstract public short readShort() throws IOException;
	abstract public int readInt() throws IOException;
	abstract public long readLong() throws IOException;
	abstract public float readFloat() throws IOException;
	abstract public double readDouble() throws IOException;
	abstract public String readString() throws IOException;
	public void readObject(Serializable obj) throws IOException {
		obj.readObject(this);
	}
	
}
