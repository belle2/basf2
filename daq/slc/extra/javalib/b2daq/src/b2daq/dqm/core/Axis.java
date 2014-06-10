package b2daq.dqm.core;

public class Axis {

    private String name = "";
    private int nbins = 0;
    private double min = 0;
    private double max = 1;
    private boolean fixedMin = false;
    private boolean fixedMax = false;
    private String title = null;

    public Axis(int nbin, double min, double max, String label) {
        nbins = nbin;
        this.min = min;
        this.max = max;
        title = label;
    }

    public Axis() {
        this(1, 0, 1, "");
    }

    public Axis(int nbin, double min, double max) {
        this(nbin, min, max, "");
    }

    public Axis(Axis axis) {
        this(axis.nbins, axis.min, axis.max, axis.title);
    }

    public void copy(Axis axis) {
        nbins = axis.nbins;
        min = axis.min;
        max = axis.max;
        title = axis.title;
        fixedMin = axis.fixedMin;
        fixedMax = axis.fixedMax;
    }

    public void copyRange(Axis axis) {
        if (!fixedMin) {
            min = axis.min;
        }
        if (!fixedMax) {
            max = axis.max;
        }
    }

    public int getNbins() {
        return nbins;
    }

    public double getMax() {
        return max;
    }

    public double getMin() {
        return min;
    }

    public String getTitle() {
        return title;
    }

    public boolean isFixedMin() {
        return fixedMin;
    }

    public boolean isFixedMax() {
        return fixedMax;
    }

    public void setRange(int nbin, double min, double max) {
        nbins = nbin;
        this.min = min;
        this.max = max;
    }

    public void setNbins(int nbins) {
        this.nbins = nbins;
    }

    public void setMax(double max) {
        //if (!fixedMax) {
            this.max = max;
        //}
    }

    public void setMin(double min) {
        //if (!fixedMin) {
            this.min = min;
        //}
    }

    public void setTitle(String label) {
        title = label;
    }

    public void setFixedMin(boolean fix) {
        fixedMin = fix;
    }

    public void setFixedMax(boolean fix) {
        fixedMax = fix;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }

    public String toString() {
        String script = "{"
                + "'label':'" + getTitle() + "',"
                + "'nbins':" + getNbins() + ","
                + "'min':" + getMin() + ","
                + "'max':" + getMax() + "}";
        return script;
    }

    public String getXML(String name) {
        String script = "<" + name + " "
                + "label=\"" + getTitle() + "\" "
                + "nbins=\"" + getNbins() + "\" "
                + "min=\"" + getMin() + "\" "
                + "max=\"" + getMax() + "\" ";
        if (isFixedMin()) {
            script += "fixed-min=\"" + isFixedMin() + "\" ";
        }
        if (isFixedMax()) {
            script += "fixed-max=\"" + isFixedMax() + "\" ";
        }
        script += "/>\n";
        return script;
    }

}
