package b2daq.database;

import java.util.ArrayList;
import java.util.HashMap;

import b2daq.core.Serializable;

public abstract class DBObject implements Serializable {

    private int m_index;
    private int m_id;
    private int m_revision;
    private String m_name = "";
    private String m_node = "";
    private String m_table = "";
    private boolean m_isconfig;
    private ArrayList<String> m_name_v = new ArrayList<String>();
    private HashMap<String, FieldInfo.Property> m_pro_m = new HashMap<String, FieldInfo.Property>();
    private HashMap<String, HashMap<String, Integer>> m_enum_m_m = new HashMap<String, HashMap<String, Integer>>();

    public int getId() {
        return m_id;
    }

    public void setId(int id) {
        m_id = id;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getNode() {
        return m_node;
    }

    public void setNode(String node) {
        m_node = node;
    }

    public String getTable() {
        return m_table;
    }

    public void setTable(String table) {
        m_table = table;
    }

    public int getIndex() {
        return m_index;
    }

    public void setIndex(int index) {
        m_index = index;
    }

    public int getRevision() {
        return m_revision;
    }

    public void setRevision(int revision) {
        m_revision = revision;
    }

    public void setConfig(boolean isconfig) {
        m_isconfig = isconfig;
    }

    public ArrayList<String> getFieldNames() {
        return m_name_v;
    }

    public FieldInfo.Property getProperty(String name) {
        return m_pro_m.get(name);
    }

    public boolean hasField(String name) {
        return m_pro_m.containsKey(name);
    }

    public boolean hasArray(String name) {
        return hasField(name) && !hasObject(name) && !hasText(name);
    }

    public boolean hasValue(String name) {
        return hasField(name)
                && m_pro_m.get(name).getType() != FieldInfo.TEXT
                && m_pro_m.get(name).getType() != FieldInfo.ENUM
                && m_pro_m.get(name).getType() != FieldInfo.OBJECT
                && m_pro_m.get(name).getType() != FieldInfo.NSM_OBJECT;
    }

    public boolean hasText(String name) {
        return hasField(name) && m_pro_m.get(name).getType() == FieldInfo.TEXT;
    }

    public boolean hasEnum(String name) {
        return hasField(name) && m_pro_m.get(name).getType() == FieldInfo.ENUM;
    }

    public boolean hasObject(String name, int index) {
        return hasField(name) && (m_pro_m.get(name).getType() == FieldInfo.OBJECT
                || m_pro_m.get(name).getType() == FieldInfo.NSM_OBJECT)
                && (index < 0 || index < getNObjects(name));
    }

    public boolean hasObject(String name) {
        return hasObject(name, 0);
    }

    public HashMap<String, Integer> getEnumList(String name) {
        return m_enum_m_m.get(name);
    }

    public int getEnumId(String name) {
        if (!hasEnum(name)) {
            return 0;
        }
        return m_enum_m_m.get(name).get(getEnum(name));
    }

    public void setEnum(String name, int value) {
        if (hasEnum(name)) {
            HashMap<String, Integer> enum_m = m_enum_m_m.get(name);
            for (String label : enum_m.keySet()) {
                if (value == enum_m.get(label)) {
                    setEnum(name, label);
                    return;
                }
            }
        }
    }

    public void addEnumList(String name, HashMap<String, Integer> enum_m) {
        if (!m_enum_m_m.containsKey(name)) {
            m_enum_m_m.put(name, enum_m);
        }
    }

    public void addEnumList(String name, String str) {
        if (!m_enum_m_m.containsKey(name)) {
            String[] name_v = str.split(",");
            HashMap<String, Integer> enum_m = new HashMap<String, Integer>();
            for (int i = 0; i < name_v.length; i++) {
                enum_m.put(name_v[i], i + 1);
            }
            addEnumList(name, enum_m);
        }
    }

    public void add(String name, FieldInfo.Property pro) {
        if (!hasField(name)) {
            m_name_v.add(name);
            m_pro_m.put(name, pro);
        }
    }

    public String getValueText(String name) {
        if (hasField(name)) {
            switch (getProperty(name).getType()) {
                case FieldInfo.BOOL:
                    return getBool(name) ? "true" : "false";
                case FieldInfo.CHAR:
                    return "" + (int) getChar(name);
                case FieldInfo.SHORT:
                    return "" + (int) getShort(name);
                case FieldInfo.INT:
                    return "" + (int) getInt(name);
                case FieldInfo.LONG:
                    return "" + getLong(name);
                case FieldInfo.FLOAT:
                    return "" + getFloat(name);
                case FieldInfo.DOUBLE:
                    return "" + getDouble(name);
                case FieldInfo.NSM_CHAR:
                    return "" + (int) getChar(name);
                case FieldInfo.NSM_INT16:
                    return "" + (int) getShort(name);
                case FieldInfo.NSM_INT32:
                    return "" + (int) getInt(name);
                case FieldInfo.NSM_INT64:
                    return "" + getLong(name);
                case FieldInfo.NSM_BYTE8:
                    return "" + (int) getChar(name);
                case FieldInfo.NSM_UINT16:
                    return "" + (int) getShort(name);
                case FieldInfo.NSM_UINT32:
                    return "" + getInt(name);
                case FieldInfo.NSM_UINT64:
                    return "" + getLong(name);
                case FieldInfo.NSM_FLOAT:
                    return "" + getFloat(name);
                case FieldInfo.NSM_DOUBLE:
                    return "" + getDouble(name);
                default:
                    break;
            }
        }
        return "";
    }

    public void setValueText(String name, String value) {
        if (hasField(name)) {
            switch (getProperty(name).getType()) {
                case FieldInfo.BOOL:
                    setBool(name, value == "true" || value == "t");
                    break;
                case FieldInfo.NSM_CHAR:
                case FieldInfo.NSM_BYTE8:
                case FieldInfo.CHAR:
                    setChar(name, (char) Integer.parseInt(value));
                    break;
                case FieldInfo.NSM_INT16:
                case FieldInfo.NSM_UINT16:
                case FieldInfo.SHORT:
                    setShort(name, (short) Integer.parseInt(value));
                    break;
                case FieldInfo.NSM_INT32:
                case FieldInfo.NSM_UINT32:
                case FieldInfo.INT:
                    setInt(name, (int) Integer.parseInt(value));
                    break;
                case FieldInfo.NSM_INT64:
                case FieldInfo.NSM_UINT64:
                case FieldInfo.LONG:
                    setLong(name, (long) Integer.parseInt(value));
                    break;
                case FieldInfo.NSM_FLOAT:
                case FieldInfo.FLOAT:
                    setFloat(name, (float) Double.parseDouble(value));
                    break;
                case FieldInfo.NSM_DOUBLE:
                case FieldInfo.DOUBLE:
                    setDouble(name, Double.parseDouble(value));
                    break;
                case FieldInfo.TEXT:
                    setText(name, value);
                    break;
                case FieldInfo.ENUM:
                    setEnum(name, value);
                    break;
                default:
                    break;
            }
        }
    }

    public void setText(String name, String value) {
        addText(name, value);
    }

    public void setEnum(String name, String value) {
        addEnum(name, value);
    }

    public boolean isConfig() {
        return m_isconfig;
    }

    public void reset() {
        m_index = 0;
        m_id = 0;
        m_name = "";
        m_name_v = new ArrayList<String>();
        m_pro_m = new HashMap<String, FieldInfo.Property>();
        m_enum_m_m = new HashMap<String, HashMap<String, Integer>>();
    }

    public boolean getBool(String name) {
        Object value = getValue(name);
        if (value == null) {
            return false;
        }
        return (Boolean) value;
    }

    public char getChar(String name) {
        Object value = getValue(name);
        if (value == null) {
            return 0;
        }
        return (Character) value;
    }

    public short getShort(String name) {
        Object value = getValue(name);
        if (value == null) {
            return 0;
        }
        return (Short) value;
    }

    public int getInt(String name) {
        Object value = getValue(name);
        if (value == null) {
            return 0;
        }
        return (Integer) value;
    }

    public long getLong(String name) {
        Object value = getValue(name);
        if (value == null) {
            return 0;
        }
        return (Long) value;
    }

    public float getFloat(String name) {
        Object value = getValue(name);
        if (value == null) {
            return 0;
        }
        return (Float) value;
    }

    public double getDouble(String name) {
        Object value = getValue(name);
        if (value == null) {
            return 0;
        }
        return (Double) value;
    }

    public void addBool(String name, boolean value) {
        addValue(name, FieldInfo.BOOL, value);
    }

    public void addChar(String name, int value) {
        addValue(name, FieldInfo.CHAR, value);
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

    public void addDouble(String name, double value) {
        addValue(name, FieldInfo.DOUBLE, value);
    }

    public void addText(String name, String value) {
        addValue(name, FieldInfo.TEXT, value);
    }

    public void addEnum(String name, String value) {
        addEnum(name, value, new HashMap<String, Integer>());
    }

    public void addEnum(String name, String value,
            HashMap<String, Integer> enum_m) {
        addValue(name, FieldInfo.ENUM, value);
        if (!m_enum_m_m.containsKey(name)) {
            m_enum_m_m.put(name, enum_m);
        }
    }

    public void setBool(String name, boolean value) {
        setValue(name, FieldInfo.BOOL, value);
    }

    public void setShort(String name, short value) {
        setValue(name, FieldInfo.SHORT, value);
    }

    public void setChar(String name, char value) {
        setValue(name, FieldInfo.CHAR, value);
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

    public void setDouble(String name, double value) {
        setValue(name, FieldInfo.DOUBLE, value);
    }

    public void print() {
        for (String name : getFieldNames()) {
            System.out.print(name + " : ");
            int type = getProperty(name).getType();
            switch (type) {
                case FieldInfo.BOOL:
                    System.out.print(getBool(name));
                    break;
                case FieldInfo.CHAR:
                    System.out.print(getChar(name));
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
                case FieldInfo.DOUBLE:
                    System.out.print(getDouble(name));
                    break;
                case FieldInfo.TEXT:
                    System.out.print(getText(name));
                    break;
                case FieldInfo.OBJECT: {
                    int nobj = getNObjects(name);
                    if (nobj > 0) {
                        DBObject obj = getObject(name);
                        System.out.println(obj.getTable() + " (" + obj.getRevision() + ")");
                        System.out.println("-------------------------");
                        for (int i = 0; i < nobj; i++) {
                            System.out.println("index : " + i);
                            getObject(name, i).print();
                            System.out.println("-------------------------");
                        }
                    }
                    break;
                }
                case FieldInfo.ENUM:
                    System.out.print(getEnum(name));
                    break;
                case FieldInfo.NSM_CHAR:
                    System.out.print(getChar(name));
                    break;
                case FieldInfo.NSM_INT16:
                    System.out.print(getShort(name));
                    break;
                case FieldInfo.NSM_INT32:
                    System.out.print(getInt(name));
                    break;
                case FieldInfo.NSM_INT64:
                    System.out.print(getLong(name));
                    break;
                case FieldInfo.NSM_BYTE8:
                    System.out.print(getChar(name));
                    break;
                case FieldInfo.NSM_UINT16:
                    System.out.print(getShort(name));
                    break;
                case FieldInfo.NSM_UINT32:
                    System.out.print(getInt(name));
                    break;
                case FieldInfo.NSM_UINT64:
                    System.out.print(getLong(name));
                    break;
                case FieldInfo.NSM_FLOAT:
                    System.out.print(getFloat(name));
                    break;
                case FieldInfo.NSM_DOUBLE:
                    System.out.print(getDouble(name));
                    break;
                case FieldInfo.NSM_OBJECT: {
                    int nobj = getNObjects(name);
                    System.out.println();
                    System.out.println("-------------------------");
                    for (int i = 0; i < nobj; i++) {
                        getObject(name, i).print();
                        System.out.println("-------------------------");
                    }
                    break;
                }
            }
            if (!hasObject(name)) {
                System.out.println();
            }
        }
    }

    abstract public void setObject(String name, int index, DBObject obj);

    abstract public DBObject getObject(String name, int i);

    public DBObject getObject(String name) {
        return getObject(name, 0);
    }

    abstract public int getNObjects(String name);

    abstract public void addValue(String name, int type, Object value);

    abstract public void setValue(String name, int type, Object value);

    abstract public Object getValue(String name);

    abstract public String getText(String name);

    abstract public String getEnum(String name);

}
