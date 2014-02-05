package b2dqm.core;


public class Histo2C extends Histo2 {

	public Histo2C() {
		super();
		_data_v = new CharArray(0);
	}
	public Histo2C(String name, String title, int nbinx, double xmin, double xmax,
			int nbiny, double ymin, double ymax) {
		super(name, title, nbinx, xmin, xmax, nbiny, ymin, ymax);
		_data_v = new CharArray((nbinx+2)*(nbiny+2));
	}
	public Histo2C(Histo2C h) {
		super();
		_data_v = new CharArray(0);
	}
	public String getDataType() {
		return "H2C";
	}

}
