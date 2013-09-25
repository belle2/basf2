package b2dqm.core;

public class Axis {

	private String _name = "";
	private int _nbins = 0;
	private double _min = 0;
	private double _max = 1;
	private boolean _is_fixed_min = false;
	private boolean _is_fixed_max = false;
	private String _title = null;

	public Axis(int nbin, double min, double max, String label){
		_nbins = nbin;	_min = min;	_max = max;	_title = label;
	}
	public Axis(){this(1, 0, 1, "");}
	public Axis(int nbin, double min, double max){ this(nbin, min,	max, ""); }
	public Axis(Axis axis){	this(axis._nbins, axis._min, axis._max, axis._title); }

	public void copy(Axis axis){
		_nbins = axis._nbins; _min = axis._min;
		_max = axis._max; _title = axis._title;
		_is_fixed_min= axis._is_fixed_min; _is_fixed_max = axis._is_fixed_max;
	}
	
	public void copyRange(Axis axis){
		if ( !_is_fixed_min ) _min = axis._min;
		if ( !_is_fixed_max ) _max = axis._max;
	}
	public int getNbins() { return _nbins; }
	public double getMax() { return _max; }
	public double getMin() { return _min; }
	public String getTitle() { return _title; }
	public boolean isFixMin() { return _is_fixed_min; }
	public boolean isFixMax() { return _is_fixed_max; }
	public void setRange(int nbin, double min, double max){
		_nbins = nbin;	_min = min;	_max = max;
	}
	public void setNbins(int nbins) { _nbins = nbins; }
	public void setMax(double max) { 
		if (!_is_fixed_max) _max = max;
	}
	public void setMin(double min) {
		if (!_is_fixed_min) _min = min;
	}
	public void setTitle(String label) { _title = label; }
	public void fixMin(boolean fix) { _is_fixed_min = fix; }
	public void fixMax(boolean fix) { _is_fixed_max = fix; }

	public void setName(String name) {
		_name = name;
	}

	public String getName() {
		return _name;
	}

	public String toString() {
		String script = "{"
			+ "'label':'"+getTitle() + "',"
			+ "'nbins':" + getNbins() + ","
			+ "'min':" + getMin() + ","
			+ "'max':" + getMax() + "}";
		return script;
	}
	
	public String getXML(String name) {
		String script = "<" + name + " "
			+ "label=\"" + getTitle() + "\" "
			+ "nbins=\"" + getNbins() + "\" "
			+ "min=\"" + getMin() + "\" "
			+ "max=\"" + getMax() + "\" ";
		if ( isFixMin() ) script += "fixed-min=\"" + isFixMin() + "\" ";
		if ( isFixMax() ) script += "fixed-max=\"" + isFixMax() + "\" ";
		script += "/>\n";
		return script;
	}

}
