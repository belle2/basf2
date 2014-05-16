package b2daq.core;

import java.util.ArrayList;
import java.util.HashMap;

import b2daq.core.Reader;
import b2daq.core.SerializableObject;
import b2daq.core.Writer;

public class NSMData implements SerializableObject {

	final static public int CHAR = 1;
	final static public int INT16 = 2;
	final static public int INT32 = 4;
	final static public int INT64 = 8;
	final static public int BYTE8 = 101;
	final static public int UINT16 = 102;
	final static public int UINT32 = 104;
	final static public int UINT64 = 108;
	final static public int FLOAT = 204;
	final static public int DOUBLE = 208;
	
	public class ParamInfo {
		public int type;
		public int length;
		public ArrayList<Object> buf = new ArrayList<Object>();
	};

	private int _revision;
	private String _format = "";
	private String _dataname = "";
	private int _size = 0;
	private HashMap<String, ParamInfo> _param_m = new HashMap<String, ParamInfo>();
	private ArrayList<String> _name_v = new ArrayList<String>();

	public NSMData() {
		_revision = 0;
	}

	public NSMData(String format, String dataname, int revision) {
		_format = format;
		_dataname = dataname;
		_revision = 0;
	}

	public NSMData(NSMData obj) {
		_revision = obj._revision;
		_format = obj._format;
		_dataname = obj._dataname;
		for (String name : obj._name_v) {
			ParamInfo info = obj._param_m.get(name);
			add(name, info.buf, info.type, info.length);
		}
	}

	public void print() {
		System.out.println(_dataname+"("+_format+","+_revision+")");
		for (String name : _name_v) {
			ParamInfo info = _param_m.get(name);
			System.out.print(name+ " " + getTypeName(info.type));
			if (info.length>0) System.out.print("["+info.length +"] = {");
			else System.out.print(" = ");
			ArrayList<Object> buf = info.buf;
			int length = info.length;
			if (length == 0) length = 1;
			for (int i = 0; i < length; i++) {
				switch (info.type) {
				case INT64: 
				case UINT64: System.out.print((Long) buf.get(i)); break;
				case INT32:
				case UINT32: System.out.print((Integer) buf.get(i)); break;
				case INT16: 
				case UINT16: System.out.print((Short) buf.get(i)); break;
				case CHAR:
				case BYTE8: System.out.print((int)(Character) buf.get(i)); break;
				case FLOAT: System.out.print((Float) buf.get(i));break;
				case DOUBLE: System.out.print((Double) buf.get(i)); break;
				default: break;
				}
				if (i < length - 1) System.out.print(", ");
			}
			if (info.length>0) System.out.print("}");
			System.out.println();
		}
	}
	private String getTypeName(int type) {
		switch (type) {
		case INT64:  return "int64"; 
		case UINT64: return "uint64";
		case INT32:  return "int32";
		case UINT32: return "uint32";
		case INT16:  return "int16";
		case UINT16: return "uint16";
		case CHAR:   return "char";
		case BYTE8:  return "byte8";
		case FLOAT:  return "float";
		case DOUBLE: return "double";
		default: break;
		}
		return "?";
	}

	public void readObject(Reader reader) throws Exception {
		_dataname = reader.readString();
		_format = reader.readString();
		_revision = reader.readInt();
		_size = reader.readInt();
		int npars = reader.readInt();
		for (int n = 0; n < npars; n++) {
			String name = reader.readString();
			ParamInfo info = new ParamInfo();
			info.type = (Integer) reader.readInt();
			info.length = reader.readInt();
			if (!hasValue(name)) {
				add(name, null, info.type, info.length);
			}
			info = _param_m.get(name);
			ArrayList<Object> buf = info.buf;
			int length = info.length;
			if (length == 0) length = 1;
			for (int i = 0; i < length; i++) {
				switch (info.type) {
				case INT64: 
				case UINT64: buf.add(i, reader.readLong()); break;
				case INT32: 
				case UINT32: buf.add(i, reader.readInt()); break;
				case INT16:
				case UINT16: buf.add(i, reader.readShort()); break;
				case CHAR: 
				case BYTE8: buf.add(i, reader.readChar()); break;
				case FLOAT: buf.add(i, reader.readFloat()); break;
				case DOUBLE: buf.add(i, reader.readDouble()); break;
				default: break;
				}
			}
		}
	}

	public void writeObject(Writer writer) throws Exception {
		writer.writeString(_dataname);
		writer.writeString(_format);
		writer.writeInt(_revision);
		writer.writeInt(_size);
		writer.writeInt(_name_v.size());
		for (String name : _name_v) {
			ParamInfo info = _param_m.get(name);
			writer.writeString(name);
			writer.writeInt(info.type);
			writer.writeInt(info.length);
			ArrayList<Object> buf = info.buf;
			int length = info.length;
			if (length == 0) length = 1;
			for (int i = 0; i < length; i++) {
				switch (info.type) {
				case INT64: 
				case UINT64: writer.writeLong((Long) buf.get(i)); break;
				case INT32:
				case UINT32: writer.writeInt((Integer) buf.get(i)); break;
				case INT16: 
				case UINT16: writer.writeShort((Short) buf.get(i)); break;
				case CHAR:
				case BYTE8: writer.writeChar((Character) buf.get(i)); break;
				case FLOAT: writer.writeFloat((Float) buf.get(i));break;
				case DOUBLE: writer.writeDouble((Double) buf.get(i)); break;
				default: break;
				}
			}
		}
	}

	public int getRevision() {
		return _revision;
	}

	public void setRevision(int revision) {
		_revision = revision;
	}

	public HashMap<String, ParamInfo> getParams() {
		return _param_m;
	}

	public ParamInfo getParam(String name) {
		return _param_m.get(name);
	}

	public ArrayList<String> getParamNames() {
		return _name_v;
	}

	public int length(String name) {
		return _param_m.get(name).length;
	}
	
	public Object get(String name) {
		return get(name);
	}

	public double getValue(String name, int index) {
		ParamInfo info = _param_m.get(name);
		int type = info.type;
		if (type == NSMData.INT64 || type == NSMData.UINT64 ) {
			return getLong(name, index);
		} else if (type == NSMData.INT32 || type == NSMData.UINT32 ) {
			return getInt(name, index);
		} else if (type == NSMData.INT16 || type == NSMData.UINT16 ) {
			return getShort(name, index);
		} else if (type == NSMData.CHAR || type == NSMData.BYTE8 ) {
			return getChar(name, index);
		} else if (type == NSMData.FLOAT ) {
			return getFloat(name, index);
		} else if (type == NSMData.DOUBLE ) {
			return getDouble(name, index);
		}
		return -1;
	}
	
	public Object get(String name, int i) {
		return _param_m.get(name).buf.get(i);
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

	public long getLong(String name) {
		return (Long) get(name);
	}

	public float getFloat(String name) {
		return (Float) get(name);
	}

	public double getDouble(String name) {
		return (Double) get(name);
	}

	public boolean getBool(String name, int i) {
		return (Boolean) get(name, i);
	}

	public char getChar(String name, int i) {
		return (Character) get(name, i);
	}

	public String getText(String name) {
		if (getParam(name).type == CHAR && getParam(name).length > 0) {
			return getParam(name).buf.toArray().toString();
		} else {
			return "";
		}
	}

	public short getShort(String name, int i) {
		return (Short) get(name, i);
	}

	public int getInt(String name, int i) {
		return (Integer) get(name, i);
	}

	public long getLong(String name, int i) {
		return (Long) get(name, i);
	}

	public float getFloat(String name, int i) {
		return (Float) get(name, i);
	}

	public double getDouble(String name, int i) {
		return (Double) get(name, i);
	}

	public void addLong(String name, long value) {
		add(name, value, INT64, 0);
	}

	public void addInt(String name, int value) {
		add(name, value, INT32, 0);
	}

	public void addShort(String name, int value) {
		add(name, value, INT16, 0);
	}

	public void addChar(String name, int value) {
		add(name, value, CHAR, 0);
	}

	public void addDouble(String name, double value) {
		add(name, value, DOUBLE, 0);
	}

	public void addFloat(String name, double value) {
		add(name, value, FLOAT, 0);
	}

	public void addIntArray(String name, int value, int length) {
		add(name, value, INT32, length);
	}

	public void addShortArray(String name, int value, int length) {
		add(name, value, INT16, length);
	}

	public void addCharArray(String name, int value, int length) {
		add(name, value, CHAR, length);
	}

	public void addDoubleArray(String name, double value, int length) {
		add(name, value, DOUBLE, length);
	}

	public void addFloatArray(String name, double value, int length) {
		add(name, value, FLOAT, length);
	}

	void add(String name, Object value, int type, int length) {
		ParamInfo info = new ParamInfo();
		info.type = type;
		info.length = length;
		if (length == 0) length = 1;
		for (int i = 0; i < length; i++) {
			if (value == null) break;
			switch (info.type) {
			case INT64:
			case UINT64: info.buf.add((Long) value); break;
			case INT32: 
			case UINT32: info.buf.add((Integer) value); break;
			case INT16:
			case UINT16: info.buf.add((Short) value); break;
			case CHAR:
			case BYTE8: info.buf.add((Character) value); break;
			case FLOAT: info.buf.add((Float) value); break;
			case DOUBLE: info.buf.add((Double) value); break;
			default: break;
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

	public boolean hasValue(String name) {
		return _param_m.containsKey(name);
	}

	public String getName() {
		return _dataname;
	}

	public String getFormat() {
		return _format;
	}

}
