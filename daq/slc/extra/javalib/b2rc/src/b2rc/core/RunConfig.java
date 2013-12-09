package b2rc.core;

import java.util.ArrayList;
import java.util.HashMap;

public class RunConfig extends DataObject {
	
	private HashMap<String, DataObject> _data_m = new HashMap<String, DataObject>();
	private RCMaster _master = null;

	public RunConfig() {
		this("", 0);
	}

	public RunConfig(String data_name, int revision) {
		super(data_name, "RunConfig");
		setRevision(revision);
		addText("run_type", "TESTT:DEFAULT", 64);
		addText("description", "Defualt settings", 128);
	}

	public String getRunType() {
		return getText("run_type");
	}

	public String getDescription() {
		return getText("description");
	}

	public void setRunType(String run_type) {
		setText("run_type", run_type);
	}

	public void setDescription(String description) {
		setText("_description", description);
	}

	public void add(HashMap<String, DataObject> data_m,
					  ArrayList<RCNode> node_v) {
		for (String name : data_m.keySet()) {
			if (name.length() > 0) {
				DataObject data = data_m.get(name);
				String label = name;
				addInt(label, data.getConfigNumber());
				_data_m.put(label, data);
			}
		}
		for ( RCNode node : node_v ) {
			addBool(node.getName() + "_used", node.isUsed());
		}
	}

	public void update() {
		for (String name : _data_m.keySet()) {
			if (name.length() > 0) {
				DataObject data = _data_m.get(name);
				String label = name;
				setInt(label, data.getConfigNumber());
			}
		}
		if (_master != null) {
			for (RCNode node : _master.getNSMNodes()) {
				setBool(node.getName() + "_used", node.isUsed());
				if (hasValue(node.getName()) && node.getData() != null) {
					setInt(node.getName(), node.getData().getConfigNumber());
				}
			}
		}
	}

	public void setMaster(RCMaster master) {
		_master = master;
	}

}
