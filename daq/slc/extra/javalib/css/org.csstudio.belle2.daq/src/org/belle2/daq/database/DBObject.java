package org.belle2.daq.database;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

import org.belle2.daq.base.Reader;
import org.belle2.daq.base.Writer;


public class DBObject extends AbstractDBObject {

	private HashMap<String, Object> m_value_m = new HashMap<>();
	private HashMap<String, ArrayList<DBObject>> m_obj_v_m = new HashMap<>();

	@Override
	public void reset() {
		super.reset();
		m_value_m = new HashMap<>();
		m_obj_v_m = new HashMap<>();
	}

	@Override
	public void readObject(Reader reader) throws IOException {
		try {
			reset();
			setPath(reader.readString());
			setName(reader.readString());
			int npar = reader.readInt();
			for (int i = 0; i < npar; i++) {
				String name = reader.readString();
				int type = reader.readInt();
				switch (type) {
				case FieldInfo.BOOL:
					addBool(name, reader.readBoolean());
					break;
				case FieldInfo.CHAR:
					addChar(name, reader.readChar());
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
				case FieldInfo.DOUBLE:
					addDouble(name, reader.readDouble());
					break;
				case FieldInfo.TEXT:
					String value = reader.readString();
					addText(name, value);
					break;
				case FieldInfo.OBJECT: {
					ArrayList<DBObject> obj_v = new ArrayList<>();
					int nobj = reader.readInt();
					try {
						for (int n = 0; n < nobj; n++) {
							DBObject obj = new DBObject();
							obj.readObject(reader);
							obj.setIndex(n);
							obj_v.add(obj);
						}
					} catch (IOException e) {
						System.err.println(getName() + " " + nobj);
					}
					addObjects(name, obj_v);
				}
					;
					break;
				default:
					break;
				}
			}
		} catch (IOException e) {
			throw (e);
		}
	}

	@Override
	public void writeObject(Writer writer) throws IOException {
		writer.writeString(getPath());
		writer.writeString(getName());
		ArrayList<String> name_v = getFieldNames();
		writer.writeInt(name_v.size());
		for (String name : name_v) {
			int type = getProperty(name).getType();
			writer.writeString(name);
			writer.writeInt(type);
			switch (type) {
			case FieldInfo.BOOL:
				writer.writeBoolean(getBool(name));
				break;
			case FieldInfo.CHAR:
				writer.writeChar(getChar(name));
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
			case FieldInfo.DOUBLE:
				writer.writeDouble(getDouble(name));
				break;
			case FieldInfo.TEXT:
				writer.writeString(getText(name));
				break;
			case FieldInfo.OBJECT: {
				ArrayList<DBObject> obj_v = m_obj_v_m.get(name);
				writer.writeInt(obj_v.size());
				for (DBObject obj : obj_v) {
					obj.writeObject(writer);
				}
			}
				;
				break;
			default:
				break;
			}
		}
	}

	@Override
	public void setObject(String name, int index, AbstractDBObject obj) {
		if (hasObject(name, index)) {
			m_obj_v_m.get(name).set(index, (DBObject) obj);
		}
	}

	@Override
	public AbstractDBObject getObject(String name, int i) {
		return m_obj_v_m.get(name).get(i);
	}

	@Override
	public int getNObjects(String name) {
		return m_obj_v_m.get(name).size();
	}

	@Override
	public void addValue(String name, int type, Object value) {
		FieldInfo.Property pro = new FieldInfo.Property(type, 0, 0);
		/*
		 * int size = pro.getTypeSize(); if (size <= 0) { return; }
		 */
		if (!hasField(name)) {
			add(name, pro);
		}
		m_value_m.put(name, value);
	}

	@Override
	public void setValue(String name, int type, Object value) {
		FieldInfo.Property pro = getProperty(name);
		int size = pro.getTypeSize();
		if (hasField(name) && size > 0) {
			m_value_m.put(name, value);
		}
	}

	@Override
	public Object getValue(String name) {
		if (!hasValue(name)) {
			return null;
		}
		return m_value_m.get(name);
	}

	@Override
	public String getText(String name) {
		if (!hasText(name)) {
			return "";
		}
		return (String) m_value_m.get(name);
	}

	public ArrayList<DBObject> getObjects(String name) {
		return m_obj_v_m.get(name);
	}

	public void addObject(String name, DBObject obj) {
		if (!hasField(name)) {
			add(name, new FieldInfo.Property(FieldInfo.OBJECT, 0));
			m_obj_v_m.put(name, new ArrayList<DBObject>());
		}
		m_obj_v_m.get(name).add(obj);
		getProperty(name).setLength(m_obj_v_m.get(name).size());
	}

	public void addObjects(String name, ArrayList<DBObject> obj_v) {
		if (!hasField(name)) {
			add(name, new FieldInfo.Property(FieldInfo.OBJECT, obj_v.size()));
		}
		m_obj_v_m.put(name, obj_v);
	}

}
