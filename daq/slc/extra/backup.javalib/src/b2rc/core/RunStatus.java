package b2rc.core;

import java.util.ArrayList;

import b2daq.core.DataObject;
import b2daq.core.RCNode;

public class RunStatus extends DataObject {

	private ArrayList<RCNode> _node_v = new ArrayList<RCNode>();
	private RunConfig _config;

	public RunStatus(String data_name, int revision) {
		super(data_name, "RunStatus");
		setRevision(revision);
		addInt("exp_number", 0);
		addInt("cold_number", 0);
		addInt("hot_number", 0);
		addInt("start_time", 0);
		addInt("end_time", -1);
		addInt("run_config", 0);
		addText("run_type", "", 64);
		addText("operators", "", 64);
		addText("comment", "", 512);
	}

	public RunStatus() {
		this("", -0);
	}

	public int getExpNumber() {
		return getInt("exp_number");
	}

	public int getColdNumber() {
		return getInt("cold_number");
	}

	public int getHotNumber() {
		return getInt("hot_number");
	}

	public int getStartTime() {
		return getInt("start_time");
	}

	public int getEndTime() {
		return getInt("end_time");
	}

	public int getRunConfig() {
		return getInt("run_config");
	}

	public String getRunType() {
		return getText("run_type");
	}

	public String getOperators() {
		return getText("operators");
	}

	public String getComment() {
		return getText("comment");
	}

	public void setExpNumber(int exp_number) {
		setInt("exp_number", exp_number);
	}

	public void setColdNumber(int run_number) {
		setInt("cold_number", run_number);
	}

	public void setHotNumber(int run_number) {
		setInt("hot_number", run_number);
	}

	public void setStartTime(int start_time) {
		setInt("start_time", start_time);
	}

	public void setEndTime(int end_time) {
		setInt("end_time", end_time);
	}

	public void setRunConfig(int run_config) {
		setInt("run_config", run_config);
	}

	public void setRunType(String run_type) {
		setText("run_type", run_type);
	}

	public void setOperators(String operators) {
		setText("operators", operators);
	}

	public void setComment(String comment) {
		setText("comment", comment);
	}

	public int incrementExpNumber() {
		int exp_number = getExpNumber();
		exp_number++;
		setExpNumber(exp_number);
		return exp_number;
	}

	public int incrementColdNumber() {
		int run_number = getColdNumber();
		run_number++;
		setColdNumber(run_number);
		return run_number;
	}

	public int incrementHotNumber() {
		int run_number = getHotNumber();
		run_number++;
		setHotNumber(run_number);
		return run_number;
	}

	public void add(RCNode node_in) {
		RCNode node = null;
		for (RCNode nodec : _node_v) {
			if (node_in == nodec) {
				node = node_in;
			}
		}
		if (node == node_in) {
			setInt(node.getName() + "_state", node.getState().getId());
			setInt(node.getName() + "_connection", node.getConnection().getId());
		} else {
			node = node_in;
			addInt(node.getName() + "_state", node.getState().getId());
			addInt(node.getName() + "_connection", node.getConnection().getId());
			_node_v.add(node);
		}
	}

	public void update() {
		for (RCNode node : _node_v) {
			setInt(node.getName() + "_state", node.getState().getId());
			setInt(node.getName() + "_connection", node.getConnection().getId());
		}
		setInt("run_config", _config.getConfigNumber());

	}

	public void setConfig(RunConfig config) {
		_config = config;
	}

}
