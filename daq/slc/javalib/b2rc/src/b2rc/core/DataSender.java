package b2rc.core;

public class DataSender extends SCObject {

	public static String TAG = "data_sender";

	private String _script;
	private String _host;
	private short _port;
	private int _event_size;

	public String getScript() {
		return _script;
	}

	public String getHost() {
		return _host;
	}

	public short getPort() {
		return _port;
	}

	public int getEventSize() {
		return _event_size;
	}

	public void setScript(String script) {
		_script = script;
	}

	public void setHost(String host) {
		_host = host;
	}

	public void setPort(short port) {
		_port = port;
	}

	public void setEventSize(int size) {
		_event_size = size;
	}

	@Override
	public String getTag() {
		return TAG;
	}

	@Override
	public String getSQLFields() {
		return ", script text, port smallint, event_size int";
	}

	@Override
	public String getSQLLabels() {
		return ", script, port, event_size";
	}

	@Override
	public String getSQLValues() {
		return ", '" + _script + "', " + _port + ", "
				+ _event_size;
	}

}
