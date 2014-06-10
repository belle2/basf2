package b2daq.dqm.core;


public class Histo2C extends Histo2 {

	public Histo2C() {
		super();
		data = new CharArray(0);
	}
	public Histo2C(String name, String title, int nbinx, double xmin, double xmax,
			int nbiny, double ymin, double ymax) {
		super(name, title, nbinx, xmin, xmax, nbiny, ymin, ymax);
		data = new CharArray((nbinx+2)*(nbiny+2));
	}
	public Histo2C(Histo2C h) {
		super();
		data = new CharArray(0);
	}
	public String getDataType() {
		return "H2C";
	}

}
