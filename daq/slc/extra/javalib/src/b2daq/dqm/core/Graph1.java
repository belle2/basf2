package b2daq.dqm.core;

import java.io.IOException;

import b2daq.core.Reader;
import b2daq.core.Writer;

public abstract class Graph1 extends Histo {
	public Graph1() {
		super();
	}

	public Graph1(String name, String title, int nbinx, double xmin, double xmax,
			double ymin, double ymax) {
		super(name, title);
		_axis_x.setRange(nbinx, xmin, xmax);
		_axis_y.setRange(1, ymin, ymax);
	}

	public void fixMaximum(double data, boolean fix) {
		_axis_y.setMax(data);
		_axis_y.fixMax(fix);
	}

	public void fixMinimum(double data, boolean fix) {
		_axis_y.setMin(data);
		_axis_y.fixMin(fix);
	}

	public int getDim() { return 1; }
	public double getMaximum() { return _axis_y.getMax(); }
	public double getMinimum() { return _axis_y.getMin(); }
	public void setMaximum(double data) {
		if ( !_axis_y.isFixMax() ) _axis_y.setMax(data);
	}

	public void setMinimum(double data) { 
		if ( !_axis_y.isFixMin() ) _axis_y.setMin(data);
	}

	public double getPointX(int n) { return _data_v.get(n); }
	public double getPointY(int n) { return _data_v.get(_data_v.length()/2 + n); }
	public void setPointX(int n, double data) { _data_v.set(n, data); }
	public void setPointY(int n, double data) { _data_v.set(_data_v.length()/2 + n, data); }

	@Override
	public void setMaxAndMin(){
		if ( ! _axis_y.isFixMax() || ! _axis_y.isFixMin()) {
			double data, min = getMinimum(), max = getMaximum(); 
			for ( int nx = 0;  nx < _axis_x.getNbins(); nx++) {
				data = getPointY(nx);
				if ( data*1.05 > max ) max = data*1.05;
				if ( data*1.05 < min ) min = data*1.05;
			}
			if ( getMaximum() < max && getMinimum() > min ) {
				setMaximum((max+min)/2. + (max-min)/2.*1.05);
				setMinimum((max+min)/2. - (max-min)/2.*1.05);
			}
		}
	}

	@Override
	public void readConfig(Reader reader) throws IOException {
		setTabId(reader.readChar());
		setPositionId(reader.readChar());
		setName(reader.readString());
		setTitle(reader.readString());
		setAxisX(readAxis(reader));
		setAxisY(readAxis(reader));
		getData().resize(getAxisX().getNbins()*2);
	}

	@Override
	public void writeConfig(Writer writer) throws IOException {
		writer.writeString(getDataType());
		writer.writeChar((char)getTabId());
		writer.writeChar((char)getPositionId());
		writer.writeString(getName());
		writer.writeString(getTitle());
		writeAxis(writer, getAxisX());
		writeAxis(writer, getAxisY());
	}

	public Histo clone() {
		try {
			Histo h = (Histo) HistoFactory.create(getDataType());
			h.setName(getName());
			h.setTitle(getTitle());
			h.getAxisX().copy(getAxisX());
			h.getAxisY().copy(getAxisY());
			h.getData().copy(getData());
			return h;
		} catch (WrongDataTypeException e) {
			e.printStackTrace();
			return null;
		} 
	}


}
