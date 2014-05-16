package b2daq.core;

public abstract class Writer {

	abstract public void writeBoolean(boolean c) throws Exception;
	abstract public void writeChar(char c) throws Exception;
	abstract public void writeByte(byte c) throws Exception;
	abstract public void writeShort(short c) throws Exception;
	abstract public void writeInt(int c) throws Exception;
	abstract public void writeLong(long c) throws Exception;
	abstract public void writeFloat(float c) throws Exception;
	abstract public void writeDouble(double c) throws Exception;
	abstract public void writeString(String str) throws Exception;
	public void writeObject(SerializableObject obj) throws Exception {
		obj.writeObject(this);
	}

}
