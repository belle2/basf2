package b2daq.database;

import java.util.HashMap;

public class FieldInfo {

    static public final int BOOL = 1;
    static public final int CHAR = 2;
    static public final int SHORT = 3;
    static public final int INT = 4;
    static public final int LONG = 5;
    static public final int FLOAT = 6;
    static public final int DOUBLE = 7;
    static public final int TEXT = 8;
    static public final int OBJECT = 9;
    static public final int NSM_CHAR = 10;
    static public final int NSM_INT16 = 11;
    static public final int NSM_INT32 = 12;
    static public final int NSM_INT64 = 13;
    static public final int NSM_BYTE8 = 14;
    static public final int NSM_UINT16 = 15;
    static public final int NSM_UINT32 = 16;
    static public final int NSM_UINT64 = 17;
    static public final int NSM_FLOAT = 18;
    static public final int NSM_DOUBLE = 19;
    static public final int NSM_OBJECT = 20;

    public static class Property {

        public Property() {
        }

        public Property(int type, int length, int offset) {
            m_type = type;
            m_length = length;
            m_offset = offset;
        }

        public Property(int type, int length) {
            this(type, length, 0);
        }

        public int getType() {
            return m_type;
        }

        public int getLength() {
            return m_length;
        }

        public int getTypeSize() {
            switch (m_type) {
                case BOOL:
                    return 1;
                case CHAR:
                    return 1;
                case SHORT:
                    return 2;
                case INT:
                    return 4;
                case LONG:
                    return 8;
                case FLOAT:
                    return 4;
                case DOUBLE:
                    return 8;
                case NSM_CHAR:
                    return 1;
                case NSM_INT16:
                    return 2;
                case NSM_INT32:
                    return 4;
                case NSM_INT64:
                    return 8;
                case NSM_BYTE8:
                    return 1;
                case NSM_UINT16:
                    return 2;
                case NSM_UINT32:
                    return 4;
                case NSM_UINT64:
                    return 8;
                case NSM_FLOAT:
                    return 4;
                case NSM_DOUBLE:
                    return 8;
                default:
                    break;
            }
            return 0;
        }

        public int getOffset() {
            return m_offset;
        }

        public void setType(int type) {
            m_type = type;
        }

        public void setLength(int length) {
            m_length = length;
        }

        private int m_type = 0;
        private int m_length = 0;
        private int m_offset = 0;

    };

    static private HashMap<String, FieldInfo> g_info_m = new HashMap<>();

    static public FieldInfo getInfo(String name) {
        if (g_info_m.containsKey(name)) {
            return g_info_m.get(name);
        }
        return null;
    }

    static public void addInfo(String name, FieldInfo info) {
        if (!g_info_m.containsKey(name)) {
            g_info_m.put(name, info);
        }
        System.out.println(name + " : " + info.getValue());
    }

    static public void clearInfoList() {
        g_info_m = new HashMap<>();
    }

    private String m_name = "";
    private int m_type = TEXT;
    private String m_value = "";
    private String m_tablename = "";

    public FieldInfo() {
    }

    public FieldInfo(String name, int type, String value, String tablename) {
        m_name = name;
        m_type = type;
        m_value = value;
        m_tablename = tablename;
    }

    public String getName() {
        return m_name;
    }

    public int getType() {
        return m_type;
    }

    public String getTypeAlias() {
        switch (m_type) {
            case BOOL:
                return "boolean";
            case SHORT:
                return "smallint";
            case INT:
                return "int";
            case LONG:
                return "bigint";
            case FLOAT:
                return "float";
            case DOUBLE:
                return "double precision";
            case TEXT:
                return "text";
            case OBJECT:
                return "int";
        }
        return "text";
    }

    public String getValue() {
        return m_value;
    }

    public String getTable() {
        return m_tablename;
    }

    public void setName(String name) {
        m_name = name;
    }

    public void setType(int type) {
        m_type = type;
    }

    public void setValue(String value) {
        m_value = value;
    }

    public void setTable(String tablename) {
        m_tablename = tablename;
    }
}
