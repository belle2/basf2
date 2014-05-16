package b2daq.dqm.core;

import java.io.IOException;

import b2daq.core.Reader;
import b2daq.core.Writer;

public class CharArray extends NumberArray {

	private char[] _value_v;
	
	public CharArray() {
		_value_v = new char [0];
		for (int n = 0; n < length(); n++ ) _value_v[n] = 0;
	}

	public CharArray(int len) {
		_value_v = new char [len];
		for (int n = 0; n < length(); n++ ) _value_v[n] = 0;
	}

	public CharArray(NumberArray array) { copy(array); }

	public void copy(NumberArray array) {
		_value_v = new char [array.length()];
		for (int n = 0; n < length(); n++ ) _value_v[n] = (char)array.get(n);
	}

	public void copy(char [] array) {
		_value_v = new char [array.length];
		for (int n = 0; n < length(); n++ ) _value_v[n] = array[n];
	}

	public double get(int index) {
		if ( index >= 0 && index < length() ) return _value_v[index];
		else return -1;
	}

	public char at(int index) {
		if ( index >= 0 && index < length() ) return _value_v[index];
		else return (char)-1;
	}

	public void set(int index, char data) {
		if ( index >= 0 && index < length() ) _value_v[index] = data;
	}

	public void set(int index, double data) {
		if ( index >= 0 && index < length() ) _value_v[index] = (char)data;
	}

	public int length() { return _value_v.length; }

	public void resize(int len) {
		_value_v = new char [len];
		for (int n = 0; n < length(); n++ ) _value_v[n] = 0;
	}

	public void writeSingle(double value, Writer writer) throws IOException {
		writer.writeChar((char)value);
	}

	public double readSingle(Reader reader) throws IOException {
		return reader.readChar();
	}

}
