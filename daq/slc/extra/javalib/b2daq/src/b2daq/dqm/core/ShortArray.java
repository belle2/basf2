package b2daq.dqm.core;

import java.io.IOException;

import b2daq.core.Reader;
import b2daq.core.Writer;

public class ShortArray extends NumberArray {

	private short[] _value_v;
	
	public ShortArray() {
		_value_v = new short [0];
		for (int n = 0; n < length(); n++ ) _value_v[n] = 0;
	}

	public ShortArray(int len) {
		_value_v = new short [len];
		for (int n = 0; n < length(); n++ ) _value_v[n] = 0;
	}

	public ShortArray(NumberArray array) { copy(array); }

	public void copy(short [] array) {
		_value_v = new short [array.length];
		for (int n = 0; n < length(); n++ ) _value_v[n] = (short)array[n];
	}

	public void copy(NumberArray array) {
		_value_v = new short [array.length()];
		for (int n = 0; n < length(); n++ ) _value_v[n] = (short)array.get(n);
	}

	public double get(int index) {
		if ( index >= 0 && index < length() ) return _value_v[index];
		else return -1;
	}

	public void set(int index, double data) {
		if ( index >= 0 && index < length() ) _value_v[index] = (short)data;
	}

	public void set(int index, short data) {
		if ( index >= 0 && index < length() ) _value_v[index] = data;
	}

	public int length() { return _value_v.length; }

	public void resize(int len) {
		_value_v = new short [len];
		for (int n = 0; n < length(); n++ ) _value_v[n] = 0;
	}

	public void writeSingle(double value, Writer writer) throws IOException {
		writer.writeShort((short)value);
	}

	public double readSingle(Reader reader) throws IOException {
		return reader.readShort();
	}

}
