package b2daq.dqm.core;

import java.io.IOException;

import b2daq.core.Reader;
import b2daq.core.Writer;

public class IntArray extends NumberArray {

	private int[] _value_v;
	
	public IntArray() {
		_value_v = new int [0];
		for (int n = 0; n < length(); n++ ) _value_v[n] = 0;
	}

	public IntArray(int len) {
		_value_v = new int [len];
		for (int n = 0; n < length(); n++ ) _value_v[n] = 0;
	}

	public IntArray(NumberArray array) { copy(array); }

	public void copy(NumberArray array) {
		if( _value_v.length != array.length() ) _value_v = new int [array.length()];
		for (int n = 0; n < length(); n++ ) _value_v[n] = (int)array.get(n);
	}

	public void copy(int [] array) {
		if( _value_v.length != array.length ) _value_v = new int [array.length];
		for (int n = 0; n < length(); n++ ) _value_v[n] = array[n];
	}

	public double get(int index) {
		if ( index >= 0 && index < length() ) return _value_v[index];
		else return -1;
	}

	public void set(int index, double data) {
		if ( index >= 0 && index < length() ) _value_v[index] = (int)data;
	}

	public void set(int index, int data) {
		if ( index >= 0 && index < length() ) _value_v[index] = data;
	}

	public int length() { return _value_v.length; }

	public void resize(int len) {
		_value_v = new int [len];
		for (int n = 0; n < length(); n++ ) _value_v[n] = 0;
	}

	public void writeSingle(double value, Writer writer) throws IOException {
		writer.writeInt((int)value);
	}

	public double readSingle(Reader reader) throws IOException {
		return reader.readInt();
	}

}
