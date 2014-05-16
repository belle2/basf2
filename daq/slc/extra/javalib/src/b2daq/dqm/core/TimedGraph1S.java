package b2daq.dqm.core;

public class TimedGraph1S extends TimedGraph1 {

	public TimedGraph1S() {
		super();
		_data_v = new ShortArray();
	}

	public TimedGraph1S(String name, String title, int nbins, double xmin, double xmax) {
		super(name, title, nbins, xmin, xmax);
		_data_v = new ShortArray(nbins);
	}

	public String getDataType() {
		return "TGS";
	}

}
