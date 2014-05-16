package b2daq.dqm.core;

import java.io.IOException;

import b2daq.core.Reader;
import b2daq.core.Writer;

public abstract class Histo2 extends Histo {

	private boolean _stat = true;
	
	public Histo2() {
		super();
	}

	public Histo2(String name, String title, int nbinx, double xmin, double xmax
			, int nbiny, double ymin, double ymax) {
		super(name, title);
		_axis_x.setRange(nbinx, xmin, xmax);
		_axis_y.setRange(nbiny, ymin, ymax);
	}
/*
	public void reset() {
		super.reset();
		setMaximum(1);
	}
*/	
	public boolean getStat() { return _stat; }
	public void setStat(boolean stat) { _stat = stat; }
	
	public void fixMaximum(double data, boolean fix) {
		_axis_z.setMax(data);
		_axis_z.fixMax(fix);
	}

	public void fixMinimum(double data, boolean fix) {
		_axis_z.setMin(data);
		_axis_z.fixMin(fix);
	}
	public double getBinContent(int nx, int ny) { 
		return _data_v.get((nx+1) + (ny+1)*(_axis_x.getNbins()+2));
	}
	public double getOverFlow() { return _data_v.get(_data_v.length() - 1); }
	public double getUnderFlow() { return _data_v.get(0); }
	public double getOverFlowX(int ny) { return _data_v.get(_data_v.length() - 1); }
	public double getUnderFlowX(int ny) { return _data_v.get(0); }
	public double getOverFlowY(int nx) { return _data_v.get(_data_v.length() - 1); }
	public double getUnderFlowY(int nx) { return _data_v.get(0); }
	public void setBinContent(int nx, int ny, double data) {
		if ( nx >= 0 && nx < _axis_x.getNbins() && ny >= 0 && ny < _axis_y.getNbins()) {
			_data_v.set((nx+1) + (ny+1)*(_axis_x.getNbins()+2), data);
			if ( data * 1.05 > getMaximum() ) setMaximum(data * 1.05);
		}
	}
	public void setOverFlow(double data) { _data_v.set(_data_v.length() - 1, data); }
	public void setUnderFlow(double data) { _data_v.set(0, data); }
    
	public int getDim() { return 2; }
	public double getMaximum() { return _axis_z.getMax(); }
	public double getMinimum() { return _axis_z.getMin(); }
	public void setMaximum(double data) {
		if ( !_axis_z.isFixMax() ) _axis_z.setMax(data);
	}
	public void setMinimum(double data) {
		if ( !_axis_z.isFixMin() ) _axis_z.setMin(data);
	}

	public void setMaxAndMin(){
		if ( ! _axis_y.isFixMax() || ! _axis_y.isFixMin()) {
			double data; 
			for ( int ny = 0;  ny < _axis_y.getNbins(); ny++) {
				for ( int nx = 0;  nx < _axis_x.getNbins(); nx++) {
					data = getBinContent(nx, ny);
					if ( data * 1.05 > getMaximum() ) setMaximum(data * 1.05);
				}
			}
		}
	}

	public double getMeanX() {
		int entries = 0;
		double bin = 0;
		double mean = 0;
		for (int nx = 0; nx < _axis_x.getNbins(); nx++) {
			bin = 0;
			for (int ny = 0; ny < _axis_y.getNbins(); ny++) {
				bin += getBinContent(nx, ny);
			}
			mean += bin * ((nx + 0.5) * (_axis_x.getMax() - _axis_x.getMin())
					/ _axis_x.getNbins() + _axis_x.getMin());
			entries += bin;
		}
		return mean / entries;
	}
	
	public double getMeanY() {
		int entries = 0;
		double bin = 0;
		double mean = 0;
		for (int ny = 0; ny < _axis_y.getNbins(); ny++) {
			bin = 0;
			for (int nx = 0; nx < _axis_x.getNbins(); nx++) {
				bin += getBinContent(nx, ny);
			}
			mean += bin * ((ny + 0.5) * (_axis_y.getMax() - _axis_y.getMin())
					/ _axis_y.getNbins() + _axis_y.getMin());
			entries += bin;
		}
		return mean / entries;
	}
	
	public double getEntries() {
		int entries = 0;
		for ( int ny = 0;  ny < _axis_y.getNbins(); ny++) {
			for ( int nx = 0;  nx < _axis_x.getNbins(); nx++) {
				entries += getBinContent(nx, ny);
			}
		}
		return entries;
	}

	@Override
	public void readConfig(Reader reader) throws IOException {
		setTabId(reader.readChar());
		setPositionId(reader.readChar());
		setName(reader.readString());
		setTitle(reader.readString());
		setAxisX(readAxis(reader));
		setAxisY(readAxis(reader));
		_data_v.resize((getAxisX().getNbins()+2)*(getAxisY().getNbins()+2));
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

	protected String getXML() {
		String str = "title=\"" + getTitle() + 
		"\" data=\"" + _data_v.toString() + 
		"\" >\n";
		str += _axis_x.getXML("axis-x");
		str += _axis_y.getXML("axis-y");
		str += _axis_z.getXML("axis-z");
		return str;
	}

	public Histo clone() {
		try {
			Histo2 h = (Histo2) HistoFactory.create(getDataType());
			h.setName(getName());
			h.setTitle(getTitle());
			h.getAxisX().copy(getAxisX());
			h.getAxisY().copy(getAxisY());
			h.getAxisZ().copy(getAxisZ());
			h.getData().copy(getData());
			return h;
		} catch (WrongDataTypeException e) {
			e.printStackTrace();
			return null;
		} 
	}

	public void add(Histo h, double scale) {
		if (h.getAxisY().getNbins() == getAxisY().getNbins() &&
				h.getAxisY().getMin() == getAxisY().getMin() && 
				h.getAxisY().getMax() == getAxisY().getMax() &&
				h.getAxisX().getNbins() == getAxisX().getNbins() &&
				h.getAxisX().getMin() == getAxisX().getMin() && 
				h.getAxisX().getMax() == getAxisX().getMax() ) {
			for (int ny = 0; ny < getAxisY().getNbins(); ny++) {
				for (int nx = 0; nx < getAxisX().getNbins(); nx++) {
					setBinContent(nx, ny, getBinContent(nx, ny) + h.getBinContent(nx, ny) * scale);
				}
			}
		}
	}

}
