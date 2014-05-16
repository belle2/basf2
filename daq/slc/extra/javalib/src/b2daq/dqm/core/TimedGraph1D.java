package b2daq.dqm.core;

public class TimedGraph1D extends TimedGraph1 {

	public TimedGraph1D() {
		super();
		_data_v = new DoubleArray();
	}

	public TimedGraph1D(String name, String title, int nbins, double xmin, double xmax) {
		super(name, title, nbins, xmin, xmax);
		_data_v = new DoubleArray(nbins);
	}

	public String getDataType() {
		return "TGD";
	}

}
