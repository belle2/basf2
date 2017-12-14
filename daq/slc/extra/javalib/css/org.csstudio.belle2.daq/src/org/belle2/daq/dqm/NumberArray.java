package org.belle2.daq.dqm;

import java.io.IOException;

import org.belle2.daq.base.Reader;
import org.belle2.daq.base.Serializable;
import org.belle2.daq.base.Writer;

public abstract class NumberArray implements Serializable {
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

	public void readObject(Reader reader) throws IOException {
		for ( int n = 0; n < length(); n++ ) {
			set(n, readSingle(reader));
		}
	}
	
	abstract public double readSingle(Reader reader) throws IOException;

	public void writeObject(Writer writer) throws IOException {
		for ( int n = 0; n < length(); n++ ) {
			writeSingle(get(n), writer);
		}
	}
	
	public void setAll(double v) {
		for ( int n = 0; n < length(); n++ ) {
			set(n, v);
		}
	}

	abstract public void writeSingle(double v, Writer writer) throws IOException;
	
	public String toString() {
		StringBuffer ss = new StringBuffer();
		ss.append("");
		for ( int n = 0; n < length(); n++ ) {
			ss.append(get(n));
			if ( n < length() - 1 ) ss.append(" ");
		}
		ss.append("");
		return ss.toString();
	}
	
}
