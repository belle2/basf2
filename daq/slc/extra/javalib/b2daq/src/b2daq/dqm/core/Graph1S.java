package b2daq.dqm.core;


public class Graph1S extends Graph1 {

	public Graph1S() {
		super();
		data = new ShortArray(0);
	}
	public Graph1S(String name, String title, int nbinx, double xmin, double xmax,
			double ymin, double ymax) {
		super(name, title, nbinx, xmin, xmax, ymin, ymax);
		data = new ShortArray(nbinx*2);
	}
	public Graph1S(Graph1S h) {
		super();
		data = new ShortArray(0);
	}
	public String getDataType() {
		return "g1S";
	}

}
