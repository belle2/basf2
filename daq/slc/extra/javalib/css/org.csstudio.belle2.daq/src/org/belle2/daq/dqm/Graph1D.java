package org.belle2.daq.dqm;


public class Graph1D extends Graph1 {

	public Graph1D() {
		super();
		data = new DoubleArray(0);
	}
	public Graph1D(String name, String title, int nbinx, double xmin, double xmax,
			double ymin, double ymax) {
		super(name, title, nbinx, xmin, xmax, ymin, ymax);
		data = new DoubleArray(nbinx*2);
	}
	public Graph1D(Graph1D h) {
		super();
		data = new DoubleArray(0);
	}
	public String getDataType() {
		return "g1D";
	}

}
