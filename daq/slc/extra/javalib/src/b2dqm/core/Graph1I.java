package b2dqm.core;


public class Graph1I extends Graph1 {

	public Graph1I() {
		super();
		_data_v = new IntArray(0);
	}
	public Graph1I(String name, String title, int nbinx, double xmin, double xmax,
			double ymin, double ymax) {
		super(name, title, nbinx, xmin, xmax, ymin, ymax);
		_data_v = new IntArray(nbinx*2);
	}
	public Graph1I(Graph1I h) {
		super();
		_data_v = new IntArray(0);
	}
	public String getDataType() {
		return "g1I";
	}

}
