package b2daq.database;

import java.util.ArrayList;
import java.util.HashMap;

import b2daq.core.Reader;
import b2daq.core.SerializableObject;
import b2daq.core.Writer;

public class DBObject implements SerializableObject {

	private int m_index;
	private int m_configid;
	private String m_configname;
	private ArrayList<String> m_name_v = new ArrayList<String>();
	private HashMap<String, Integer> m_type_m = new HashMap<String, Integer>();
	private HashMap<String, Object> m_value_m = new HashMap<String, Object>();
	private HashMap<String, DBObjectList> m_obj_v_m = new HashMap<String, DBObjectList>();

	public int getConfigId() {
		return m_configid;
	}

	public void setConfigId(int configid) {
		m_configid = configid;
	}

	public int getIndex() {
		return m_index;
	}

	public void setIndex(int index) {
		m_index = index;
	}

	public String getRuntype() {
		return m_configname;
	}

	public String getConfigName() {
		return m_configname;
	}

	public void setConfigName(String name) {
		m_configname = name;
	}

	public ArrayList<String> getFieldNames() {
		return m_name_v;
	}

	public void reset() {
		for (String name : m_name_v) {
			if (m_type_m.get(name) == FieldInfo.TABLE) {
				for (DBObject obj : m_obj_v_m.get(name).get())
					obj.reset();
			}
		}
		m_index = 0;
		m_configid = 0;
		m_configname = "";
		m_name_v = new ArrayList<String>();
		m_type_m = new HashMap<String, Integer>();
		m_value_m = new HashMap<String, Object>();
		m_obj_v_m = new HashMap<String, DBObjectList>();
	}

	public void print() {
		System.out.println(getConfigName() + " " + getConfigId());
		for (String name : m_name_v) {
			System.out.print(name + " ");
			int type = getType(name);
			switch (type) {
			case FieldInfo.BOOL:
				System.out.print(getBool(name));
				break;
			case FieldInfo.SHORT:
				System.out.print(getShort(name));
				break;
			case FieldInfo.INT:
				System.out.print(getInt(name));
				break;
			case FieldInfo.LONG:
				System.out.print(getLong(name));
				break;
			case FieldInfo.FLOAT:
				System.out.print(getFloat(name));
				break;
			case FieldInfo.TEXT:
				System.out.print(getText(name));
				break;
			case FieldInfo.TABLE:
			case FieldInfo.OBJECT: {
				DBObjectList obj_v = getObjects(name);
				if (type == FieldInfo.TABLE) {
					System.out.println(obj_v.getConfigName() + " "
							+ obj_v.getConfigId() + " " + obj_v.getNode() + " "
							+ obj_v.getTable());
				}
				for (DBObject obj : getObjects(name).get())
					obj.print();
				break;
			}
			case FieldInfo.ENUM:
				System.out.print(getEnum(name));
				break;
			}
			System.out.println();
		}
	}

	public void readObject(Reader reader) throws Exception {
		reset();
		m_index = reader.readInt();
		m_configid = reader.readInt();
		m_configname = reader.readString();
		int npar = reader.readInt();
		for (int i = 0; i < npar; i++) {
			String name = reader.readString();
			int type = reader.readInt();
			switch (type) {
			case FieldInfo.BOOL:
				addBool(name, reader.readBoolean());
				break;
			case FieldInfo.SHORT:
				addShort(name, reader.readShort());
				break;
			case FieldInfo.INT:
				addInt(name, reader.readInt());
				break;
			case FieldInfo.LONG:
				addLong(name, reader.readLong());
				break;
			case FieldInfo.FLOAT:
				addFloat(name, reader.readFloat());
				break;
			case FieldInfo.TEXT:
				addText(name, reader.readString());
				break;
			case FieldInfo.ENUM:
				addEnum(name, reader.readString());
				break;
			case FieldInfo.TABLE:
			case FieldInfo.OBJECT: {
				DBObjectList obj_v = new DBObjectList();
				obj_v.setConfigId(reader.readInt());
				obj_v.setConfigName(reader.readString());
				obj_v.setNode(reader.readString());
				obj_v.setTable(reader.readString());
				int nobj = reader.readInt();
				for (int n = 0; n < nobj; n++) {
					DBObject obj = new DBObject();
					reader.readObject(obj);
					obj_v.add(obj);
				}
				addObjects(name, obj_v, type);
			}
				;
				break;
			}
		}
	}

	public void writeObject(Writer writer) throws Exception {
		writer.writeInt(m_index);
		writer.writeInt(m_configid);
		writer.writeString(m_configname);
		writer.writeInt(m_name_v.size());
		for (String name : m_name_v) {
			int type = m_type_m.get(name);
			writer.writeString(name);
			writer.writeInt(type);
			switch (type) {
			case FieldInfo.BOOL:
				writer.writeBoolean(getBool(name));
				break;
			case FieldInfo.SHORT:
				writer.writeShort(getShort(name));
				break;
			case FieldInfo.INT:
				writer.writeInt(getInt(name));
				break;
			case FieldInfo.LONG:
				writer.writeLong(getLong(name));
				break;
			case FieldInfo.FLOAT:
				writer.writeFloat(getFloat(name));
				break;
			case FieldInfo.TEXT:
				writer.writeString(getText(name));
				break;
			case FieldInfo.ENUM:
				writer.writeString(getEnum(name));
				break;
			case FieldInfo.TABLE:
			case FieldInfo.OBJECT: {
				DBObjectList obj_v = m_obj_v_m.get(name);
				writer.writeInt(obj_v.getConfigId());
				writer.writeString(obj_v.getConfigName());
				writer.writeString(obj_v.getNode());
				writer.writeString(obj_v.getTable());
				writer.writeInt(obj_v.size());
				for (DBObject obj : m_obj_v_m.get(name).get()) {
					obj.writeObject(writer);
				}
			}
				break;
			}
		}
	}

	public int getType(String name) {
		if (!m_type_m.containsKey(name)) {
			System.err.println("No iterm = " + name);
			return 0;
		}
		return m_type_m.get(name);
	}

	public boolean hasField(String name) {
		return m_type_m.containsKey(name);
	}

	public boolean hasValue(String name) {
		return hasField(name) && m_type_m.get(name) != FieldInfo.TEXT
				&& m_type_m.get(name) != FieldInfo.TABLE;
	}

	public boolean hasText(String name) {
		return hasField(name) && m_type_m.get(name) == FieldInfo.TEXT;
	}

	public boolean hasEnum(String name) {
		return hasField(name) && m_type_m.get(name) == FieldInfo.ENUM;
	}

	public boolean hasObject(String name, int index) {
		return hasField(name)
				&& (m_type_m.get(name) == FieldInfo.TABLE || m_type_m.get(name) == FieldInfo.OBJECT)
				&& (index < 0 || index < m_obj_v_m.get(name).size());
	}

	public Object getValue(String name) {
		if (!hasValue(name))
			return null;
		return m_value_m.get(name);
	}

	public boolean getBool(String name) {
		Object value = getValue(name);
		if (value == null)
			return false;
		return (Boolean) value;
	}

	public short getShort(String name) {
		Object value = getValue(name);
		if (value == null)
			return 0;
		return (Short) value;
	}

	public int getInt(String name) {
		Object value = getValue(name);
		if (value == null)
			return 0;
		return (Integer) value;
	}

	public long getLong(String name) {
		Object value = getValue(name);
		if (value == null)
			return 0;
		return (Long) value;
	}

	public float getFloat(String name) {
		Object value = getValue(name);
		if (value == null)
			return 0;
		return (Float) value;
	}

	public String getText(String name) {
		if (!hasText(name))
			return "";
		return (String) m_value_m.get(name);
	}

	public String getEnum(String name) {
		if (!hasEnum(name))
			return "";
		return (String) m_value_m.get(name);
	}

	public DBObjectList getObjects(String name) {
		if (!hasObject(name, 0))
			return null;
		return m_obj_v_m.get(name);
	}

	public void addValue(String name, int type, Object value) {
		if (!m_type_m.containsKey(name)) {
			m_name_v.add(name);
			m_type_m.put(name, type);
			m_value_m.put(name, value);
		} else {
			setValue(name, type, value);
		}
	}

	public void addBool(String name, boolean value) {
		addValue(name, FieldInfo.BOOL, value);
	}

	public void addShort(String name, int value) {
		addValue(name, FieldInfo.SHORT, value);
	}

	public void addInt(String name, int value) {
		addValue(name, FieldInfo.INT, value);
	}

	public void addLong(String name, long value) {
		addValue(name, FieldInfo.LONG, value);
	}

	public void addFloat(String name, float value) {
		addValue(name, FieldInfo.FLOAT, value);
	}

	public void addText(String name, String value) {
		addValue(name, FieldInfo.TEXT, value);
	}

	public void addEnum(String name, String value) {
		addValue(name, FieldInfo.ENUM, value);
	}

	public void addObjects(String name, DBObjectList obj_v, int type) {
		if (!m_type_m.containsKey(name)) {
			m_name_v.add(name);
			m_type_m.put(name, type);
			m_obj_v_m.put(name, obj_v);
		}
	}

	public void setValue(String name, int type, Object value) {
		if (hasField(name) && type == m_type_m.get(name)
				&& type != FieldInfo.TABLE) {
			m_value_m.put(name, value);
		}
	}

	public void setBool(String name, boolean value) {
		setValue(name, FieldInfo.BOOL, value);
	}

	public void setShort(String name, short value) {
		setValue(name, FieldInfo.SHORT, value);
	}

	public void setInt(String name, int value) {
		setValue(name, FieldInfo.INT, value);
	}

	public void setLong(String name, long value) {
		setValue(name, FieldInfo.LONG, value);
	}

	public void setFloat(String name, float value) {
		setValue(name, FieldInfo.FLOAT, value);
	}

	public void setText(String name, String value) {
		setValue(name, FieldInfo.TEXT, value);
	}

	public void setEnum(String name, String value) {
		setValue(name, FieldInfo.ENUM, value);
	}

	public void setObject(String name, int index, DBObject obj) {
		if (hasObject(name, index)) {
			m_obj_v_m.get(name).get().set(index, obj);
		}
	}

}
