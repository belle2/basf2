package b2daq.core;

public abstract class Reader {
	
	abstract public boolean readBoolean() throws Exception;
	abstract public char readChar() throws Exception;
	abstract public byte readByte() throws Exception;
	abstract public short readShort() throws Exception;
	abstract public int readInt() throws Exception;
	abstract public long readLong() throws Exception;
	abstract public float readFloat() throws Exception;
	abstract public double readDouble() throws Exception;
	abstract public String readString() throws Exception;
	public void readObject(SerializableObject obj) throws Exception {
		obj.readObject(this);
	}
	
}
