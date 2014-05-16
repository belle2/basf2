package b2daq.io;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.OutputStream;

import b2daq.core.Writer;


public class DataWriter extends Writer {

	protected DataOutputStream _stream;
	
	public DataWriter() {}
	public DataWriter(OutputStream stream) {
		_stream = new DataOutputStream(stream);
	}
	
	@Override
	public void writeBoolean(boolean c) throws IOException {
		_stream.writeByte((byte)(c?1:0));
	}

	@Override
	public void writeByte(byte c) throws IOException {
		_stream.writeByte(c);
	}

	@Override
	public void writeChar(char c) throws IOException {
		_stream.writeByte((byte)c);
	}

	@Override
	public void writeDouble(double c) throws IOException {
		_stream.writeDouble(c);
	}

	@Override
	public void writeFloat(float c) throws IOException {
		_stream.writeFloat(c);
	}

	@Override
	public void writeInt(int c) throws IOException {
		_stream.writeInt(c);
	}

	@Override
	public void writeLong(long c) throws IOException {
		_stream.writeLong(c);
	}

	@Override
	public void writeShort(short c) throws IOException {
		_stream.writeShort(c);
	}

	@Override
	public void writeString(String str) throws IOException {
		writeInt(str.length());
		for ( int n = 0; n < str.length(); n++ ) {
			writeChar(str.charAt(n));
		}
	}

}
