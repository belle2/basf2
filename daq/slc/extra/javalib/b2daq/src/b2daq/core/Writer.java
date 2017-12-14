package b2daq.core;

import java.io.IOException;

public abstract class Writer {

	abstract public void writeBoolean(boolean c) throws IOException;
	abstract public void writeChar(char c) throws IOException;
	abstract public void writeByte(byte c) throws IOException;
	abstract public void writeShort(short c) throws IOException;
	abstract public void writeInt(int c) throws IOException;
	abstract public void writeLong(long c) throws IOException;
	abstract public void writeFloat(float c) throws IOException;
	abstract public void writeDouble(double c) throws IOException;
	abstract public void writeString(String str) throws IOException;
	public void writeObject(Serializable obj) throws IOException {
		obj.writeObject(this);
	}

}
