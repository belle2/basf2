package b2daq.core;

public class SizeCounter extends Writer {

	private int _count = 0;

	public SizeCounter() {
	}
	
	public int count() { return _count; }
	
	@Override
	public void writeBoolean(boolean c) throws Exception {
		_count += 1;
	}

	@Override
	public void writeByte(byte c) throws Exception {
		_count += 1;
	}

	@Override
	public void writeChar(char c) throws Exception {
		_count += 1;
	}

	@Override
	public void writeDouble(double c) throws Exception {
		_count += 8;
	}

	@Override
	public void writeFloat(float c) throws Exception {
		_count += 4;
	}

	@Override
	public void writeInt(int c) throws Exception {
		_count += 4;
	}

	@Override
	public void writeLong(long c) throws Exception {
		_count += 8;
	}

	@Override
	public void writeShort(short c) throws Exception {
		_count += 2;
	}

	@Override
	public void writeString(String str) throws Exception {
		_count += 4 + str.length();
	}

}
