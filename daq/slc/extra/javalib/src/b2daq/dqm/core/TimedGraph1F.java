package b2daq.dqm.core;

public class TimedGraph1F extends TimedGraph1 {

	public TimedGraph1F() {
		super();
		_data_v = new FloatArray();
	}

	public TimedGraph1F(String name, String title, int nbins, double xmin, double xmax) {
		super(name, title, nbins, xmin, xmax);
		_data_v = new FloatArray(nbins);
	}

	public String getDataType() {
		return "TGF";
	}

}
