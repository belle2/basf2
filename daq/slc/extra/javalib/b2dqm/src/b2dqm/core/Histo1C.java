package b2dqm.core;

public class Histo1C extends Histo1 {

	public Histo1C() {
		super();
		_data_v = new CharArray(0);
	}
	public Histo1C(String name, String title, int nbinx, double xmin, double xmax) {
		super(name, title, nbinx, xmin, xmax);
		_data_v = new CharArray(nbinx+2);
	}
	public Histo1C(Histo1C h) {
		super();
		_data_v = new CharArray(0);
	}
	public String getDataType() {
		return "H1C";
	}
	
}
