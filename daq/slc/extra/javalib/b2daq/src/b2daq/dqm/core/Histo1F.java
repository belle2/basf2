package b2daq.dqm.core;


public class Histo1F extends Histo1 {

	public Histo1F() {
		super();
		data = new FloatArray(0);
	}
	public Histo1F(String name, String title, int nbinx, double xmin, double xmax) {
		super(name, title, nbinx, xmin, xmax);
		data = new FloatArray(nbinx+2);
	}
	public Histo1F(Histo1F h) {
		super();
		data = new FloatArray(0);
	}
	public String getDataType() {
		return "H1F";
	}

}
