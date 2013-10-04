package b2rc.core;

import java.util.ArrayList;
import java.util.HashMap;

public class RCNodeSystem extends SCObject {

	static final public int MODE_LOCAL = 1;
	static final public int MODE_GLOBAL = 2;

	static final public String TAG = "system";

	private int _version = -1;
	private int _operation_mode = MODE_LOCAL;
	private String _run_type = "";
	private String _description = "";
	private ArrayList<RCHost> _host_v = new ArrayList<RCHost>();
	private ArrayList<RCNode> _node_v = new ArrayList<RCNode>();
	private ArrayList<RCNodeGroup> _group_v = new ArrayList<RCNodeGroup>();
	private ArrayList<TTDNode> _ttd_v = new ArrayList<TTDNode>();
	private ArrayList<FTSW> _ftsw_v = new ArrayList<FTSW>();
	private ArrayList<RONode> _recv_v = new ArrayList<RONode>();
	private ArrayList<COPPERNode> _copper_v = new ArrayList<COPPERNode>();
	private ArrayList<HSLB> _hslb_v = new ArrayList<HSLB>();
	private ArrayList<FEEModule> _module_v = new ArrayList<FEEModule>();
	private HashMap<String, ArrayList<FEEModule>> _module_v_m = new HashMap<String, ArrayList<FEEModule>>();
	private String _operators = "";
	private RCNode _rc_node = new RCNode();
	private int _exp_no;
	private int _run_no;
	private int _start_time;
	private int _end_time;
	private int _trigger_mode = 0;
	private int _dummy_rate = -1;
	private int _trigger_limit = 0;

	public RCNodeSystem() {
	}

	public RCNodeSystem(int id) {
		super(id);
	}

	public RCNodeSystem(int version, int id) {
		super(version, id);
	}

	@Override
	public String getTag() {
		return TAG;
	}

	public void setVersion(int version) {
		_version = version;
	}

	public void setVersion(String label, int version) {
		if (label == "host") {
			for (RCHost host : _host_v) {
				host.setVersion(version);
			}
		} else if (label == "copper") {
			for (COPPERNode copper : _copper_v) {
				copper.setVersion(version);
			}
		} else if (label == "hslb") {
			for (HSLB hslb : _hslb_v) {
				hslb.setVersion(version);
			}
		} else if (label == "ttd") {
			for (TTDNode ttd : _ttd_v) {
				ttd.setVersion(version);
			}
		} else if (label == "ftsw") {
			for (FTSW ftsw : _ftsw_v) {
				ftsw.setVersion(version);
			}
		} else if (label == "receiver") {
			for (RONode recv : _recv_v) {
				recv.setVersion(version);
			}
		} else {
			for (FEEModule module : _module_v_m.get(label)) {
				module.setVersion(version);
			}
		}
	}

	public void setOperationMode(int operation_mode) {
		_operation_mode = operation_mode;
	}

	public void setRunType(String run_type) {
		_run_type = run_type;
	}

	public void setDescription(String description) {
		_description = description;
	}

	public int getVersion() {
		return _version;
	}

	public int getOperationMode() {
		return _operation_mode;
	}

	public String getRunType() {
		return _run_type;
	}

	public String getDescription() {
		return _description;
	}

	public ArrayList<RCHost> getHosts() {
		return _host_v;
	}

	public ArrayList<RCNode> getNodes() {
		return _node_v;
	}

	public ArrayList<RCNodeGroup> getNodeGroups() {
		return _group_v;
	}

	public ArrayList<RONode> getReceiverNodes() {
		return _recv_v;
	}

	public ArrayList<TTDNode> getTTDNodes() {
		return _ttd_v;
	}

	public ArrayList<FTSW> getFTSWs() {
		return _ftsw_v;
	}

	public ArrayList<COPPERNode> getCOPPERNodes() {
		return _copper_v;
	}

	public ArrayList<HSLB> getHSLBs() {
		return _hslb_v;
	}

	public ArrayList<FEEModule> getModules() {
		return _module_v;
	}

	public ArrayList<FEEModule> getModules(String module_class) {
		return _module_v_m.get(module_class);
	}

	public HashMap<String, ArrayList<FEEModule>> getModuleLists() {
		return _module_v_m;
	}

	public void addHost(RCHost host) {
		_host_v.add(host);
	}

	public void addNode(RCNode node) {
		_node_v.add(node);
	}

	public void addNodeGroup(RCNodeGroup group) {
		_group_v.add(group);
		ArrayList<RCNode> node_v = group.getNodes();
		for (RCNode node : node_v) {
			_node_v.add(node);
		}
	}

	public void addReceiverNode(RONode node) {
		_recv_v.add(node);
	}

	public void addCOPPERNode(COPPERNode node) {
		_copper_v.add(node);
	}

	public void addTTDNode(TTDNode node) {
		_ttd_v.add(node);
	}

	public void addFTSW(FTSW ftsw) {
		_ftsw_v.add(ftsw);
	}

	public void addHSLB(HSLB hslb) {
		_hslb_v.add(hslb);
	}

	public void addModule(FEEModule module) {
		_module_v.add(module);
	}

	public void addModules(String module_class, ArrayList<FEEModule> module_v) {
		_module_v_m.put(module_class, module_v);
	}

	public boolean hasModuleClass(String module_class) {
		return _module_v_m.containsKey(module_class);
	}

	public String getSQLFields() {
		StringBuffer ss = new StringBuffer();
		ss.append("version int, date timestamp, ");
		ss.append("operation_mode int, run_type text, description text, ");
		ss.append("hosts_ver int, copper_node_ver int, hslb_ver int, ttd_node_ver int, ");
		ss.append("ftsw_ver int, receiver_node_ver int");
		for (String label : _module_v_m.keySet()) {
			ss.append(", " + label + "_ver int");
		}
		return ss.toString();
	}

	public String getSQLLabels() {
		StringBuffer ss = new StringBuffer();
		ss.append("version, date, ");
		ss.append("operation_mode, run_type, description, ");
		ss.append("hosts_ver, copper_node_ver, hslb_ver, ttd_node_ver, ");
		ss.append("ftsw_ver, receiver_node_ver");
		for (String label : _module_v_m.keySet()) {
			ss.append(", " + label + "_ver");
		}
		return ss.toString();
	}

	public String getSQLValues() {
		StringBuffer ss = new StringBuffer();
		ss.append(_version + ", current_timestamp, " + _operation_mode + ", '");
		ss.append(_run_type + "', '" + _description.replace("'", "^!^") + "', ");
		ss.append(((_host_v.size() > 0) ? _host_v.get(0).getVersion() : 0)
				+ ", ");
		ss.append(((_copper_v.size() > 0) ? _copper_v.get(0).getVersion() : 0)
				+ ", ");
		ss.append(((_hslb_v.size() > 0) ? _hslb_v.get(0).getVersion() : 0)
				+ ", ");
		ss.append(((_ttd_v.size() > 0) ? _ttd_v.get(0).getVersion() : 0) + ", ");
		ss.append(((_ftsw_v.size() > 0) ? _ftsw_v.get(0).getVersion() : 0)
				+ ", ");
		ss.append(((_recv_v.size() > 0) ? _recv_v.get(0).getVersion() : 0));
		for (String label : _module_v_m.keySet()) {
			ArrayList<FEEModule> module_v = _module_v_m.get(label);
			ss.append(", "
					+ ((module_v.size() > 0) ? module_v.get(0).getVersion() : 0));
		}

		return ss.toString();
	}

	public void setOperators(String operators) {
		_operators = operators;
	}

	public String  getOperators() {
		return _operators;
	}

	public RCNode getRunControlNode() {
		return _rc_node;
	}

	public void setRunControlNode(RCNode rc_node) {
		_rc_node = rc_node;
	}

	public void setExpNumber(int no) {
		_exp_no = no;
	}

	public void setRunNumber(int no) {
		_run_no = no;
	}

	public void setStartTime(int time) {
		_start_time = time;
	}

	public void setEndTime(int time) {
		_end_time = time;
	}

	public int getExpNumber() {
		return _exp_no;
	}

	public int getRunNumber() {
		return _run_no;
	}

	public int getStartTime() {
		return _start_time;
	}

	public int getEndTime() {
		return _end_time;
	}

	public int getTriggerMode() {
		return _trigger_mode;
	}
	
	public void setTriggerMode(int mode) {
		_trigger_mode = mode;
	}

	public void setDummyRate(int rate) {
		_dummy_rate = rate;
	}

	public void setTriggerLimit(int limit) {
		_trigger_limit = limit;
	}

	public int getDummyRate() {
		return _dummy_rate;
	}

	public int getTriggerLimit() {
		return _trigger_limit;
	}

}
