package b2daq.database;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

import b2daq.core.Reader;
import b2daq.core.Writer;

public class ConfigObject extends DBObject {

    private HashMap<String, Object> m_value_m = new HashMap<>();
    private HashMap<String, ArrayList<ConfigObject>> m_obj_v_m
            = new HashMap<>();

    @Override
    public void reset() {
        super.reset();
        m_value_m = new HashMap<>();
        m_obj_v_m = new HashMap<>();
    }

    @Override
    public void readObject(Reader reader) throws IOException {
        reset();
        setId(reader.readInt());
        setName(reader.readString());
        setNode(reader.readString());
        setTable(reader.readString());
        setRevision(reader.readInt());
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
                    addText(name, reader.readString());
                    break;
                case FieldInfo.OBJECT: {
                    ArrayList<ConfigObject> obj_v = new ArrayList<>();
                    int nobj = reader.readInt();
                    for (int n = 0; n < nobj; n++) {
                        ConfigObject obj = new ConfigObject();
                        obj.readObject(reader);
                        obj.setIndex(n);
                        obj_v.add(obj);
                    }
                    addObjects(name, obj_v);
                }
                ;
                break;
                case FieldInfo.ENUM: {
                    HashMap<String, Integer> enum_m = new HashMap<>();
                    int nenum = reader.readInt();
                    for (int n = 0; n < nenum; n++) {
                        String ename = reader.readString();
                        int id = reader.readInt();
                        enum_m.put(ename, id);
                    }
                    addEnum(name, reader.readString(), enum_m);
                }
                break;
                default:
                    break;
            }
        }
    }

    @Override
    public void writeObject(Writer writer) throws IOException {
        writer.writeInt(getId());
        writer.writeString(getName());
        writer.writeString(getNode());
        writer.writeString(getTable());
        writer.writeInt(getRevision());
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
                    ArrayList<ConfigObject> obj_v = m_obj_v_m.get(name);
                    writer.writeInt(obj_v.size());
                    for (ConfigObject obj : obj_v) {
                        obj.writeObject(writer);
                    }
                }
                ;
                break;
                case FieldInfo.ENUM: {
                    HashMap<String, Integer> enum_m = getEnumList(name);
                    writer.writeInt(enum_m.size());
                    for (String label : enum_m.keySet()) {
                        writer.writeString(label);
                        writer.writeInt(getEnumId(label));
                    }
                    writer.writeString(getEnum(name));
                }
                ;
                break;
                default:
                    break;
            }
        }
    }

    @Override
    public void setObject(String name, int index, DBObject obj) {
        if (hasObject(name, index)) {
            m_obj_v_m.get(name).set(index, (ConfigObject) obj);
        }
    }

    @Override
    public DBObject getObject(String name, int i) {
        return m_obj_v_m.get(name).get(i);
    }

    @Override
    public int getNObjects(String name) {
        return m_obj_v_m.get(name).size();
    }

    @Override
    public void addValue(String name, int type, Object value) {
        FieldInfo.Property pro = new FieldInfo.Property(type, 0, 0);
        int size = pro.getTypeSize();
        /*
        if (size <= 0) {
            return;
        }
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
        return (String)m_value_m.get(name);
    }

    @Override
    public String getEnum(String name) {
        if (!hasEnum(name)) {
            return "";
        }
        return (String)m_value_m.get(name);
    }

    public ArrayList<ConfigObject> getObjects(String name) {
        return m_obj_v_m.get(name);
    }

    public void addObject(String name, ConfigObject obj) {
        if (!hasField(name)) {
            add(name, new FieldInfo.Property(FieldInfo.OBJECT, 0));
            m_obj_v_m.put(name, new ArrayList<>());
        }
        m_obj_v_m.get(name).add(obj);
        getProperty(name).setLength(m_obj_v_m.get(name).size());
    }

    public void addObjects(String name, ArrayList<ConfigObject> obj_v) {
        if (!hasField(name)) {
            add(name, new FieldInfo.Property(FieldInfo.OBJECT, obj_v.size()));
        }
        m_obj_v_m.put(name, obj_v);
    }

}
