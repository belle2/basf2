package b2daq.dqm.core;

import java.io.IOException;

import b2daq.core.Reader;
import b2daq.core.Writer;

public class DoubleArray extends NumberArray {

	private double[] _value_v;
	
	public DoubleArray() {
		_value_v = new double [0];
		for (int n = 0; n < length(); n++ ) _value_v[n] = 0;
	}

	public DoubleArray(int len) {
		_value_v = new double [len];
		for (int n = 0; n < length(); n++ ) _value_v[n] = 0;
	}

	public DoubleArray(NumberArray array) { copy(array); }

	public void copy(NumberArray array) {
		_value_v = new double [array.length()];
		for (int n = 0; n < length(); n++ ) _value_v[n] = array.get(n);
	}

	public void copy(double [] array) {
		_value_v = new double [array.length];
		for (int n = 0; n < length(); n++ ) _value_v[n] = array[n];
	}

	public double get(int index) {
		if ( index >= 0 && index < length() ) return _value_v[index];
		else return -1;
	}

	public void set(int index, double data) {
		if ( index >= 0 && index < length() ) _value_v[index] = data;
	}

	public int length() { return _value_v.length; }

	public void resize(int len) {
		_value_v = new double [len];
		for (int n = 0; n < length(); n++ ) _value_v[n] = 0;
	}

	public void writeSingle(double value, Writer writer) throws IOException {
		writer.writeDouble(value);
	}

	public double readSingle(Reader reader) throws IOException {
		return reader.readDouble();
	}

}
