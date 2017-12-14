package b2daq.dqm.core;


public class Histo2D extends Histo2 {

	public Histo2D() {
		super();
		data = new DoubleArray(0);
	}
	public Histo2D(String name, String title, int nbinx, double xmin, double xmax,
			int nbiny, double ymin, double ymax) {
		super(name, title, nbinx, xmin, xmax, nbiny, ymin, ymax);
		data = new DoubleArray((nbinx+2)*(nbiny+2));
	}
	public Histo2D(Histo2D h) {
		super();
		data = new DoubleArray(0);
	}
	public String getDataType() {
		return "H2D";
	}

}
