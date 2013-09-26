package b2rc.core;

public class RCNode extends SCObject {

	protected RCHost _host = null;
	protected boolean _used = true;
	protected RCState _state = new RCState(RCState.UNKNOWN);
	protected RCConnection _connection = new RCConnection(RCConnection.OFFLINE);
	protected String _name = "";
	private String _type = "";
	protected int _node_id = -1;
	protected int _index = -1;
	
	public RCNode() {
		_state.copy(RCCommand.UNKNOWN);
	}

	public RCNode(String name) {
		super(-1);
		_name = name;
	}

	public String getName() {
		return _name;
	}

	public String getType() {
		return _type;
	}

	public boolean isUsed() {
		return _used;
	}

	public RCHost getHost() {
		return _host;
	}

	public RCState getState() {
		return _state;
	}

	public int getNodeID() {
		return _node_id;
	}

	public int getIndex() {
		return _index;
	}

	public void setName(String name) {
		_name = name;
	}

	public void setUsed(boolean used) {
		_used = used;
	}

	public void setHost(RCHost host) {
		_host = host;
	}

	public void setState(RCState state) {
		_state.copy(state);
	}

	public void setState(int id) {
		_state.copy(id);
	}

	public void setNodeID(int id) {
		_node_id = id;
	}

	public void setIndex(int index) {
		_index = index;
	}

	@Override
	public String getSQLFields() {
		return super.getSQLFields() + ", name text, used boolean, host_id int";
	}

	@Override
	public String getSQLLabels() {
		return super.getSQLLabels() + ", name, used, host_id";

	}

	@Override
	public String getSQLValues() {
		return super.getSQLValues() + ", '" + _name + "', " + ((_used)?"true":"false") + ", " + _host.getId();
	}

	@Override
	public String getTag() {
		return "rc_node";
	}

	public void setConnection(int id) {
		_connection.copy(id);
	}

	public RCConnection getConnection() {
		return _connection;
	}

	public void setConnection(RCConnection connection) {
		_connection.copy(connection);
	}

}
