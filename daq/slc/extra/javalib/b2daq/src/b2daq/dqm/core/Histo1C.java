package b2daq.dqm.core;

public class Histo1C extends Histo1 {

    public Histo1C() {
        super();
        data = new CharArray(0);
    }

    public Histo1C(String name, String title, int nbinx, double xmin, double xmax) {
        super(name, title, nbinx, xmin, xmax);
        data = new CharArray(nbinx + 2);
    }

    public Histo1C(Histo1C h) {
        super();
        data = new CharArray(0);
    }

    @Override
    public String getDataType() {
        return "H1C";
    }

}
