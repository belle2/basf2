package b2daq.dqm.core;

public class TimedGraph1D extends TimedGraph1 {

	public TimedGraph1D() {
		super();
		data = new DoubleArray();
	}

	public TimedGraph1D(String name, String title, int nbins, double xmin, double xmax) {
		super(name, title, nbins, xmin, xmax);
		data = new DoubleArray(nbins);
	}

	public String getDataType() {
		return "TGD";
	}

}
