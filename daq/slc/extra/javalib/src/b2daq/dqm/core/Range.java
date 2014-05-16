package b2daq.dqm.core;

import java.io.IOException;

import b2daq.core.Reader;
import b2daq.core.Writer;

public abstract class Range extends Histo {
		
	public Range (String name, double min, double max) {
		super(name, name);
		_data_v = new DoubleArray(2);
		_data_v.set(0, min);
		_data_v.set(1, max);
	}
	
	public void set(double min, double max) {
		_data_v.set(0, min);
		_data_v.set(1, max);
	}
	
	@Override
	public void fixMaximum(double data, boolean fix) {
		setMaximum(data);
	}

	@Override
	public void fixMinimum(double data, boolean fix) {
		setMinimum(data);
	}

	public void reset() {}

	@Override
	public int getDim() {
		return 0;
	}

	@Override
	public double getMaximum() {
		return _data_v.get(1);
	}

	@Override
	public double getMinimum() {
		return _data_v.get(0);
	}

	@Override
	public void setMaxAndMin() {}

	@Override
	public void setMaximum(double data) {
		_data_v.set(1, data);
	}

	@Override
	public void setMinimum(double data) {
		_data_v.set(0, data);
	}

	@Override
	public void readConfig(Reader reader) throws IOException {
		setTabId(reader.readChar());
		setPositionId(reader.readChar());
		setName(reader.readString());
		setMinimum(reader.readDouble());
		setMaximum(reader.readDouble());
	}

	@Override
	public void writeConfig(Writer writer) throws IOException {
		writer.writeString(getDataType());
		writer.writeChar((char)getTabId());
		writer.writeChar((char)getPositionId());
		writer.writeString(getName());
		writer.writeDouble(getMinimum());
		writer.writeDouble(getMaximum());
	}

	public void readUpdate(Reader reader) throws IOException {}
	public void writeUpdate(Writer writer) throws IOException {}

}
