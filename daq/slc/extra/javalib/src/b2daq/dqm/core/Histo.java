package b2daq.dqm.core;

import java.io.IOException;

import b2daq.core.Reader;
import b2daq.core.Writer;

public abstract class Histo extends AbstractHisto {

	public String parseUnicode(String code) {
		String str = code.replace("&lt;", "<");
		str = str.replace("&gt;",">");
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

	public void fill(double vx) {
		int Nx = getAxisX().getNbins() + 1;
		double dbinx = (_axis_x.getMax() - _axis_x.getMin()) / _axis_x.getNbins();

		if (vx < _axis_x.getMin()) {
			Nx = 0;
		} else if (vx <= _axis_x.getMax()) {
			for (int nx = 0; nx < _axis_x.getNbins(); nx++) {
				if ( vx >= dbinx * nx + _axis_x.getMin() &&
					 vx <= dbinx * (nx + 1) + _axis_x.getMin()) {
					Nx = nx + 1;
					break;
				}
			}
		}
		_data_v.set(Nx, _data_v.get(Nx) + 1);
	}

	public void fill(double vx, double vy) {
		int Nx = getAxisX().getNbins() + 1;
		int Ny = getAxisY().getNbins() + 1;
		double dbinx = (_axis_x.getMax() - _axis_x.getMin()) / _axis_x.getNbins();
		double dbiny = (_axis_y.getMax() - _axis_y.getMin()) / _axis_y.getNbins();

		if (vx < _axis_x.getMin()) {
			Nx = 0;
		} else if (vx <= _axis_x.getMax()) {
			for (int nx = 0; nx < _axis_x.getNbins(); nx++) {
				if (vx >= dbinx * nx + _axis_x.getMin()
						&& vx <= dbinx * (nx + 1) + _axis_x.getMin()) {
					Nx = nx + 1;
					break;
				}
			}
		}
		if (vy < _axis_y.getMin()) {
			Ny = 0;
		} else if (vy <= _axis_y.getMax()) {
			for (int ny = 0; ny < _axis_y.getNbins(); ny++) {
				if (vy >= dbiny * ny + _axis_y.getMin()
						&& vy <= dbiny * (ny + 1) + _axis_y.getMin()) {
					Ny = ny + 1;
					break;
				}
			}
		}
		int N = Nx + (_axis_y.getNbins() + 2) * Ny;
		_data_v.set(N, _data_v.get(N) + 1);
	}

	public double getMean() {
		int entries = 0;
		double bin = 0;
		double mean = 0;
		for (int n = 0; n < _axis_x.getNbins(); n++) {
			bin = getBinContent(n);
			mean += bin * ((n + 0.5) * (_axis_x.getMax() - _axis_x.getMin())
					/ _axis_x.getNbins() + _axis_x.getMin());
			entries += bin;
		}
		return mean / entries;
	}

	public double getEntries() {
		int entries = 0;
		for (int n = 0; n < _axis_x.getNbins(); n++) {
			entries += getBinContent(n);
		}
		return entries;
	}

	public double getRMS() {
		return -1;
	}

	public String toString() {
		StringBuffer script = new StringBuffer();
		script.append("      {\n" + "        'datatype':'" + getDataType()
				+ "',\n" + "        'name':'" + getName() + "',\n"
				+ "        'title':'" + getTitle() + "',\n"
				+ "        'tabid':" + (int) getTabId() + ",\n"
				+ "        'positionid':" + (int) getPositionId() + ",\n"
				+ "        'axisx':" + getAxisX().toString() + ",\n"
				+ "        'axisy':" + getAxisY().toString() + ",\n"
				+ "        'axisz':" + getAxisZ().toString() + ",\n"
				+ "        'bins':" + _data_v.toString() + "\n"
				+ "      }");
		return script.toString();
	}

	public void setData(NumberArray array) {
		_data_v = array;
	}

	public NumberArray getData() {
		return _data_v;
	}

	public void reset() {
		_data_v.setAll(0);
		setMaximum(0);
	}

	@Override
	public void readContents(Reader reader) throws IOException {
		_data_v.readObject(reader);
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
		_data_v.writeObject(writer);
	}
	
	protected void writeAxis(Writer writer, Axis axis) throws IOException {
		writer.writeString(axis.getTitle());
		writer.writeInt(axis.getNbins());
		writer.writeDouble(axis.getMin());
		writer.writeDouble(axis.getMax());
	}

	protected String getXML() {
		String str = "title=\"" + getTitle() + 
		"\" data=\"" + _data_v.toString() + 
		"\" >\n";
		str += _axis_x.getXML("axis-x");
		str += _axis_y.getXML("axis-y");
		return str;
	}

	public Histo clone() {
		return null;
	}

	public void add(Histo h) {
		add(h, 1);
	}
	
	public void add(Histo h, double scale) {
		
	}
	
}
