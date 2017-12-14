package b2daq.dqm.core;

import b2daq.core.Reader;
import b2daq.core.Writer;
import b2daq.dqm.graphics.GHisto2;
import b2daq.graphics.GraphicsDrawer;
import java.io.IOException;

public abstract class Histo2 extends Histo {

    private boolean _stat = true;

    public Histo2() {
        super();
        drawer = new GHisto2(null, this);
    }

    public Histo2(String name, String title, int nbinx, double xmin, double xmax, int nbiny, double ymin, double ymax) {
        super(name, title);
        axisX.setRange(nbinx, xmin, xmax);
        axisY.setRange(nbiny, ymin, ymax);
        drawer = new GHisto2(null, this);
    }
    /*
     public void reset() {
     super.reset();
     setMaximum(1);
     }
     */

    public boolean getStat() {
        return _stat;
    }

    public void setStat(boolean stat) {
        _stat = stat;
    }

    @Override
    public void fixMaximum(double data, boolean fix) {
        axisZ.setMax(data);
        axisZ.setFixedMax(fix);
    }

    @Override
    public void fixMinimum(double data, boolean fix) {
        axisZ.setMin(data);
        axisZ.setFixedMin(fix);
    }

    @Override
    public double getBinContent(int nx, int ny) {
        return data.get((nx + 1) + (ny + 1) * (axisX.getNbins() + 2));
    }

    @Override
    public double getOverFlow() {
        return data.get(data.length() - 1);
    }

    @Override
    public double getUnderFlow() {
        return data.get(0);
    }

    @Override
    public double getOverFlowX(int ny) {
        return data.get(data.length() - 1);
    }

    @Override
    public double getUnderFlowX(int ny) {
        return data.get(0);
    }

    @Override
    public double getOverFlowY(int nx) {
        return data.get(data.length() - 1);
    }

    @Override
    public double getUnderFlowY(int nx) {
        return data.get(0);
    }

    @Override
    public void setBinContent(int nx, int ny, double data) {
        if (nx >= 0 && nx < axisX.getNbins() && ny >= 0 && ny < axisY.getNbins()) {
            this.data.set((nx + 1) + (ny + 1) * (axisX.getNbins() + 2), data);
            if (data * 1.05 > getMaximum()) {
                setMaximum(data * 1.05);
            }
        }
    }

    @Override
    public void setOverFlow(double data) {
        this.data.set(this.data.length() - 1, data);
    }

    @Override
    public void setUnderFlow(double data) {
        this.data.set(0, data);
    }

    @Override
    public int getDim() {
        return 2;
    }

    @Override
    public double getMaximum() {
        return axisZ.getMax();
    }

    @Override
    public double getMinimum() {
        return axisZ.getMin();
    }

    @Override
    public void setMaximum(double data) {
        if (!axisZ.isFixedMax()) {
            axisZ.setMax(data);
        }
    }

    @Override
    public void setMinimum(double data) {
        if (!axisZ.isFixedMin()) {
            axisZ.setMin(data);
        }
    }

    @Override
    public void setMaxAndMin() {
        if (!axisY.isFixedMax() || !axisY.isFixedMin()) {
            double data;
            for (int ny = 0; ny < axisY.getNbins(); ny++) {
                for (int nx = 0; nx < axisX.getNbins(); nx++) {
                    data = getBinContent(nx, ny);
                    if (data * 1.05 > getMaximum()) {
                        setMaximum(data * 1.05);
                    }
                }
            }
        }
    }

    public double getMeanX() {
        int entries = 0;
        double bin = 0;
        double mean = 0;
        for (int nx = 0; nx < axisX.getNbins(); nx++) {
            bin = 0;
            for (int ny = 0; ny < axisY.getNbins(); ny++) {
                bin += getBinContent(nx, ny);
            }
            mean += bin * ((nx + 0.5) * (axisX.getMax() - axisX.getMin())
                    / axisX.getNbins() + axisX.getMin());
            entries += bin;
        }
        return mean / entries;
    }

    public double getMeanY() {
        int entries = 0;
        double bin = 0;
        double mean = 0;
        for (int ny = 0; ny < axisY.getNbins(); ny++) {
            bin = 0;
            for (int nx = 0; nx < axisX.getNbins(); nx++) {
                bin += getBinContent(nx, ny);
            }
            mean += bin * ((ny + 0.5) * (axisY.getMax() - axisY.getMin())
                    / axisY.getNbins() + axisY.getMin());
            entries += bin;
        }
        return mean / entries;
    }

    @Override
    public double getEntries() {
        int entries = 0;
        for (int ny = 0; ny < axisY.getNbins(); ny++) {
            for (int nx = 0; nx < axisX.getNbins(); nx++) {
                entries += getBinContent(nx, ny);
            }
        }
        return entries;
    }

    @Override
    public void readConfig(Reader reader) throws IOException {
        setTabId(reader.readChar());
        setPositionId(reader.readChar());
        setName(reader.readString());
        setTitle(reader.readString());
        setAxisX(readAxis(reader));
        setAxisY(readAxis(reader));
        data.resize((getAxisX().getNbins() + 2) * (getAxisY().getNbins() + 2));
    }

    @Override
    public void writeConfig(Writer writer) throws IOException {
        writer.writeString(getDataType());
        writer.writeChar((char) getTabId());
        writer.writeChar((char) getPositionId());
        writer.writeString(getName());
        writer.writeString(getTitle());
        writeAxis(writer, getAxisX());
        writeAxis(writer, getAxisY());
    }

    @Override
    protected String getXML() {
        String str = "title=\"" + getTitle()
                + "\" data=\"" + data.toString()
                + "\" >\n";
        str += axisX.getXML("axis-x");
        str += axisY.getXML("axis-y");
        str += axisZ.getXML("axis-z");
        return str;
    }

    @Override
    public Histo clone() {
        Histo2 h = (Histo2) HistoFactory.create(getDataType());
        h.setName(getName());
        h.setTitle(getTitle());
        h.getAxisX().copy(getAxisX());
        h.getAxisY().copy(getAxisY());
        h.getAxisZ().copy(getAxisZ());
        h.getData().copy(getData());
        return h;
    }

    @Override
    public void add(Histo h, double scale) {
        if (h.getAxisY().getNbins() == getAxisY().getNbins()
                && h.getAxisY().getMin() == getAxisY().getMin()
                && h.getAxisY().getMax() == getAxisY().getMax()
                && h.getAxisX().getNbins() == getAxisX().getNbins()
                && h.getAxisX().getMin() == getAxisX().getMin()
                && h.getAxisX().getMax() == getAxisX().getMax()) {
            for (int ny = 0; ny < getAxisY().getNbins(); ny++) {
                for (int nx = 0; nx < getAxisX().getNbins(); nx++) {
                    setBinContent(nx, ny, getBinContent(nx, ny) + h.getBinContent(nx, ny) * scale);
                }
            }
        }
    }

    @Override
    public void draw(GraphicsDrawer canvas) {
        drawer.draw(canvas);
    }

    public void setNbinsx(int nbinsx) {
        axisX.setNbins(nbinsx);
        data.resize((nbinsx + 2) * (axisY.getNbins() + 2));
    }

    public void setNbinsy(int nbinsy) {
        axisY.setNbins(nbinsy);
        data.resize((nbinsy + 2) * (axisX.getNbins() + 2));
    }

}
