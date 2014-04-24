package b2dqm.core;

import b2daq.core.Reader;
import b2daq.core.Writer;

public abstract class Histo1 extends Histo {

	public Histo1() {
		super();
	}

	public Histo1(String name, String title, int nbinx, double xmin, double xmax) {
		super(name, title);
		_axis_x.setRange(nbinx, xmin, xmax);
	}

	public void fixMaximum(double data, boolean fix) {
		_axis_y.setMax(data);
		_axis_y.fixMax(fix);
	}

	public void fixMinimum(double data, boolean fix) {
		_axis_y.setMin(data);
		_axis_y.fixMin(fix);
	}

/*
 	public void reset() {
		super.reset();
		setMaximum(1);
	}
*/
	
	public int getDim() { return 1; }
	public double getMaximum() { return _axis_y.getMax(); }
	public double getMinimum() { return _axis_y.getMin(); }
	public void setMaximum(double data) {
		if ( !_axis_y.isFixMax() ) _axis_y.setMax(data);
	}

	public void setMinimum(double data) { 
		if ( !_axis_y.isFixMin() ) _axis_y.setMin(data);
	}

	public double getBinContent(int nx) { return _data_v.get(nx+1); }
	public double getOverFlow() { return _data_v.get(getAxisX().getNbins()+1); }
	public double getUnderFlow() { return _data_v.get(0); }
	public void setBinContent(int nx, double data) {
		if ( nx >= 0 && nx < _axis_x.getNbins() ) {
			_data_v.set(nx+1, data);
			if ( data * 1.1 > getMaximum() ) setMaximum(data * 1.1);
		}
	}
	public void setOverFlow(double data) { _data_v.set(getAxisX().getNbins()+1, data); }
	public void setUnderFlow(double data) { _data_v.set(0, data); }
    
	public void setMaxAndMin(){
		if ( ! _axis_y.isFixMax() || ! _axis_y.isFixMin()) {
			double data, data_max = getBinContent(0); 
			for ( int nx = 0;  nx < _axis_x.getNbins(); nx++) {
				data = getBinContent(nx);
				if ( data > data_max ) data_max = data;
			}
			setMaximum(data_max * 1.1);
		}
	}

	@Override
	public void readConfig(Reader reader) throws Exception {
		setTabId(reader.readChar());
		setPositionId(reader.readChar());
		setName(reader.readString());
		setTitle(reader.readString());
		setAxisX(readAxis(reader));
		getAxisY().setTitle(reader.readString());
		_data_v.resize(getAxisX().getNbins()+2);
	}

	@Override
	public void writeConfig(Writer writer) throws Exception {
		writer.writeString(getDataType());
		writer.writeChar((char)getTabId());
		writer.writeChar((char)getPositionId());
		writer.writeString(getName());
		writer.writeString(getTitle());
		writeAxis(writer, getAxisX());
		writer.writeString(getAxisY().getTitle());
	}

	public Histo clone() {
		try {
			Histo1 h = (Histo1) HistoFactory.create(getDataType());
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

	public void add(Histo h, double scale) {
		if (h.getAxisX().getNbins() == getAxisX().getNbins() &&
			 h.getAxisX().getMin() == getAxisX().getMin() && 
			 h.getAxisX().getMax() == getAxisX().getMax() ) {
			for (int nx = 0; nx < getAxisX().getNbins(); nx++) {
				setBinContent(nx, getBinContent(nx) + h.getBinContent(nx) * scale);
			}
		}
		setMaxAndMin();
	}

}
