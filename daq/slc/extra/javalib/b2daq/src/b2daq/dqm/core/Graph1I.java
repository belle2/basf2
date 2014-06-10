package b2daq.dqm.core;


public class Graph1I extends Graph1 {

	public Graph1I() {
		super();
		data = new IntArray(0);
	}
	public Graph1I(String name, String title, int nbinx, double xmin, double xmax,
			double ymin, double ymax) {
		super(name, title, nbinx, xmin, xmax, ymin, ymax);
		data = new IntArray(nbinx*2);
	}
	public Graph1I(Graph1I h) {
		super();
		data = new IntArray(0);
	}
	public String getDataType() {
		return "g1I";
	}

}
