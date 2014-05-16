package b2daq.dqm.core;


public abstract class AbstractHisto extends MonObject {

	protected String _title;
	protected Axis _axis_x;
	protected Axis _axis_y;
	protected Axis _axis_z;
	protected NumberArray _data_v;
	protected long _time;
	
	public AbstractHisto() {
		this("","");
	}
	
	public AbstractHisto(String name, String title) {
		_position_id = 0;
		_tab_id = 0;
		_name = new String(name);
		String [] title_v = title.split(";");
		_title = new String("");
		_axis_x = new Axis(1, 0, 1, "");
		_axis_y = new Axis(1, 0, 1, "");
		_axis_z = new Axis(1, 0, 1, "");
		if ( title_v.length > 0 ) _title = title_v[0];
		if ( title_v.length > 1 ) _axis_x.setTitle(title_v[1]);
		if ( title_v.length > 2 ) _axis_y.setTitle(title_v[2]);
		if ( title_v.length > 3 ) _axis_z.setTitle(title_v[3]);
	}

	public void setTime(long time) {
		_time = time;
	}

	public long getTime() {
		return _time;
	}
    
    public String getTitle() {  return _title;  }
    public Axis getAxisX() {  return _axis_x;  }
    public Axis getAxisY() {  return _axis_y;  }
    public Axis getAxisZ() {  return _axis_z;  }

    public void setTitle(String title) {
		String [] title_v = title.split(";");
		if ( title_v.length > 0 ) _title = title_v[0];
		else _title = title;
		if ( title_v.length > 1 ) _axis_x.setTitle(title_v[1]);
		if ( title_v.length > 2 ) _axis_y.setTitle(title_v[2]);
		if ( title_v.length > 3 ) _axis_z.setTitle(title_v[3]);
    }
    public void setAxisX(Axis axis) { _axis_x.copy(axis);  }
    public void setAxisY(Axis axis) { _axis_y.copy(axis);  }
    public void setAxisZ(Axis axis) { _axis_z.copy(axis);  }
    
    public double getLatestPoint() { return 0; }
    public double getPoint(int nx) { return 0; }
    public double getPointX(int nx) { return 0; }
    public double getPointY(int nx) { return 0; }
    public double getBinContent(int nx) { return 0; }
    public double getBinContent(int nx, int ny) { return 0; }
    public double getOverFlow() { return 0; }
    public double getOverFlowX(int ny) { return 0; }
    public double getOverFlowY(int nx) { return 0; }
    public double getUnderFlow() { return 0; }
    public double getUnderFlowX(int ny) { return 0; }
    public double getUnderFlowY(int nx) { return 0; }
    public void addPoint(double data) {}
    public void setPoint(int nx, double data) {}
    public void setPointX(int nx, double data) {}
    public void setPointY(int nx, double data) {}
    public void setBinContent(int nx, double data) {}
    public void setBinContent(int nx, int ny, double data) {}
    public void setOverFlow(double data) {}
    public void setOverFlowX(int ny, double data) {}
    public void setOverFlowY(int nx, double data) {}
    public void setUnderFlow(double data) {}
    public void setUnderFlowX(int ny, double data) {}
    public void setUnderFlowY(int nx, double data) {}

    public abstract double getMaximum();
    public abstract double getMinimum();
    public abstract void setMaximum(double data);
    public abstract void setMinimum(double data);
    public abstract void fixMaximum(double data, boolean fix);
    public abstract void fixMinimum(double data, boolean fix);
	    
    public abstract int getDim();
    public abstract String getDataType();

	public abstract void setMaxAndMin();

	public abstract void fill(double vx);
	public abstract void fill(double vx, double vy);
	public abstract double getMean();
	public abstract double getRMS();

}
