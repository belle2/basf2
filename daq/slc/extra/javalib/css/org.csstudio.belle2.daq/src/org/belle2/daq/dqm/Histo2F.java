package org.belle2.daq.dqm;


public class Histo2F extends Histo2 {

	public Histo2F() {
		super();
		data = new FloatArray(0);
	}
	public Histo2F(String name, String title, int nbinx, double xmin, double xmax,
			int nbiny, double ymin, double ymax) {
		super(name, title, nbinx, xmin, xmax, nbiny, ymin, ymax);
		data = new FloatArray((nbinx+2)*(nbiny+2));
	}
	public Histo2F(Histo2F h) {
		super();
		data = new FloatArray(0);
	}
	public String getDataType() {
		return "H2F";
	}

}
