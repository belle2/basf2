package b2daq.database;

import java.util.ArrayList;

public class DBObjectList {

	private ArrayList<DBObject> m_obj_v = new ArrayList<DBObject>();
	private int m_configid = 0;
	private String m_configname = "";
	private String m_node = "";
	private String m_table = "";

	public void clear() {
		m_obj_v.clear();
	}

	public ArrayList<DBObject> get() {
		return m_obj_v;
	}
	
	public DBObject get(int i) {
		return m_obj_v.get(i);
	}

	public void add(DBObject obj) {
		m_obj_v.add(obj);
	}

	public int size() {
		return m_obj_v.size();
	}

	public int getConfigId() {
		return m_configid;
	}

	public void setConfigId(int id) {
		m_configid = id;
	}

	public String getConfigName() {
		return m_configname;
	}

	public void setConfigName(String name) {
		m_configname = name;
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

}
