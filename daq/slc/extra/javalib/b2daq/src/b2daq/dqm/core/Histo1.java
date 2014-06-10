package b2daq.dqm.core;

import java.io.IOException;

import b2daq.core.Reader;
import b2daq.core.Writer;
import b2daq.dqm.graphics.GHisto1;
import javafx.scene.paint.Color;

public abstract class Histo1 extends Histo {

    public Histo1() {
        super();
        setLineColor(Color.RED);
        setFontSize(0.5);
        drawer = new GHisto1(null, this);
    }

    public Histo1(String name, String title, int nbinx, double xmin, double xmax) {
        super(name, title);
        setLineColor(Color.RED);
        setFontSize(0.5);
        axisX.setRange(nbinx, xmin, xmax);
        drawer = new GHisto1(null, this);
    }

    @Override
    public void fixMaximum(double data, boolean fix) {
        axisY.setMax(data);
        axisY.setFixedMax(fix);
    }

    @Override
    public void fixMinimum(double data, boolean fix) {
        axisY.setMin(data);
        axisY.setFixedMin(fix);
    }

    /*
     public void reset() {
     super.reset();
     setMaximum(1);
     }
     */
    @Override
    public int getDim() {
        return 1;
    }

    @Override
    public double getMaximum() {
        return axisY.getMax();
    }

    @Override
    public double getMinimum() {
        return axisY.getMin();
    }

    @Override
    public void setMaximum(double data) {
        if (!axisY.isFixedMax()) {
            axisY.setMax(data);
        }
    }

    @Override
    public void setMinimum(double data) {
        if (!axisY.isFixedMin()) {
            axisY.setMin(data);
        }
    }

    @Override
    public double getBinContent(int nx) {
        return data.get(nx + 1);
    }

    @Override
    public double getOverFlow() {
        return data.get(getAxisX().getNbins() + 1);
    }

    @Override
    public double getUnderFlow() {
        return data.get(0);
    }

    @Override
    public void setBinContent(int nx, double data) {
        if (nx >= 0 && nx < axisX.getNbins()) {
            this.data.set(nx + 1, data);
            if (data * 1.1 > getMaximum()) {
                setMaximum(data * 1.1);
            }
        }
    }

    @Override
    public void setOverFlow(double data) {
        this.data.set(getAxisX().getNbins() + 1, data);
    }

    @Override
    public void setUnderFlow(double data) {
        this.data.set(0, data);
    }

    @Override
    public void setMaxAndMin() {
        if (!axisY.isFixedMax() || !axisY.isFixedMin()) {
            double data, data_max = getBinContent(0);
            for (int nx = 0; nx < axisX.getNbins(); nx++) {
                data = getBinContent(nx);
                if (data > data_max) {
                    data_max = data;
                }
            }
            setMaximum(data_max * 1.1);
        }
    }

    @Override
    public void readConfig(Reader reader) throws IOException {
        setTabId(reader.readChar());
        setPositionId(reader.readChar());
        setName(reader.readString());
        setTitle(reader.readString());
        setAxisX(readAxis(reader));
        getAxisY().setTitle(reader.readString());
        data.resize(getAxisX().getNbins() + 2);
    }

    @Override
    public void writeConfig(Writer writer) throws IOException {
        writer.writeString(getDataType());
        writer.writeChar((char) getTabId());
        writer.writeChar((char) getPositionId());
        writer.writeString(getName());
        writer.writeString(getTitle());
        writeAxis(writer, getAxisX());
        writer.writeString(getAxisY().getTitle());
    }

    @Override
    public Histo clone() {
        try {
            Histo1 h = (Histo1) HistoFactory.create(getDataType());
            h.setName(getName());
            h.setTitle(getTitle());
            h.getAxisX().copy(getAxisX());
            h.getAxisY().copy(getAxisY());
            h.getData().copy(getData());
            return h;
        } catch (WrongDataTypeException e) {
            return null;
        }
    }

    @Override
    public void add(Histo h, double scale) {
        if (h.getAxisX().getNbins() == getAxisX().getNbins()
                && h.getAxisX().getMin() == getAxisX().getMin()
                && h.getAxisX().getMax() == getAxisX().getMax()) {
            for (int nx = 0; nx < getAxisX().getNbins(); nx++) {
                setBinContent(nx, getBinContent(nx) + h.getBinContent(nx) * scale);
            }
        }
        setMaxAndMin();
    }
    
    public void setNbinsx(int nbinsx) {
        axisX.setNbins(nbinsx);
        data.resize(nbinsx + 2);
    }
}
