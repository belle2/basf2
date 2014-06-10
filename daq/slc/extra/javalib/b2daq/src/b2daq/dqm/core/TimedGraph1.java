package b2daq.dqm.core;

import b2daq.core.Reader;
import b2daq.core.Writer;
import b2daq.dqm.graphics.GTimedGraph1;
import java.io.IOException;
import java.util.Date;
import javafx.scene.paint.Color;

public abstract class TimedGraph1 extends Histo {

    protected int iter;
    protected LongArray times;

    public TimedGraph1() {
        this("", "", 1, 0, 1);
        setLineColor(Color.RED);
        setFontSize(0.5);
        drawer = new GTimedGraph1(null, this);
    }

    public TimedGraph1(String name, String title, int nbinx, double xmin, double xmax) {
        super(name, title);
        setLineColor(Color.RED);
        setFontSize(0.5);
        axisX.setRange(nbinx, xmin, xmax);
        times = new LongArray(nbinx);
        drawer = new GTimedGraph1(null, this);
    }

    @Override
    public void reset() {
        iter = 0;
        data.setAll(0);
        times.setAll(0);
        getAxisY().setMax(1);
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
    public double getPoint(int n) {
        return data.get(n);
    }

    @Override
    public void setPoint(int n, double data) {
        this.data.set(n, data);
    }

    public long getTime(int n) {
        return (long) times.get(n);
    }

    public void setTime(int n, long time) {
        times.set(n, time / 1000);
    }

    public void addPoint(long time, double data) {
        iter++;
        if (iter == this.data.length()
                || iter == times.length()) {
            iter = 0;
        }
        setPoint(iter, data);
        setTime(iter, time);
        setUpdateTime(time);
    }

    @Override
    public void addPoint(double data) {
        addPoint(new Date().getTime(), data);
    }

    @Override
    public double getLatestPoint() {
        return data.get(iter);
    }

    public long getLatestTime() {
        return (long) times.get(iter);
    }

    @Override
    public void setMaxAndMin() {
        if (!axisY.isFixedMax() || !axisY.isFixedMin()) {
            double data_in, min = getMinimum(), max = getMaximum();
            for (int nx = 0; nx < getAxisX().getNbins(); nx++) {
                data_in = getPoint(nx);
                if (data_in > max) {
                    max = data_in;
                }
                if (data_in < min) {
                    min = data_in;
                }
            }
            setMaximum((max + min) / 2. + (max - min) / 2. * 1.05);
            setMinimum((max + min) / 2. - (max - min) / 2. * 1.05);
        }
    }

    @Override
    public String toString() {
        StringBuilder script = new StringBuilder();
        script.append("      {\n" + "        'datatype':'" + getDataType()
                + "',\n" + "        'name':'" + getName() + "',\n"
                + "        'title':'" + getTitle() + "',\n"
                + "        'tabid':" + (int) getTabId() + ",\n"
                + "        'positionid':" + (int) getPositionId() + ",\n"
                + "        'axisx':" + getAxisX().toString() + ",\n"
                + "        'axisy':" + getAxisY().toString() + ",\n"
                + "        'axisz':" + getAxisZ().toString() + ",\n"
                + "        'iter':" + iter + "\n"
                + "        'time':" + times.toString() + "\n"
                + "        'data':" + data.toString() + "\n" + "      }");
        return script.toString();
    }

    @Override
    protected String getXML() {
        String str = "title=\"" + getTitle()
                + "\" data=\"" + data.toString()
                + "\" iter=\"" + iter
                + "\" >\n";
        str += axisX.getXML("axis-x");
        str += axisY.getXML("axis-y");
        return str;
    }

    @Override
    public void readObject(Reader reader) throws IOException {
        readConfig(reader);
        iter = reader.readInt() - 1;
        getData().readObject(reader);
        times.readObject(reader);
    }

    @Override
    public void readContents(Reader reader) throws IOException {
        iter = reader.readInt();
        times.readObject(reader);
        getData().readObject(reader);
    }

    @Override
    public void readConfig(Reader reader) throws IOException {
        setTabId(reader.readChar());
        setPositionId(reader.readChar());
        setName(reader.readString());
        setTitle(reader.readString());
        setAxisX(readAxis(reader));
        getAxisY().setTitle(reader.readString());
        data.resize(axisX.getNbins());
        times.resize(axisX.getNbins());
    }

    @Override
    public void writeObject(Writer writer) throws IOException {
        writeConfig(writer);
        writer.writeInt(iter);
        data.writeObject(writer);
        times.writeObject(writer);
    }

    @Override
    public void writeContents(Writer writer) throws IOException {
        writer.writeInt(iter);
        times.writeObject(writer);
        data.writeObject(writer);
    }

    @Override
    public void writeConfig(Writer writer) throws IOException {
        super.writeConfig(writer);
        writeAxis(writer, getAxisX());
        writer.writeString(getAxisY().getTitle());
    }

    public int getIter() {
        return iter;
    }

    @Override
    public void setNbinsx(int nbinsx) {
        axisX.setNbins(nbinsx);
        times.resize(nbinsx);
        data.resize(nbinsx);
    }
}
