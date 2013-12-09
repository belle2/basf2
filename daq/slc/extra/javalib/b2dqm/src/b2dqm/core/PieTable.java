package b2dqm.core;

import b2daq.core.Reader;
import b2daq.core.Writer;

public class PieTable extends MonShape {

	private String _title;
	private String [] _label_v;
	private DoubleArray _data_v;
	
	public PieTable() {
		this("", 0);
	}
	
	public PieTable(String name, int nindex) {
		_name = name;
		_label_v = new String[nindex];
		_data_v = new DoubleArray(nindex);
	}
	
	public int getNIndex() {
		return _data_v.length();
	}
	
	public void setData(int index, double data) {
		_data_v.set(index, data);
	}

	public double getData(int index) {
		return _data_v.get(index);
	}

	public void setLabel(int index, String label) {
		_label_v[index] = label;
	}

	public String getLabel(int index) {
		return _label_v[index];
	}

	@Override
	public String getDataType() {
		return "PIE";
	}

	public void writeConfig(Writer writer) throws Exception {
		super.writeConfig(writer);
		writer.writeString(_title);
		writer.writeInt(getNIndex());
		for (int n = 0; n < getNIndex(); n++ ) {
			writer.writeString(_label_v[n]);
		}
	}

	public void writeUpdate(Writer writer) throws Exception {
		_data_v.writeObject(writer);
	}

	public void writeContents(Writer writer) throws Exception {
		writeUpdate(writer);
	}
	
	public void readConfig(Reader reader) throws Exception {
		super.readConfig(reader);
	}

	public void readUpdate(Reader reader) throws Exception {
		_title = reader.readString();
		int nindex = reader.readInt();
		_label_v = new String[nindex];
		_data_v = new DoubleArray(nindex);
		for (int n = 0; n < getNIndex(); n++ ) {
			_label_v[n] = reader.readString();
		}
		
	}

	public void readContents(Reader reader) throws Exception {
		readUpdate(reader);
	}
	protected String getXML() {
		String str = "title=\"" + _title + 
		"\" >\n";
		return str;
	}

}
