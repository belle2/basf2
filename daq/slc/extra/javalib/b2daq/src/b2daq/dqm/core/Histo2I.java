package b2daq.dqm.core;


public class Histo2I extends Histo2 {

	public Histo2I() {
		super();
		data = new IntArray(0);
	}
	public Histo2I(String name, String title, int nbinx, double xmin, double xmax,
			int nbiny, double ymin, double ymax) {
		super(name, title, nbinx, xmin, xmax, nbiny, ymin, ymax);
		data = new IntArray((nbinx+2)*(nbiny+2));
	}
	public Histo2I(Histo2I h) {
		super();
		data = new IntArray(0);
	}
	public String getDataType() {
		return "H2I";
	}

}
