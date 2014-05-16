package b2daq.core;

import java.util.ArrayList;
import java.util.HashMap;

import b2daq.core.Reader;
import b2daq.core.SerializableObject;
import b2daq.core.Writer;

public class DataObject implements SerializableObject {

	final static public int BOOL = 0;
	final static public int SHORT = 2;
	final static public int INT = 3;
	final static public int LONG = 4;
	final static public int FLOAT = 9;
	final static public int DOUBLE = 10;
	final static public int TEXT = 11;

	public class ParamInfo {
		public int type;
		public int length;
		public ArrayList<Object> buf = new ArrayList<Object>();
	};

	private int _revision;
	private int _confno;
	private int _id;
	private String _class = "";
	private String _base_class = "";
	private HashMap<String, ParamInfo> _param_m = new HashMap<String, ParamInfo>();
	private HashMap<String, HashMap<String, Integer>> _enum_m_m = new HashMap<String, HashMap<String, Integer>>();
	private ArrayList<String> _name_v = new ArrayList<String>();

	public DataObject() {
		_revision = 0;
		_confno = 0;
		_id = 0;
	}

	public DataObject(DataObject obj) {
		_revision = obj._revision;
		_confno = obj._confno;
		_id = obj._id;
		_class = obj._class;
		_base_class = obj._base_class;
		for (String name : obj._name_v) {
			ParamInfo info = obj._param_m.get(name);
			add(name, info.buf, info.type, info.length);
		}
	}

	public DataObject(String data_class, String base_class) {
		_revision = 0;
		_confno = 0;
		_id = 0;
		_class = data_class;
		_base_class = base_class;
	}

	public void print() {
		System.out.println("-------------------");
		System.out.println(_class + " rev = " + _revision);
		System.out.println("config_no = " + _confno);
		for (String name : _name_v) {
			ParamInfo info = _param_m.get(name);
			int type = info.type;
			if (type == TEXT) {
				System.out.println(name + " : '" + getText(name) + "'");
			} else {
				ArrayList<Object> buf = info.buf;
				int length = info.length;
				if (length == 0)
					length = 1;
				for (int i = 0; i < length; i++) {
					System.out.print(name
							+ ((info.length > 0) ? ("[" + i + "] : ") : " : "));
					switch (type) {
					case BOOL:
						System.out.print((Boolean) buf.get(i) ? "true"
								: "false");
						break;
					case LONG:
						System.out.print((Long) buf.get(i));
						break;
					case INT:
						System.out.print((Integer) buf.get(i));
						break;
					case SHORT:
						System.out.print((Short) buf.get(i));
						break;
					case FLOAT:
						System.out.print((Float) buf.get(i));
						break;
					case DOUBLE:
						System.out.print((Double) buf.get(i));
						break;
					default:
						break;
					}
					System.out.println();
				}
			}
		}
		System.out.println("-------------------");
	}

	public void setValues(ArrayList<String> name_v, ArrayList<String> value_v) {
		for (int i = 0; i < name_v.size(); i++) {
			setValue(name_v.get(i), value_v.get(i));
		}
	}

	public void setValue(String name_in, String value) {
		String[] str_v = name_in.split(":");
		String name = str_v[0];
		if (_param_m.containsKey(name)) {
			ParamInfo info = _param_m.get(name);
			int type = info.type;
			ArrayList<Object> buf = info.buf;
			if (type == TEXT) {
				setText(name, value);
			} else {
				int i = (str_v.length > 1) ? Integer.parseInt(str_v[1]) : 0;
				switch (type) {
				case BOOL:
					buf.set(i,
							(value == "true" || value == "t" || value == "1"));
					break;
				case LONG:
					buf.set(i, Long.parseLong(value));
					break;
				case INT:
					buf.set(i, Integer.parseInt(value));
					break;
				case SHORT:
					buf.set(i, (Short) Short.parseShort(value));
					break;
				case DOUBLE:
					buf.set(i, Integer.parseInt(value));
					break;
				case FLOAT:
					buf.set(i, Integer.parseInt(value));
					break;
				default:
					break;
				}
			}
		}
	}

	public void readObject(Reader reader) throws Exception {
		_revision = reader.readInt();
		_confno = reader.readInt();
		_id = reader.readInt();
		_class = reader.readString();
		while (true) {
			String name = reader.readString();
			// if (name == "") continue;
			if (name.matches("==OBJECT_END=="))
				break;
			ParamInfo info = new ParamInfo();
			info.type = (Integer) reader.readInt();
			info.length = reader.readInt();
			if (!hasValue(name)) {
				info.buf = null;
			} else {
				info = _param_m.get(name);
			}
			if (info.type == TEXT) {
				String str = reader.readString();
				if (info.buf == null)
					addText(name, str, info.length);
				else
					setText(name, str);
			} else {
				if (info.buf == null) {
					add(name, null, info.type, info.length);
				}
				info = _param_m.get(name);
				ArrayList<Object> buf = info.buf;
				int length = info.length;
				if (length == 0)
					length = 1;
				for (int i = 0; i < length; i++) {
					switch (info.type) {
					case BOOL:
						buf.add(i, reader.readBoolean());
						break;
					case LONG:
						buf.add(i, reader.readLong());
						break;
					case INT:
						buf.add(i, reader.readInt());
						break;
					case SHORT:
						buf.add(i, reader.readShort());
						break;
					case FLOAT:
						buf.add(i, reader.readFloat());
						break;
					case DOUBLE:
						buf.add(i, reader.readDouble());
						break;
					default:
						break;
					}
				}
			}
		}
	}

	public void writeObject(Writer writer) throws Exception {
		writer.writeInt(_revision);
		writer.writeInt(_confno);
		writer.writeInt(_id);
		// writer.writeString(_name);
		writer.writeString(_class);
		for (String name : _name_v) {
			ParamInfo info = _param_m.get(name);
			writer.writeString(name);
			writer.writeInt(info.type);
			writer.writeInt(info.length);
			if (info.type == TEXT) {
				writer.writeString(getText(name));
			} else {
				ArrayList<Object> buf = info.buf;
				int length = info.length;
				if (length == 0)
					length = 1;
				for (int i = 0; i < length; i++) {
					switch (info.type) {
					case BOOL:
						writer.writeBoolean((Boolean) buf.get(i));
						break;
					case LONG:
						writer.writeLong((Long) buf.get(i));
						break;
					case INT:
						writer.writeInt((Integer) buf.get(i));
						break;
					case SHORT:
						writer.writeShort((Short) buf.get(i));
						break;
					case FLOAT:
						writer.writeFloat((Float) buf.get(i));
						break;
					case DOUBLE:
						writer.writeDouble((Double) buf.get(i));
						break;
					default:
						break;
					}
				}
			}
		}
		writer.writeString("==OBJECT_END==");
	}

	public int getRevision() {
		return _revision;
	}

	public void setRevision(int revision) {
		_revision = revision;
	}

	public int getConfigNumber() {
		return _confno;
	}

	public void setConfigNumber(int confno) {
		_confno = confno;
	}

	public int getId() {
		return _id;
	}

	public void setId(int id) {
		_id = id;
	}

	public String getClassName() {
		return _class;
	}

	public String getBaseClassName() {
		return _base_class;
	}

	public HashMap<String, ParamInfo> getParams() {
		return _param_m;
	}

	public ArrayList<String> getParamNames() {
		return _name_v;
	}

	public Object get(String name) {
		return _param_m.get(name).buf.get(0);
	}

	public boolean getBool(String name) {
		return (Boolean) get(name);
	}

	public char getChar(String name) {
		return (Character) get(name);
	}

	public short getShort(String name) {
		return (Short) get(name);
	}

	public int getInt(String name) {
		return (Integer) get(name);
	}

	public int getEnum(String name) {
		return (Integer) get(name);
	}

	// public String getEnumText(String name);
	public long getLong(String name) {
		return (Long) get(name);
	}

	public char getUChar(String name) {
		return (Character) get(name);
	}

	public short getUShort(String name) {
		return (Short) get(name);
	}

	public int getUInt(String name) {
		return (Integer) get(name);
	}

	public long getULong(String name) {
		return (Long) get(name);
	}

	public float getFloat(String name) {
		return (Float) get(name);
	}

	public double getDouble(String name) {
		return (Double) get(name);
	}

	public String getText(String name) {
		if ( _param_m.get(name) != null)
			return (String) get(name);
		else 
			return null;
	}

	public DataObject getObject(String name) {
		if ( _param_m.get(name) != null)
			return (DataObject) _param_m.get(name).buf.get(0);
		else 
			return null;
	}

	public ArrayList<Object> getArray(String name) {
		return _param_m.get(name).buf;
	}

	public void addBool(String name, boolean value) {
		add(name, value, BOOL, 0);
	}

	public void addLong(String name, long value) {
		add(name, value, LONG, 0);
	}

	public void addInt(String name, int value) {
		add(name, value, INT, 0);
	}

	public void addShort(String name, int value) {
		add(name, value, SHORT, 0);
	}

	public void addDouble(String name, double value) {
		add(name, value, DOUBLE, 0);
	}

	public void addFloat(String name, double value) {
		add(name, value, FLOAT, 0);
	}

	public void addBoolArray(String name, boolean value, int length) {
		add(name, value, BOOL, length);
	}

	public void addIntArray(String name, int value, int length) {
		add(name, value, INT, length);
	}

	public void addShortArray(String name, int value, int length) {
		add(name, value, SHORT, length);
	}

	public void addDoubleArray(String name, double value, int length) {
		add(name, value, DOUBLE, length);
	}

	public void addFloatArray(String name, double value, int length) {
		add(name, value, FLOAT, length);
	}

	public HashMap<String, Integer> getEnumList(String name) {
		return _enum_m_m.get(name);
	}

	protected void addText(String name, String value, int length) {
		ParamInfo info = new ParamInfo();
		info.type = TEXT;
		info.length = length;
		String str = new String();
		str = value;
		info.buf.add(str);
		_param_m.put(name, info);
		_name_v.add(name);
	}

	void add(String name, Object value, int type, int length) {
		ParamInfo info = new ParamInfo();
		info.type = type;
		info.length = length;
		if (length == 0)
			length = 1;
		for (int i = 0; i < length; i++) {
			if (value == null)
				break;
			switch (info.type) {
			case BOOL:
				info.buf.add((Boolean) value);
				break;
			case LONG:
				info.buf.add((Long) value);
				break;
			case INT:
				info.buf.add((Integer) value);
				break;
			case SHORT:
				info.buf.add((Short) value);
				break;
			case FLOAT:
				info.buf.add((Float) value);
				break;
			case DOUBLE:
				info.buf.add((Double) value);
				break;
			default:
				break;
			}
		}
		if (info.buf != null) {
			_param_m.put(name, info);
			_name_v.add(name);
		}
	}

	public void set(String name, Object value, int index) {
		_param_m.get(name).buf.set(index, value);
	}

	public void setBool(String name, boolean value) {
		set(name, value, 0);
	}

	public void setLong(String name, long value) {
		set(name, value, 0);
	}

	public void setInt(String name, int value) {
		set(name, value, 0);
	}

	public void setShort(String name, int value) {
		set(name, value, 0);
	}

	public void setChar(String name, int value) {
		set(name, value, 0);
	}

	public void setULong(String name, long value) {
		set(name, value, 0);
	}

	public void setUInt(String name, int value) {
		set(name, value, 0);
	}

	public void setUShort(String name, int value) {
		set(name, value, 0);
	}

	public void setUChar(String name, char value) {
		set(name, value, 0);
	}

	public void setDouble(String name, double value) {
		set(name, value, 0);
	}

	public void setFloat(String name, double value) {
		set(name, value, 0);
	}

	// public void setEnum(String name, String value);
	// public void setEnum(String name, String value, int id);
	public void setText(String name, String value) {
		_param_m.get(name).buf.set(0, value);
	}

	public void setObject(String name, DataObject value) {
		_param_m.get(name).buf.set(0, value);
	}

	public boolean hasValue(String name) {
		return _param_m.containsKey(name);
	}

	public boolean hasInt(String name) {
		return (_param_m.containsKey(name) && _param_m.get(name).type == INT);
	}

}
