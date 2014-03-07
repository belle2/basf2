package b2rc.java.ui2;

public class RunConfig {
	
	private String _runtype = "default";
	private String [] _node_daq_v;
	private String [] _node_monitor_v;
	private String _description = "";
	
	public RunConfig() {
	}

	public String getRunType() {
		return _runtype;
	}

	public void setRunType(String runtype) {
		_runtype = runtype;
	}

	public String [] getDAQNodes() {
		return _node_daq_v;
	}

	public void setDAQNodes(String [] node_daq_v) {
		_node_daq_v = node_daq_v;
	}

	public String [] getMontorNodes() {
		return _node_monitor_v;
	}

	public void setMonitorNodes(String [] node_monitor_v) {
		_node_monitor_v = node_monitor_v;
	}

	public String getDescription() {
		return _description;
	}

	public void setDescription(String description) {
		_description = description;
	}

}
