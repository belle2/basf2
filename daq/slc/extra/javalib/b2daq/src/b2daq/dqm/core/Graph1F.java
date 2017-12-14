package b2daq.dqm.core;


public class Graph1F extends Graph1 {

	public Graph1F() {
		super();
		data = new FloatArray(0);
	}
	public Graph1F(String name, String title, int nbinx, double xmin, double xmax,
			double ymin, double ymax) {
		super(name, title, nbinx, xmin, xmax, ymin, ymax);
		data = new FloatArray(nbinx*2);
	}
	public Graph1F(Graph1F h) {
		super();
		data = new FloatArray(0);
	}
	public String getDataType() {
		return "g1F";
	}

}
