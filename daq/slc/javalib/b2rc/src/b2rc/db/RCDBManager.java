package b2rc.db;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.HashMap;

import b2rc.core.COPPERNode;
import b2rc.core.DataRecieverNode;
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

	static public RCDBManager open(String hostname, String database,
			String username, String password, int port) throws Exception {
		return __dbman = new RCDBManager(hostname, database, username,
				password, port);
	}

	static public RCDBManager get() throws Exception {
		if (__dbman == null) {
			String hostname = System.getenv("B2SC_DB_HOST");
			if ( hostname == null ) hostname = "localhost";
			String dbname = System.getenv("B2SC_DB_NAME");
			if ( dbname == null ) dbname = "b2daq";
			String username = System.getenv("B2SC_DB_USER");
			if ( username == null ) username = "slcdaq";
			String password = System.getenv("B2SC_DB_PASS");
			if ( password == null ) password = "slcdaq";
			String port_s = System.getenv("B2SC_DB_PORT");
			if ( port_s == null ) port_s = "33306";
			int port = 0;
			try {
				port = Integer.parseInt(port_s);
			} catch (Exception e) {
				port = 3306;
			}
			return open(hostname, dbname, username, password, port);
			//return open("localhost", "b2daq", "slcdaq", "slcdaq", 33306);
		}
		return __dbman;
	}

	public RCDBManager(String hostname, String database, String username,
			String password, int port) throws Exception {
		final String driver = "org.gjt.mm.mysql.Driver";
		Class.forName(driver);
		String url = "jdbc:mysql://" + hostname + ":" + port + "/" + database;
		_conn = DriverManager.getConnection(url, username, password);
		_statement = _conn.createStatement();
	}

	public RCDBManager(String hostname, String database, String username,
			String password) throws Exception {
		this(hostname, database, username, password, 0);
	}

	public synchronized ResultSet executeQuery(String sql) {
		try {
			//System.out.println(sql);
			return _statement.executeQuery(sql);
		} catch (SQLException e) {
			//e.printStackTrace();
			return null;
		}
	}

	public synchronized boolean execute(String sql) {
		try {
			return _statement.execute(sql);
		} catch (SQLException e) {
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
			ver_m.put("reciever_node_ver", results.getInt("reciever_node_ver"));
			for (String label : _system.getModuleLists().keySet()) {
				ver_m.put(label + "_ver", results.getInt(label + "_ver"));
			}
			results.close();
			readHostTable(ver_m.get("hosts_ver"));
			readCOPPERNodeTable(ver_m.get("copper_node_ver"));
			readHSLBTable(ver_m.get("hslb_ver"));
			readDataReceiverNodeTable(ver_m.get("reciever_node_ver"));
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
			ftsw.clearModules();
			for (int slot = 0; slot < 10; slot++) {
				String module_type = results.getString("module_type_" + slot);
				int module_id = results.getInt("module_id_" + slot);
				ArrayList<FEEModule> module_v = _system.getModules(module_type);
				if (module_v != null && module_id >= 0
						&& module_id < (int) module_v.size())
					ftsw.addFEEModule(module_v.get(module_id));
			}
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

	public synchronized void readDataReceiverNodeTable(int version) throws SQLException {
		ResultSet results = executeQuery("select * from receiver_node_conf where version = " + version + ";");
		if ( results == null ) return;
		ArrayList<DataRecieverNode> node_v = _system.getReceiverNodes();
		ArrayList<COPPERNode> copper_v = _system.getCOPPERNodes();
		while ( !results.isClosed() && results.next() ) {
			int id = results.getInt("id");
			if (id < 0 || id >= (int) node_v.size()) continue;
			DataRecieverNode node = node_v.get(id);
			node.setName(results.getString("name"));
			node.setUsed(results.getBoolean("used"));
			node.setScript(results.getString("script"));
			node.clearSenders();
			for (int slot = 0; slot < 10; slot++) {
				int sender_id = results.getInt("sender_id_" + slot);
				if (sender_id >= 0 && sender_id < (int) copper_v.size()) {
					COPPERNode copper = copper_v.get(sender_id);
					node.addSender(copper.getSender());
				}
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
