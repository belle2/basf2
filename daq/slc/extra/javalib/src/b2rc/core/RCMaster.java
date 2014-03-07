package b2rc.core;

import java.util.ArrayList;
import java.util.HashMap;

import b2daq.core.DataObject;
import b2daq.core.RCConnection;
import b2daq.core.RCNode;
import b2daq.core.RCState;
import b2daq.java.io.ConfigFile;

public class RCMaster {

	private RCNode _master_node;
	private RunStatus _status;
	private RunConfig _config;
	private DataObject _data;
	private ArrayList<RCNode> _node_v = new ArrayList<RCNode>();
	private HashMap<Integer, RCNode> _node_id_m = new HashMap<Integer, RCNode>();
	private HashMap<String, RCNode> _node_name_m = new HashMap<String, RCNode>();
	private ConfigFile _config_file = new ConfigFile();
	private ArrayList<ConfigFile> _config_file_v = new ArrayList<ConfigFile>();
	// private RCDatabaseManager _dbmanager;

	public RCMaster(RCNode node, RunConfig config, RunStatus status) {
		_master_node = node;
		_config = config;
		_status = status;
	}

	public ArrayList<RCNode> getNSMNodes() {
		return _node_v;
	}

	public DataObject getData() {
		return _data;
	}

	public void setData(DataObject data) {
		_data = data;
		for (String name : data.getParamNames()) {
			HashMap<String, DataObject.ParamInfo> param_m = data.getParams();
			if (param_m.get(name).type == DataObject.OBJECT) {
				RCNode node = new RCNode(name, data.getObject(name));
				addNode(node);
				_status.add(node);
			} else if (param_m.get(name).type == DataObject.INT) {
				RCNode node = new RCNode(name, new DataObject());
				addNode(node);
				_status.add(node);
				_config.addInt(name, data.getInt(name));
			}
		}
	}

	public RCNode getNodeByID(int id) {
		return (_node_id_m.containsKey(id)) ? _node_id_m.get(id) : null;
	}

	public RCNode getNodeByName(String name) {
		return (_node_name_m.containsKey(name)) ? _node_name_m.get(name) : null;
	}

	public void addNode(RCNode node) {
		if (node != null) {
			_node_v.add(node);
			if (!_node_name_m.containsKey(node.getName()))
				_node_name_m.put(node.getName(), node);
		}
	}

	public void load() {
		for (RCNode node : getNSMNodes()) {
			node.setUsed(_config.getBool(node.getName()+"_used"));
			node.getState().copy(_status.getInt(node.getName()+"_state"));
			node.getConnection().copy(_status.getInt(node.getName()+"_connection"));
			if (node.getConnection().equals(RCConnection.OFFLINE)) {
				node.setState(RCState.UNKNOWN);
			}
		}
	}
	
	public void addNode(int id, RCNode node) {
		if (id >= 0 && node != null) {
			node.setNodeID(id);
			_node_id_m.put(id, node);
			if (!_node_name_m.containsKey(node.getName()))
				_node_name_m.put(node.getName(), node);
		}
	}

	public RCNode getNode() {
		return _master_node;
	}

	public RunStatus getStatus() {
		return _status;
	}

	public RunConfig getConfig() {
		return _config;
	}

	public ConfigFile getConfigFile() {
		return _config_file;
	}

	public void setConfigFile(ConfigFile config_file) {
		_config_file = config_file;
	}

	public ArrayList<ConfigFile> getConfigFiles() {
		return _config_file_v;
	}

	public void setConfigFiles(ArrayList<ConfigFile> config_file_v) {
		_config_file_v = config_file_v;
	}

}