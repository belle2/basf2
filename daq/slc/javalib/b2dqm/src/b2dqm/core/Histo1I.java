package b2dqm.core;


public class Histo1I extends Histo1 {

	public Histo1I() {
		super();
		_data_v = new IntArray(0);
	}
	public Histo1I(String name, String title, int nbinx, double xmin, double xmax) {
		super(name, title, nbinx, xmin, xmax);
		_data_v = new IntArray(nbinx+2);
	}
	public Histo1I(Histo1I h) {
		super();
		_data_v = new IntArray(0);
	}
	public String getDataType() {
		return "H1I";
	}
	
}
