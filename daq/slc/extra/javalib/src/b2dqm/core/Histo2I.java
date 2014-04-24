package b2dqm.core;


public class Histo2I extends Histo2 {

	public Histo2I() {
		super();
		_data_v = new IntArray(0);
	}
	public Histo2I(String name, String title, int nbinx, double xmin, double xmax,
			int nbiny, double ymin, double ymax) {
		super(name, title, nbinx, xmin, xmax, nbiny, ymin, ymax);
		_data_v = new IntArray((nbinx+2)*(nbiny+2));
	}
	public Histo2I(Histo2I h) {
		super();
		_data_v = new IntArray(0);
	}
	public String getDataType() {
		return "H2I";
	}

}
