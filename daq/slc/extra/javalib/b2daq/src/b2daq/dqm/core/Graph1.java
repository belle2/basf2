package b2daq.dqm.core;

import b2daq.core.Reader;
import b2daq.core.Writer;
import b2daq.dqm.graphics.GGraph1;
import java.io.IOException;
import javafx.scene.paint.Color;

public abstract class Graph1 extends Histo {

    public Graph1() {
        super();
        setLineColor(Color.RED);
        setFontSize(0.5);
        drawer = new GGraph1(null, this);
    }

    public Graph1(String name, String title, int nbinx, double xmin, double xmax,
            double ymin, double ymax) {
        super(name, title);
        setLineColor(Color.RED);
        setFontSize(0.5);
        axisX.setRange(nbinx, xmin, xmax);
        axisY.setRange(1, ymin, ymax);
        drawer = new GGraph1(null, this);
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
    public double getPointX(int n) {
        return data.get(n);
    }

    @Override
    public double getPointY(int n) {
        return data.get(data.length() / 2 + n);
    }

    @Override
    public void setPointX(int n, double data) {
        this.data.set(n, data);
    }

    @Override
    public void setPointY(int n, double data) {
        this.data.set(this.data.length() / 2 + n, data);
    }

    @Override
    public void setMaxAndMin() {
        if (!axisY.isFixedMax() || !axisY.isFixedMin()) {
            double data, min = getMinimum(), max = getMaximum();
            for (int nx = 0; nx < axisX.getNbins(); nx++) {
                data = getPointY(nx);
                if (data * 1.05 > max) {
                    max = data * 1.05;
                }
                if (data * 1.05 < min) {
                    min = data * 1.05;
                }
            }
            if (getMaximum() < max && getMinimum() > min) {
                setMaximum((max + min) / 2. + (max - min) / 2. * 1.05);
                setMinimum((max + min) / 2. - (max - min) / 2. * 1.05);
            }
        }
    }

    @Override
    public void readConfig(Reader reader) throws IOException {
        setTabId(reader.readChar());
        setPositionId(reader.readChar());
        setName(reader.readString());
        setTitle(reader.readString());
        setAxisX(readAxis(reader));
        setAxisY(readAxis(reader));
        getData().resize(getAxisX().getNbins() * 2);
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
    public Histo clone() {
        Histo h = (Histo) HistoFactory.create(getDataType());
        h.setName(getName());
        h.setTitle(getTitle());
        h.getAxisX().copy(getAxisX());
        h.getAxisY().copy(getAxisY());
        h.getData().copy(getData());
        return h;
    }

    public void setNbinsx(int nbinsx) {
        axisX.setNbins(nbinsx);
        data.resize(nbinsx * 2);
    }

}
