package org.belle2.daq.nsm;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;

import java.util.Arrays;

import org.belle2.daq.base.Reader;
import org.belle2.daq.base.Writer;
import org.belle2.daq.database.AbstractDBObject;
import org.belle2.daq.database.FieldInfo;

public class NSMData extends AbstractDBObject {

    private int m_revision;
    private String m_format = "";
    private int m_size = 0;
    private HashMap<String, ArrayList<Object>> m_data_m = new HashMap<>();

    public NSMData() {
        this("", "", 0);
    }

    public NSMData(String dataname, String format, int revision) {
        setName(dataname);
        setFormat(format);
        setRevision(revision);
    }

    public String getFormat() {
        return m_format;
    }

    public final void setFormat(String format) {
        m_format = format;
    }

    public int getRevision() {
        return m_revision;
    }

    public final void setRevision(int revision) {
        m_revision = revision;
    }

    @Override
    public void readObject(Reader reader) throws IOException {
        reset();
        setName(reader.readString());
        setFormat(reader.readString());
        setRevision(reader.readInt());
        int size = reader.readInt();
        if (size <= 0) {
            System.out.println("size error : " + size);
        }
        m_data_m = new HashMap<>();
        int npars = reader.readInt();
        for (int n = 0; n < npars; n++) {
            String name = reader.readString();
            int type = reader.readInt();
            int length = reader.readInt();
            int offset = reader.readInt();
            if (length == 0) {
                length = 1;
            }
            if (type == FieldInfo.OBJECT) {
                ArrayList<Object> data_v = new ArrayList<>();
                try {
                    for (int i = 0; i < length; i++) {
                        NSMData data = new NSMData();
                        reader.readObject(data);
                        data_v.add(data);
                    }
                } catch (IOException e) {
                }
                add(name, new FieldInfo.Property(type, length, offset));
                m_data_m.put(name, data_v);
            } else {
                for (int i = 0; i < length; i++) {
                    switch (type) {
                        case FieldInfo.LONG:
                            addValue(name, type, reader.readLong());
                            break;
                        case FieldInfo.INT:
                            addValue(name, type, reader.readInt());
                            break;
                        case FieldInfo.SHORT:
                            addValue(name, type, reader.readShort());
                            break;
                        case FieldInfo.CHAR:
                            addValue(name, type, reader.readChar());
                            break;
                        case FieldInfo.FLOAT:
                            addValue(name, type, reader.readFloat());
                            break;
                        case FieldInfo.DOUBLE:
                            addValue(name, type, reader.readDouble());
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }

    @Override
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
            if (pro.getType() == FieldInfo.OBJECT) {
                for (int i = 0; i < length; i++) {
                    writer.writeObject(getObject(name, i));
                }
            } else {
                for (int i = 0; i < length; i++) {
                    switch (pro.getType()) {
                        case FieldInfo.CHAR:
                            writer.writeChar((Character) buf.get(i));
                            break;
                        case FieldInfo.SHORT:
                            writer.writeShort((Short) buf.get(i));
                            break;
                        case FieldInfo.LONG:
                            writer.writeLong((Long) buf.get(i));
                            break;
                        case FieldInfo.INT:
                            writer.writeInt((Integer) buf.get(i));
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }

    @Override
    public void addText(String name, String value) {
    }

    @Override
    public void setObject(String name, int index, AbstractDBObject obj) {
    }

    @Override
    public String getText(String name) {
        if (getProperty(name).getType() == FieldInfo.CHAR
                && getProperty(name).getLength() > 0) {
            return Arrays.toString(m_data_m.get(name).toArray());
        } else {
            return "";
        }
    }

    @Override
    public AbstractDBObject getObject(String name, int i) {
        return (AbstractDBObject) m_data_m.get(name).get(i);
    }

    public ArrayList<Object> getObjects(String name) {
        return m_data_m.get(name);
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
            m_data_m.put(name, new ArrayList<>());
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

    public Object find(String name_in) {
        String name_out = name_in;
        if (name_out.contains(".")) {
            String[] str = name_out.split("\\.");
            String[] sstr = str[0].split("\\[");
            ArrayList<Object> data_v = getObjects(sstr[0]);
            int index = 0;
            if (sstr.length > 1) {
                String s = sstr[1].split("\\]")[0];
                index = Integer.parseInt(s);
            }
            name_out = name_in.substring(name_in.indexOf(".") + 1);
            NSMData data = ((NSMData) data_v.get(index));
            if (data != null) return data.find(name_out);
            else return null;
        }
        int index = 0;
        if (name_out.contains("[")) {
            String[] str = name_out.split("\\[");
            name_out = str[0];
            index = Integer.parseInt(str[1].split("\\]")[0]);
        }
        if (!hasValue(name_out)) {
            return null;
        }
        return getValue(name_out, index);
    }

}
