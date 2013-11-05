package b2rc.db;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.HashMap;

import b2rc.core.COPPERNode;
import b2rc.core.RONode;
import b2rc.core.FEEModule;
import b2rc.core.FTSW;
import b2rc.core.HSLB;
import b2rc.core.RCHost;
import b2rc.core.RCNodeSystem;
import b2rc.core.TTDNode;

public class RCDBManager {

	private static RCDBManager __dbman = null;
	private Connection _conn;
	private Statement _statement;
	private RCNodeSystem _system;
	private String _hostname;
	private String _database;
	private String _username;
	private String _password;
	private int _port;

	static public RCDBManager open(String hostname, String database,
			String username, String password, int port) throws Exception {
		return __dbman = new RCDBManager(hostname, database, username,
				password, port);
	}

	static public RCDBManager get() throws Exception {
		return __dbman;
	}

	public RCDBManager(String hostname, String database, String username,
			String password, int port) throws Exception {
		_hostname = hostname;
		_database = database;
		_username = username;
		_password = password;
		_port = port;
		connect();
	}

	public void connect() throws Exception { 
		final String driver = "org.gjt.mm.mysql.Driver";
		Class.forName(driver);
		String url = "jdbc:mysql://" + _hostname + ":" + _port + "/" + _database;
		_conn = DriverManager.getConnection(url, _username, _password);
		_statement = _conn.createStatement();
	}

	public RCDBManager(String hostname, String database, String username,
			String password) throws Exception {
		this(hostname, database, username, password, 0);
	}

	public synchronized ResultSet executeQuery(String sql) {
		try {
			return _statement.executeQuery(sql);
		} catch (SQLException e) {
			e.printStackTrace();
			return null;
		}
	}

	public synchronized boolean execute(String sql) {
		try {
			return _statement.execute(sql);
		} catch (SQLException e) {
			e.printStackTrace();
			return false;
		}
	}

	public synchronized void close() {
		try {
			_statement.close();
			_conn.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public synchronized ResultSet readVersionControlTable(int version) {
		return executeQuery("select * from version_control where version = "
				+ version + ";");
	}

	public synchronized void readTables(int version) throws SQLException {
		_system.setVersion(version);
		ResultSet results = readVersionControlTable(version);
		while ( !results.isClosed() && results.next() ) {
			_system.setRunType(results.getString("run_type"));
			_system.setDescription(results.getString("description"));
			HashMap<String, Integer> ver_m = new HashMap<String, Integer>();
			ver_m.put("hosts_ver", results.getInt("hosts_ver"));
			ver_m.put("copper_node_ver", results.getInt("copper_node_ver"));
			ver_m.put("hslb_ver", results.getInt("hslb_ver"));
			ver_m.put("ttd_node_ver", results.getInt("ttd_node_ver"));
			ver_m.put("ftsw_ver", results.getInt("ftsw_ver"));
			ver_m.put("ro_node_ver", results.getInt("ro_node_ver"));
			for (String label : _system.getModuleLists().keySet()) {
				ver_m.put(label + "_ver", results.getInt(label + "_ver"));
			}
			results.close();
			readHostTable(ver_m.get("hosts_ver"));
			readCOPPERNodeTable(ver_m.get("copper_node_ver"));
			readHSLBTable(ver_m.get("hslb_ver"));
			readRONodeTable(ver_m.get("ro_node_ver"));
			for (String label : _system.getModuleLists().keySet()) {
				readFEEModuleTable(label, _system.getModuleLists().get(label), ver_m.get(label + "_ver"));
			}
			try {
				readTTDNodeTable(ver_m.get("ttd_node_ver"));
				readFTSWTable(ver_m.get("ftsw_ver"));
			} catch (Exception e) {
				System.err.println("Error on ttd_node_conf or ftsw_conf");
			}
		}
	}

	public synchronized void readFTSWTable(int version) throws SQLException {
		if (_system == null) return;
		ResultSet results = executeQuery("select * from ftsw_conf where version = "
				+ version + ";");
		if (results == null) return;
		ArrayList<FTSW> ftsw_v = _system.getFTSWs();
		while ( !results.isClosed() && results.next() ) {
			int id = results.getInt("id");
			if (id < 0 || id >= (int) ftsw_v.size())
				continue;
			FTSW ftsw = ftsw_v.get(id);
			ftsw.setUsed(results.getBoolean("used"));
			ftsw.setProductID(results.getInt("product_id"));
			ftsw.setLocation(results.getString("location"));
			ftsw.setTriggerMode(results.getInt("trigger_mode"));
			ftsw.setDummyRate(results.getInt("dummy_rate"));
			ftsw.setTriggerLimit(results.getInt("trigger_limit"));
		}
	}

	public synchronized void readHostTable(int version) throws SQLException {
		ResultSet results = executeQuery("select * from host_conf where version = "
				+ version + ";");
		if (_system == null || results == null)	return;
		ArrayList<RCHost> host_v = _system.getHosts();
		while (results.next()) {
			int id = results.getInt("id");
			RCHost host = host_v.get(id);
			host.setName(results.getString("hostname"));
			host.setType(results.getString("type"));
			host.setProductID(results.getInt("product_id"));
			host.setLocation(results.getString("location"));
		}
	}

	public synchronized void readCOPPERNodeTable(int version) throws SQLException {
		//if (_system == null) return;
		ResultSet results = executeQuery("select * from copper_node_conf where version = " + version + ";");
		if (results == null) return;
		ArrayList<RCHost> host_v = _system.getHosts();
		ArrayList<COPPERNode> node_v = _system.getCOPPERNodes();
		ArrayList<HSLB> hslb_v = _system.getHSLBs();
		while ( results.next() ) {
			int id = results.getInt("id");
			COPPERNode node = node_v.get(id);
			node.setName(results.getString("name"));
			node.setUsed(results.getBoolean("used"));
			int host_id = results.getInt("host_id");
			if (host_id >= 0 && host_id < host_v.size()) {
				node.setHost(host_v.get(host_id));
			}
			for (int slot = 0; slot < 4; slot++) {
				int hslb_id = results.getInt("hslb_id_" + slot);
				if (hslb_id >= 0) {
					node.setHSLB(slot, hslb_v.get(hslb_id));
				} else {
					node.setHSLB(slot, null);
				}
			}
			node.getSender().setScript(results.getString("script"));
			node.getSender().setPort(results.getShort("port"));
			node.getSender().setEventSize(results.getInt("event_size"));
		}
	}

	public synchronized void readHSLBTable(int version) throws SQLException {
		ResultSet results = executeQuery("select * from hslb_conf where version = "
				+ version + ";");
		if ( results == null || _system == null ) return; 
		ArrayList<HSLB> hslb_v = _system.getHSLBs();
		while ( !results.isClosed() && results.next() ) {
			int id = results.getInt("id");
			if (id < 0 || id >= (int) hslb_v.size()) continue;
			HSLB hslb = hslb_v.get(id);
			hslb.setUsed(results.getBoolean("used"));
			hslb.setProductID(results.getInt("product_id"));
			hslb.setLocation(results.getString("location"));
			hslb.setFirmware(results.getString("firmware"));
			hslb.setTriggerMode(results.getInt("trigger_mode"));
		}
	}

	public synchronized void readTTDNodeTable(int version) throws SQLException {
		ResultSet results = executeQuery("select * from ttd_node_conf where version = " + version + ";");
		if ( results == null ) return;
		ArrayList<RCHost> host_v = _system.getHosts();
		ArrayList<TTDNode> node_v = _system.getTTDNodes();
		ArrayList<FTSW> ftsw_v = _system.getFTSWs();
		while (results.next()) {
			int id = results.getInt("id");
			if (id < 0 || id >= (int) node_v.size()) continue;
			TTDNode node = node_v.get(id);
			node.setName(results.getString("name"));
			node.setUsed(results.getBoolean("used"));
			int host_id = results.getInt("host_id");
			if (host_id >= 0 && host_id < (int) host_v.size())
				node.setHost(host_v.get(host_id));
			node.clearFTSWs();
			for (int slot = 0; slot < 10; slot++) {
				int ftsw_id = results.getInt("ftsw_id_" + slot);
				if (ftsw_id >= 0) {
					node.addFTSW(ftsw_v.get(ftsw_id));
				}
			}
		}
	}

	public synchronized void readRONodeTable(int version) throws SQLException {
		ResultSet results = executeQuery("select * from ro_node_conf where version = " + version + ";");
		if ( results == null ) return;
		ArrayList<RONode> node_v = _system.getReceiverNodes();
		while ( !results.isClosed() && results.next() ) {
			int id = results.getInt("id");
			if (id < 0 || id >= (int) node_v.size()) continue;
			RONode node = node_v.get(id);
			node.setName(results.getString("name"));
			node.setUsed(results.getBoolean("used"));
			node.setScript(results.getString("script"));
			node.clearSenders();
			for (int slot = 0; slot < 10; slot++) {
				String sender = results.getString("sender_" + slot);
				node.addSender(sender);
			}
		}
	}

	public synchronized void readFEEModuleTable(String module_type,
			ArrayList<FEEModule> module_v, int version) throws SQLException {
		ResultSet results = executeQuery("select * from " + module_type
				+ "_conf where version = " + version + ";");
		if ( results == null ) return;
		while ( !results.isClosed() && results.next() ) {
			int id = results.getInt("id");
			if (id < 0 || id >= module_v.size()) continue;
			FEEModule module = module_v.get(id);
			for (int slot = 0; slot < module.getRegisters().size(); slot++) {
				FEEModule.Register reg = module.getRegister(slot);
				for (int i = 0; i < reg.length(); i++) {
					int value = results.getInt(reg.getName() + "_" + i);
					reg.setValue(i, value);
				}
			}
		}
	}

	public void setNodeSystem(RCNodeSystem system) {
		_system = system;
	}

	public RCNodeSystem getNodeSystem() {
		return _system;
	}

}
