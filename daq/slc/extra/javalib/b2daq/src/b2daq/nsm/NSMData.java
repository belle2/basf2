package b2daq.nsm;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

import b2daq.core.Reader;
import b2daq.core.Writer;
import b2daq.database.DBObject;
import b2daq.database.FieldInfo;

public class NSMData extends DBObject {

    private int m_size = 0;
    private HashMap<String, ArrayList<Object>> m_data_m
            = new HashMap<String, ArrayList<Object>>();

    public NSMData() {
        this("", "", 0);
    }

    public NSMData(String dataname, String format, int revision) {
        setConfig(false);
        setName(dataname);
        setFormat(format);
        setRevision(revision);
    }

    public String getFormat() {
        return getTable();
    }

    public void setFormat(String format) {
        setTable(format);
    }

    public void readObject(Reader reader) throws IOException {
        reset();
        setName(reader.readString());
        setFormat(reader.readString());
        setRevision(reader.readInt());
        int size = reader.readInt();
        if (size <= 0) {
            System.out.println("size error : " + size);
        }
        m_data_m = new HashMap<String, ArrayList<Object>>();
        int npars = reader.readInt();
        for (int n = 0; n < npars; n++) {
            String name = reader.readString();
            int type = reader.readInt();
            int length = reader.readInt();
            int offset = reader.readInt();
            if (length == 0) {
                length = 1;
            }
            if (type == FieldInfo.NSM_OBJECT) {
                ArrayList<Object> data_v = new ArrayList<Object>();
                try {
                    for (int i = 0; i < length; i++) {
                        NSMData data = new NSMData();
                        reader.readObject(data);
                        data_v.add(data);
                    }
                } catch (Exception e) {
                    e.printStackTrace();
                }
                add(name, new FieldInfo.Property(type, length, offset));
                m_data_m.put(name, data_v);
            } else {
                for (int i = 0; i < length; i++) {
                    switch (type) {
                        case FieldInfo.NSM_INT64:
                        case FieldInfo.NSM_UINT64:
                            addValue(name, type, reader.readLong());
                            break;
                        case FieldInfo.NSM_INT32:
                        case FieldInfo.NSM_UINT32:
                            addValue(name, type, reader.readInt());
                            break;
                        case FieldInfo.NSM_INT16:
                        case FieldInfo.NSM_UINT16:
                            addValue(name, type, reader.readShort());
                            break;
                        case FieldInfo.NSM_CHAR:
                        case FieldInfo.NSM_BYTE8:
                            addValue(name, type, reader.readChar());
                            break;
                        case FieldInfo.NSM_FLOAT:
                            addValue(name, type, reader.readFloat());
                            break;
                        case FieldInfo.NSM_DOUBLE:
                            addValue(name, type, reader.readDouble());
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }

    public void writeObject(Writer writer) throws IOException {
        writer.writeString(getName());
        writer.writeString(getFormat());
        writer.writeInt(getRevision());
        writer.writeInt(m_size);
        ArrayList<String> name_v = getFieldNames();
        writer.writeInt(name_v.size());
        for (String name : name_v) {
            writer.writeString(name);
            FieldInfo.Property pro = getProperty(name);
            writer.writeInt(pro.getType());
            writer.writeInt(pro.getLength());
            writer.writeInt(pro.getOffset());
            int length = pro.getLength();
            ArrayList<Object> buf = m_data_m.get(name);
            if (length == 0) {
                length = 1;
            }
            if (pro.getType() == FieldInfo.NSM_OBJECT) {
                for (int i = 0; i < length; i++) {
                    writer.writeObject(getObject(name, i));
                }
            } else {
                for (int i = 0; i < length; i++) {
                    switch (pro.getType()) {
                        case FieldInfo.NSM_CHAR:
                            writer.writeChar((Character) buf.get(i));
                            break;
                        case FieldInfo.NSM_INT16:
                            writer.writeShort((Short) buf.get(i));
                            break;
                        case FieldInfo.NSM_INT64:
                            writer.writeLong((Long) buf.get(i));
                            break;
                        case FieldInfo.NSM_INT32:
                            writer.writeInt((Integer) buf.get(i));
                            break;
                        case FieldInfo.NSM_BYTE8:
                            writer.writeChar((Character) buf.get(i));
                            break;
                        case FieldInfo.NSM_UINT16:
                            writer.writeShort((Short) buf.get(i));
                            break;
                        case FieldInfo.NSM_UINT32:
                            writer.writeInt((Integer) buf.get(i));
                            break;
                        case FieldInfo.NSM_UINT64:
                            writer.writeLong((Long) buf.get(i));
                            break;
                        case FieldInfo.NSM_FLOAT:
                            writer.writeFloat((Float) buf.get(i));
                            break;
                        case FieldInfo.NSM_DOUBLE:
                            writer.writeDouble((Double) buf.get(i));
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }

    public void addText(String name, String value) {
    }

    public void addEnum(String name, String value) {
    }

    public void setObject(String name, int index, DBObject obj) {
    }

    public String getEnum(String name) {
        return "";
    }

    public String getText(String name) {
        if (getProperty(name).getType() == FieldInfo.NSM_CHAR
                && getProperty(name).getLength() > 0) {
            return m_data_m.get(name).toArray().toString();
        } else {
            return "";
        }
    }

    @Override
    public DBObject getObject(String name, int i) {
        return (DBObject) m_data_m.get(name).get(i);
    }

    @Override
    public int getNObjects(String name) {
        return m_data_m.get(name).size();
    }

    @Override
    public void addValue(String name, int type, Object value) {
        FieldInfo.Property pro = new FieldInfo.Property(type, 0, 0);
        int size = pro.getTypeSize();
        if (size <= 0) {
            return;
        }
        if (!hasField(name)) {
            add(name, pro);
            m_data_m.put(name, new ArrayList<Object>());
        }
        m_data_m.get(name).add(value);
    }

    @Override
    public void setValue(String name, int type, Object value) {
        FieldInfo.Property pro = getProperty(name);
        int size = pro.getTypeSize();
        if (hasField(name) && size > 0) {
            m_data_m.get(name).add(value);
        }
    }

    @Override
    public Object getValue(String name) {
        return getValue(name, 0);
    }

    public Object getValue(String name, int index) {
        if (!hasValue(name)) {
            return null;
        }
        return m_data_m.get(name).get(index);
    }

    public boolean getBool(String name, int index) {
        Object value = getValue(name, index);
        if (value == null) {
            return false;
        }
        return (Boolean) value;
    }

    public char getChar(String name, int index) {
        Object value = getValue(name, index);
        if (value == null) {
            return 0;
        }
        return (Character) value;
    }

    public short getShort(String name, int index) {
        Object value = getValue(name, index);
        if (value == null) {
            return 0;
        }
        return (Short) value;
    }

    public int getInt(String name, int index) {
        Object value = getValue(name, index);
        if (value == null) {
            return 0;
        }
        return (Integer) value;
    }

    public long getLong(String name, int index) {
        Object value = getValue(name, index);
        if (value == null) {
            return 0;
        }
        return (Long) value;
    }

    public float getFloat(String name, int index) {
        Object value = getValue(name, index);
        if (value == null) {
            return 0;
        }
        return (Float) value;
    }

    public double getDouble(String name, int index) {
        Object value = getValue(name, index);
        if (value == null) {
            return 0;
        }
        return (Double) value;
    }

}
