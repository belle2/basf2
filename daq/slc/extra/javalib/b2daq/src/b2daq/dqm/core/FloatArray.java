package b2daq.dqm.core;

import java.io.IOException;

import b2daq.core.Reader;
import b2daq.core.Writer;

public class FloatArray extends NumberArray {

	private float[] _value_v;
	
	public FloatArray() {
		_value_v = new float [0];
		for (int n = 0; n < length(); n++ ) _value_v[n] = 0;
	}

	public FloatArray(int len) {
		_value_v = new float [len];
		for (int n = 0; n < length(); n++ ) _value_v[n] = 0;
	}

	public FloatArray(NumberArray array) { copy(array); }

	public void copy(NumberArray array) {
		_value_v = new float [array.length()];
		for (int n = 0; n < length(); n++ ) _value_v[n] = (char)array.get(n);
	}

	public void copy(float [] array) {
		_value_v = new float [array.length];
		for (int n = 0; n < length(); n++ ) _value_v[n] = (char)array[n];
	}

	public double get(int index) {
		if ( index >= 0 && index < length() ) return _value_v[index];
		else return -1;
	}

	public void set(int index, float data) {
		if ( index >= 0 && index < length() ) _value_v[index] = data;
	}

	public void set(int index, double data) {
		if ( index >= 0 && index < length() ) _value_v[index] = (float)data;
	}

	public int length() { return _value_v.length; }

	public void resize(int len) {
		_value_v = new float [len];
		for (int n = 0; n < length(); n++ ) _value_v[n] = 0;
	}

	public void writeSingle(double value, Writer writer) throws IOException {
		writer.writeFloat((float)value);
	}

	public double readSingle(Reader reader) throws IOException {
		return reader.readFloat();
	}

}
