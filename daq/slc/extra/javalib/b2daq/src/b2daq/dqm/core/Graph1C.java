package b2daq.dqm.core;


public class Graph1C extends Graph1 {

	public Graph1C() {
		super();
		data = new CharArray(0);
	}
	public Graph1C(String name, String title, int nbinx, double xmin, double xmax,
			double ymin, double ymax) {
		super(name, title, nbinx, xmin, xmax, ymin, ymax);
		data = new CharArray(nbinx*2);
	}
	public Graph1C(Graph1C h) {
		super();
		data = new CharArray(0);
	}
	public String getDataType() {
		return "g1C";
	}

}
