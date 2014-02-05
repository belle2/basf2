package b2dqm.core;

import b2daq.core.Reader;
import b2daq.core.Writer;

public class TimeArray {

	private long[] _value_v;
	private int _iter = 0;
	
	public TimeArray() {
		_value_v = new long [0];
		for (int n = 0; n < length(); n++ ) _value_v[n] = 0;
	}

	public TimeArray(int len) {
		_value_v = new long [len];
		for (int n = 0; n < length(); n++ ) _value_v[n] = 0;
	}

	public TimeArray(NumberArray array) { copy(array); }

	public void copy(NumberArray array) {
		if( _value_v.length != array.length() ) _value_v = new long [array.length()];
		for (int n = 0; n < length(); n++ ) _value_v[n] = (int)array.get(n);
	}

	public void copy(int [] array) {
		if( _value_v.length != array.length ) _value_v = new long [array.length];
		for (int n = 0; n < length(); n++ ) _value_v[n] = array[n];
	}

	public long get(int index) {
		if ( index < 0 ) return -1;
		else return _value_v[(_iter+index)%_value_v.length];
	}

	public void set(int index, long data) {
		if ( index >= 0 ) _value_v[(_iter+index)%_value_v.length] = data;
	}

	public void add(long value) {
		_iter++;
		if ( _iter == _value_v.length ) _iter = 0;
		_value_v[_iter] = value;
	}
	
	public int length() { return _value_v.length; }

	public void resize(int len) {
		_value_v = new long [len];
		for (int n = 0; n < length(); n++ ) {
			_value_v[n] = 0;	
		}
		_iter = 0;
	}

	public void writeSingle(double value, Writer writer) throws Exception {
		writer.writeInt((int)value);
	}

	public double readSingle(Reader reader) throws Exception {
		return reader.readInt();
	}

}
