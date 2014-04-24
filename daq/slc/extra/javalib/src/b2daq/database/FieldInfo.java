package b2daq.database;

import java.util.HashMap;

public class FieldInfo {

	static public final int BOOL = 1;
	static public final int SHORT = 2;
	static public final int INT = 3;
	static public final int LONG = 4;
	static public final int FLOAT = 5;
	static public final int TEXT = 6;
	static public final int ENUM = 7;
	static public final int OBJECT = 8;
	static public final int TABLE = 9;
	
	static private HashMap<String, FieldInfo> g_info_m = new HashMap<String, FieldInfo>();
	
	static public FieldInfo getInfo(String name) {
		if (g_info_m.containsKey(name)) {
			return g_info_m.get(name);
		}
		return null;
	}
	
	static public void addInfo(String name, FieldInfo info) {
		if (!g_info_m.containsKey(name)) g_info_m.put(name, info);
		System.out.println(name + " : " + info.getValue());
	}

	static public void clearInfoList() {
		g_info_m = new HashMap<String, FieldInfo>();
	}
	
	private String m_name = "";
	private int m_type = TEXT;
	private String m_value = "";
	private String m_tablename = "";

	public FieldInfo() {}

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
			return "double precision";
		case TEXT:
			return "text";
		case TABLE:
			return "int";
		case ENUM:
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
