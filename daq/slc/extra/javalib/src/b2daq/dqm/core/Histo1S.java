package b2daq.dqm.core;


public class Histo1S extends Histo1 {

	public Histo1S() {
		super();
		_data_v = new ShortArray(0);
	}
	public Histo1S(String name, String title, int nbinx, double xmin, double xmax) {
		super(name, title, nbinx, xmin, xmax);
		_data_v = new ShortArray(nbinx+2);
	}
	public Histo1S(Histo1S h) {
		super();
		_data_v = new ShortArray(0);
	}
	public String getDataType() {
		return "H1S";
	}
	
}
