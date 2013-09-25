package b2dqm.core;

import b2daq.core.Reader;
import b2daq.core.SerializableObject;
import b2daq.core.Writer;

public abstract class NumberArray implements SerializableObject {
	abstract public int length();
	abstract public double get(int index);
	abstract public void set(int index, double data);
	abstract public void resize(int len);
	abstract public void copy(NumberArray array);

	public void set(int index, char data) {}
	public void set(int index, short data) {}
	public void set(int index, int data) {}
	public void set(int index, float data) {}
	public void copy(char [] array) {}
	public void copy(short [] array) {}
	public void copy(int [] array) {}
	public void copy(float [] array) {}
	public void copy(double [] array) {}

	public void readObject(Reader reader) throws Exception {
		for ( int n = 0; n < length(); n++ ) {
			set(n, readSingle(reader));
		}
	}
	
	abstract public double readSingle(Reader reader) throws Exception;

	public void writeObject(Writer writer) throws Exception {
		for ( int n = 0; n < length(); n++ ) {
			writeSingle(get(n), writer);
		}
	}
	
	public void setAll(double v) {
		for ( int n = 0; n < length(); n++ ) {
			set(n, v);
		}
	}

	abstract public void writeSingle(double v, Writer writer) throws Exception;
	
	public String toString() {
		StringBuffer ss = new StringBuffer();
		ss.append("");
		for ( int n = 0; n < length(); n++ ) {
			ss.append(get(n));
			if ( n < length() - 1 ) ss.append(",");
		}
		ss.append("");
		return ss.toString();
	}
	
}
