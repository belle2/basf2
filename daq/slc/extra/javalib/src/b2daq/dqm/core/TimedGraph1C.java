package b2daq.dqm.core;

public class TimedGraph1C extends TimedGraph1 {

	public TimedGraph1C() {
		super();
		_data_v = new CharArray();
	}

	public TimedGraph1C(String name, String title, int nbins, double xmin, double xmax) {
		super(name, title, nbins, xmin, xmax);
		_data_v = new CharArray(nbins);
	}

	public String getDataType() {
		return "TGC";
	}

}
