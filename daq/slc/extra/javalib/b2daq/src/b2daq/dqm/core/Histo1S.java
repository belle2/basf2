package b2daq.dqm.core;


public class Histo1S extends Histo1 {

	public Histo1S() {
		super();
		data = new ShortArray(0);
	}
	public Histo1S(String name, String title, int nbinx, double xmin, double xmax) {
		super(name, title, nbinx, xmin, xmax);
		data = new ShortArray(nbinx+2);
	}
	public Histo1S(Histo1S h) {
		super();
		data = new ShortArray(0);
	}
	public String getDataType() {
		return "H1S";
	}
	
}
