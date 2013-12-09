package b2dqm.core;

import java.util.Date;

import b2daq.core.Reader;
import b2daq.core.Writer;


public abstract class TimedGraph1 extends Histo {
	
	protected int _iter;
	protected LongArray _time_v;
	
	public TimedGraph1() {
		this("", "", 1, 0, 1);
	}

	public TimedGraph1(String name, String title, int nbinx, double xmin, double xmax) {
		super(name, title);
		_axis_x.setRange(nbinx, xmin, xmax);
		_time_v = new LongArray(nbinx);
	}

	public void reset() {
		_iter = 0;
		_data_v.setAll(0);
		_time_v.setAll(0);
		getAxisY().setMax(1);
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

	public double getPoint(int n) {
		return _data_v.get(n);
	}
	
	public void setPoint(int n, double data) { 
		_data_v.set(n, data);
	}
	
	public long getTime(int n) {
		return (long)_time_v.get(n);
	}
	
	public void setTime(int n, long time) { 
		_time_v.set(n, time);
	}
	
    public void addPoint(long time, double data) {
    	_iter++;
    	if ( _iter == _data_v.length() || 
    		 _iter == _time_v.length() ) {
    		_iter = 0;
    	}
    	setPoint(_iter, data);
    	setTime(_iter, time);
    }

    public void addPoint(double data) {
    	addPoint(new Date().getTime(), data);
    }
    	
    public double getLatestPoint() {
    	return _data_v.get(_iter);
    }
    
    public long getLatestTime() {
    	return (long)_time_v.get(_iter);
    }

    public void setMaxAndMin(){
		if ( ! _axis_y.isFixMax() || ! _axis_y.isFixMin()) {
			double data, min = getMinimum(), max = getMaximum(); 
			for ( int nx = 0;  nx < getAxisX().getNbins(); nx++) {
				data = getPoint(nx);
				if ( data > max ) max = data;
				if ( data < min ) min = data;
			}
			setMaximum((max+min)/2. + (max-min)/2.*1.05);
			setMinimum((max+min)/2. - (max-min)/2.*1.05);
		}
	}

	public String toString() {
		StringBuffer script = new StringBuffer();
		script.append("      {\n" + "        'datatype':'" + getDataType()
				+ "',\n" + "        'name':'" + getName() + "',\n"
				+ "        'title':'" + getTitle() + "',\n"
				+ "        'tabid':" + (int) getTabId() + ",\n"
				+ "        'positionid':" + (int) getPositionId() + ",\n"
				+ "        'axisx':" + getAxisX().toString() + ",\n"
				+ "        'axisy':" + getAxisY().toString() + ",\n"
				+ "        'axisz':" + getAxisZ().toString() + ",\n"
				+ "        'iter':" + _iter + "\n"  
				+ "        'time':" + _time_v.toString() + "\n"
				+ "        'data':" + _data_v.toString() + "\n" + "      }");
		return script.toString();
	}

	protected String getXML() {
		String str = "title=\"" + getTitle() + 
		"\" data=\"" + _data_v.toString() + 
		"\" iter=\"" + _iter + 
		"\" >\n";
		str += _axis_x.getXML("axis-x");
		str += _axis_y.getXML("axis-y");
		return str;
	}

	@Override
	public void readObject(Reader reader) throws Exception {
		readConfig(reader);
		_iter = reader.readInt() - 1;
		getData().readObject(reader);
		_time_v.readObject(reader);
	}

	@Override
	public void readContents(Reader reader) throws Exception {
		_iter = reader.readInt();
		_time_v.readObject(reader);
		getData().readObject(reader);
	}

	@Override
	public void readConfig(Reader reader) throws Exception {
		setTabId(reader.readChar());
		setPositionId(reader.readChar());
		setName(reader.readString());
		setTitle(reader.readString());
		setAxisX(readAxis(reader));
		getAxisY().setTitle(reader.readString());
		_data_v.resize(_axis_x.getNbins());
		_time_v.resize(_axis_x.getNbins());
	}

	@Override
	public void writeObject(Writer writer) throws Exception {
		writeConfig(writer);
		writer.writeInt(_iter);
		_data_v.writeObject(writer);
		_time_v.writeObject(writer);
	}

	@Override
	public void writeContents(Writer writer) throws Exception {
		writer.writeInt(_iter);
		_time_v.writeObject(writer);
		_data_v.writeObject(writer);
	}

	@Override
	public void writeConfig(Writer writer) throws Exception {
		super.writeConfig(writer);
		writeAxis(writer, getAxisX());
		writer.writeString(getAxisY().getTitle());
	}

	public int getIter() {
		return _iter;
	}

}
