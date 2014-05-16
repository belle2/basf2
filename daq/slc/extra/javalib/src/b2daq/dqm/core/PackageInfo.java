package b2daq.dqm.core;

import java.util.Date;

public class PackageInfo {

	private int _package_id = 0;
	private int _update_id = 0;
	private MonitorConnection _connection = new MonitorConnection();
	private MonitorState _state = new MonitorState();
	private Date _update_time = new Date();
	private Date _start_time = new Date();
	private String _process_name = "";
	private String _package_name = "";

	public PackageInfo() {}
	
	public PackageInfo(String package_name) {
		_package_name = package_name;
	}
	
	public PackageInfo(String package_name, long time) {
		_package_name = package_name;
		_start_time.setTime(time);
	}
	
	public void setPackageID(int package_id) {
		_package_id = package_id;
	}

	public int getPackageID() {
		return _package_id;
	}

	public void setUpdateID(int update_id) {
		_update_id = update_id;
	}

	public int getUpdateID() {
		return _update_id;
	}

	public void setConnection(int connection) {
		_connection.setValue(connection);
	}

	public MonitorConnection getConnection() {
		return _connection;
	}

	public void setState(int state) {
		_state.setValue(state);
	}

	public MonitorState getState() {
		return _state;
	}

	public void setUpdateTime(long update_time) {
		_update_time.setTime(update_time);
	}

	public Date getUpdateTime() {
		return _update_time;
	}

	public void setStartTime(long start_time) {
		_start_time.setTime(start_time);
	}

	public Date getStartTime() {
		return _start_time;
	}

	public void setProcessName(String process_name) {
		_process_name = process_name;
	}

	public String getProcessName() {
		return _process_name;
	}

	public void setPackageName(String package_name) {
		_package_name = package_name;
	}

	public String getPackageName() {
		return _package_name;
	}

	public void setConnection(MonitorConnection connection) {
		_connection.setValue(connection.getValue());
	}

	public void setState(MonitorState state) {
		_state.setValue(state.getValue());
	}

	public String toXML() {
		String str = "<package-info " +
		" package-id=\"" + getPackageID() + 
		"\" process-name=\"" + getProcessName() + 
		"\" package-name=\"" + getPackageName() + 
		"\" start-time=\"" + getStartTime().getTime() +
		"\" update-time=\"" + getUpdateTime().getTime() +
		"\" state=\"" + getState().getText() +
		"\"  state=\"" + getConnection().getText() +
		"\" />\n";
		return str;
	}
	
}
