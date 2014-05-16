package b2daq.core;

import java.io.IOException;

public class SizeCounter extends Writer {

	private int _count = 0;

	public SizeCounter() {
	}
	
	public int count() { return _count; }
	
	@Override
	public void writeBoolean(boolean c) throws IOException {
		_count += 1;
	}

	@Override
	public void writeByte(byte c) throws IOException {
		_count += 1;
	}

	@Override
	public void writeChar(char c) throws IOException {
		_count += 1;
	}

	@Override
	public void writeDouble(double c) throws IOException {
		_count += 8;
	}

	@Override
	public void writeFloat(float c) throws IOException {
		_count += 4;
	}

	@Override
	public void writeInt(int c) throws IOException {
		_count += 4;
	}

	@Override
	public void writeLong(long c) throws IOException {
		_count += 8;
	}

	@Override
	public void writeShort(short c) throws IOException {
		_count += 2;
	}

	@Override
	public void writeString(String str) throws IOException {
		_count += 4 + str.length();
	}

}
