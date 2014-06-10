package b2daq.dqm.core;


public class Histo2S extends Histo2 {

	public Histo2S() {
		super();
		data = new ShortArray(0);
	}
	public Histo2S(String name, String title, int nbinx, double xmin, double xmax,
			int nbiny, double ymin, double ymax) {
		super(name, title, nbinx, xmin, xmax, nbiny, ymin, ymax);
		data = new ShortArray((nbinx+2)*(nbiny+2));
	}
	public Histo2S(Histo2S h) {
		super();
		data = new ShortArray(0);
	}
	public String getDataType() {
		return "H2S";
	}

}
