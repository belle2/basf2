package org.belle2.daq.dqm;

public class Histo1D extends Histo1 {

    public Histo1D() {
        super();
        data = new DoubleArray(0);
    }

    public Histo1D(String name, String title, int nbinx, double xmin, double xmax) {
        super(name, title, nbinx, xmin, xmax);
        data = new DoubleArray(nbinx + 2);
    }

    @Override
    public String getDataType() {
        return "H1D";
    }

}
