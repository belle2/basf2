package b2daq.dqm.core;

public class TimedGraph1I extends TimedGraph1 {

	public TimedGraph1I() {
		super();
		data = new IntArray();
	}

	public TimedGraph1I(String name, String title, int nbins, double xmin, double xmax) {
		super(name, title, nbins, xmin, xmax);
		data = new IntArray(nbins);
	}

	public String getDataType() {
		return "TGI";
	}

}
