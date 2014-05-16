package b2daq.dqm.core;


public class Histo1D extends Histo1 {

	public Histo1D() {
		super();
		_data_v = new DoubleArray(0);
	}
	public Histo1D(String name, String title, int nbinx, double xmin, double xmax) {
		super(name, title, nbinx, xmin, xmax);
		_data_v = new DoubleArray(nbinx+2);
	}
	public Histo1D(Histo1D h) {
		super();
		_data_v = new DoubleArray(0);
	}
	public String getDataType() {
		return "H1D";
	}

}
