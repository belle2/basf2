package b2daq.java.io;

//import java.io.BufferedInputStream;
import java.io.DataInputStream;
import java.io.EOFException;
import java.io.InputStream;

import b2daq.core.Reader;

public class DataReader extends Reader {

	protected DataInputStream _stream;
	
	public DataReader() {}
	public DataReader(InputStream stream) {
		//_stream = new DataInputStream(new BufferedInputStream(stream));
		_stream = new DataInputStream(stream);
	}
	
	@Override
	public boolean readBoolean() throws Exception {
		return _stream.readByte()==1;
	}

	@Override
	public byte readByte() throws Exception {
		return _stream.readByte();
	}

	@Override
	public char readChar() throws Exception {
		return (char)_stream.readByte();
	}

	@Override
	public double readDouble() throws Exception {
		return _stream.readDouble();
	}

	@Override
	public float readFloat() throws Exception {
		return _stream.readFloat();
	}

	@Override
	public long readLong() throws Exception {
		return _stream.readLong();
	}

	@Override
	public int readInt() throws Exception {
		while ( _stream.available() == 0 ) {}
		return _stream.readInt();
	}

	@Override
	public short readShort() throws Exception {
		return _stream.readShort();
	}

	@Override
	public String readString() throws Exception {
		int length = readInt();
		byte c;
		StringBuffer buffer = new StringBuffer();
		for ( int i = 0; i < length; i++ ) {
			c = readByte();
			buffer.append((char)c);
		}
		return buffer.toString();
	}

	public boolean available() throws Exception {
		if ( _stream.available() > 0) return true;
		else return false;
	}
	
}
