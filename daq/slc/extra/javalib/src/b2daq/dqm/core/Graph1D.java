package b2daq.dqm.core;


public class Graph1D extends Graph1 {

	public Graph1D() {
		super();
		_data_v = new DoubleArray(0);
	}
	public Graph1D(String name, String title, int nbinx, double xmin, double xmax,
			double ymin, double ymax) {
		super(name, title, nbinx, xmin, xmax, ymin, ymax);
		_data_v = new DoubleArray(nbinx*2);
	}
	public Graph1D(Graph1D h) {
		super();
		_data_v = new DoubleArray(0);
	}
	public String getDataType() {
		return "g1D";
	}

}
