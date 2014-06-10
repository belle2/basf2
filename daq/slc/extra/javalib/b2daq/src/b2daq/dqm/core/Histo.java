package b2daq.dqm.core;

import java.io.IOException;

import b2daq.core.Reader;
import b2daq.core.Writer;

public abstract class Histo extends AbstractHisto {

    protected String draw = "AL";

    public String parseUnicode(String code) {
        String str = code.replace("&lt;", "<");
        str = str.replace("&gt;", ">");
        str = str.replace("&quot;", "\"");
        str = str.replace("&apos;", "'");
        str = str.replace("&amp;", "&");
        return str;
    }

    public Histo() {
        super();
    }

    public Histo(String name, String title) {
        super(name, title);
    }

    public void setDraw(String opt) {
        draw = opt;
    }

    public String getDraw() {
        return draw;
    }

    @Override
    public void fill(double vx) {
        int Nx = getAxisX().getNbins() + 1;
        double dbinx = (axisX.getMax() - axisX.getMin()) / axisX.getNbins();

        if (vx < axisX.getMin()) {
            Nx = 0;
        } else if (vx <= axisX.getMax()) {
            for (int nx = 0; nx < axisX.getNbins(); nx++) {
                if (vx >= dbinx * nx + axisX.getMin()
                        && vx <= dbinx * (nx + 1) + axisX.getMin()) {
                    Nx = nx + 1;
                    break;
                }
            }
        }
        data.set(Nx, data.get(Nx) + 1);
    }

    @Override
    public void fill(double vx, double vy) {
        int Nx = getAxisX().getNbins() + 1;
        int Ny = getAxisY().getNbins() + 1;
        double dbinx = (axisX.getMax() - axisX.getMin()) / axisX.getNbins();
        double dbiny = (axisY.getMax() - axisY.getMin()) / axisY.getNbins();

        if (vx < axisX.getMin()) {
            Nx = 0;
        } else if (vx <= axisX.getMax()) {
            for (int nx = 0; nx < axisX.getNbins(); nx++) {
                if (vx >= dbinx * nx + axisX.getMin()
                        && vx <= dbinx * (nx + 1) + axisX.getMin()) {
                    Nx = nx + 1;
                    break;
                }
            }
        }
        if (vy < axisY.getMin()) {
            Ny = 0;
        } else if (vy <= axisY.getMax()) {
            for (int ny = 0; ny < axisY.getNbins(); ny++) {
                if (vy >= dbiny * ny + axisY.getMin()
                        && vy <= dbiny * (ny + 1) + axisY.getMin()) {
                    Ny = ny + 1;
                    break;
                }
            }
        }
        int N = Nx + (axisY.getNbins() + 2) * Ny;
        data.set(N, data.get(N) + 1);
    }

    @Override
    public double getMean() {
        int entries = 0;
        double bin = 0;
        double mean = 0;
        for (int n = 0; n < axisX.getNbins(); n++) {
            bin = getBinContent(n);
            mean += bin * ((n + 0.5) * (axisX.getMax() - axisX.getMin())
                    / axisX.getNbins() + axisX.getMin());
            entries += bin;
        }
        return mean / entries;
    }

    public double getEntries() {
        int entries = 0;
        for (int n = 0; n < axisX.getNbins(); n++) {
            entries += getBinContent(n);
        }
        return entries;
    }

    @Override
    public double getRMS() {
        return -1;
    }

//    @Override
//    public String toString() {
//        StringBuffer script = new StringBuffer();
//        script.append("      {\n" + "        'datatype':'" + getDataType()
//                + "',\n" + "        'name':'" + getName() + "',\n"
//                + "        'title':'" + getTitle() + "',\n"
//                + "        'tabid':" + (int) getTabId() + ",\n"
//                + "        'positionid':" + (int) getPositionId() + ",\n"
//                + "        'axisx':" + getAxisX().toString() + ",\n"
//                + "        'axisy':" + getAxisY().toString() + ",\n"
//                + "        'axisz':" + getAxisZ().toString() + ",\n"
//                + "        'bins':" + data.toString() + "\n"
//                + "      }");
//        return script.toString();
//    }

    public void setData(NumberArray array) {
        data = array;
    }

    public NumberArray getData() {
        return data;
    }

    @Override
    public void reset() {
        data.setAll(0);
        setMaximum(0);
    }

    @Override
    public void readContents(Reader reader) throws IOException {
        data.readObject(reader);
        setMaxAndMin();
    }

    protected Axis readAxis(Reader reader) throws IOException {
        Axis axis = new Axis();
        axis.setTitle(parseUnicode(reader.readString()));
        axis.setNbins(reader.readInt());
        axis.setMin(reader.readDouble());
        axis.setMax(reader.readDouble());
        return axis;
    }

    @Override
    public void writeContents(Writer writer) throws IOException {
        data.writeObject(writer);
    }

    protected void writeAxis(Writer writer, Axis axis) throws IOException {
        writer.writeString(axis.getTitle());
        writer.writeInt(axis.getNbins());
        writer.writeDouble(axis.getMin());
        writer.writeDouble(axis.getMax());
    }

    @Override
    protected String getXML() {
        String str = "title=\"" + getTitle()
                + "\" data=\"" + data.toString()
                + "\" >\n";
        str += axisX.getXML("axis-x");
        str += axisY.getXML("axis-y");
        return str;
    }

    @Override
    public Histo clone() {
        return null;
    }

    public void add(Histo h) {
        add(h, 1);
    }

    public void add(Histo h, double scale) {

    }

}
