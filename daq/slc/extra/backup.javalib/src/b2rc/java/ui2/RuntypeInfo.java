package b2rc.java.ui2;

import java.util.ArrayList;
import java.util.HashMap;

import b2daq.core.Reader;
import b2daq.core.SerializableObject;
import b2daq.core.Writer;

public class RuntypeInfo implements SerializableObject {

	private String _runtype;
	private HashMap<String, RunParam> _param_m = new HashMap<String, RunParam>();
	private ArrayList<String> _name_v = new ArrayList<String>();

	public RuntypeInfo() {
	}

	public RuntypeInfo(String runtype) {
		_runtype = runtype;
	}

	public String getRuntype() {
		return _runtype;
	}

	public HashMap<String, RunParam> getParams() {
		return _param_m;
	}

	public ArrayList<String> getParamNames() {
		return _name_v;
	}

	public RunParam getParam(String runtype) {
		return _param_m.get(runtype);
	}

	public void writeObject(Writer writer) throws Exception {
		writer.writeString(_runtype);
		writer.writeInt(_param_m.size());
		int count = 0;
		for (String parname : _name_v) {
			String att = _param_m.get(parname).getAttName();
			String value = _param_m.get(parname).getValue();
			writer.writeInt(count++);
			writer.writeString(parname);
			writer.writeString(att);
			writer.writeString(value);
		}
		writer.writeInt(-1);
	}

	public void readObject(Reader reader) throws Exception {
		_runtype = reader.readString();
		int npars = reader.readInt();
		for (int n = 0; n < npars; n++) {
			int count = reader.readInt();
			if (count != n) {
				throw (new Exception("stream order error"));
			}
			String parname = reader.readString();
			String att = reader.readString();
			String value = reader.readString();
			if (hasParam(parname)) {
				setValue(parname, value);
			} else {
				addParam(parname, att, value);
			}
		}
	}
	
	public String getAttName(String name) {
		return _param_m.get(name).getAttName();
	}

	public String getValue(String name) {
		return _param_m.get(name).getValue();
	}

	public int getValueInt(String name) {
		return _param_m.get(name).getValueInt();
	}

	public double getValueFloat(String name) {
		return _param_m.get(name).getValueFloat();
	}

	boolean hasParam(String name) {
		return _param_m.containsKey(name);
	}

	void addParam(String name, String att, String value) {
		RunParam param = new RunParam(att, value);
		_param_m.put(name, param);
		_name_v.add(name);
	}

	void setValue(String name, String value) {
		  if (_param_m.containsKey(name)) {
		    _param_m.get(name).setValue(value);
		  }
		}

	public void setRuntype(String runtype) {
		_runtype = runtype;
	}


}
