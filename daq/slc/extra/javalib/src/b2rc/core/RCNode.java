package b2rc.core;

public class RCNode {

	private DataObject _obj;
	private String _name = "";
	private boolean _used = true;
	private RCState _state = new RCState(RCState.UNKNOWN);;
	private RCConnection _connection = new RCConnection(RCConnection.OFFLINE);
	private int _node_id = -1;
	private int _index = -1;
	private boolean _sync;

	public RCNode() {
		_state.copy(RCCommand.UNKNOWN);
	}

	public RCNode(String name) {
		_name = name;
	}

	public RCNode(String name, DataObject object) {
		_name = name;
		_obj = object;
	}

	public DataObject getData() {
		return _obj;
	}

	public void setData(DataObject data) {
		_obj = data;
	}

	public String getName() {
		return _name;
	}

	public boolean isUsed() {
		return _used;
	}

	public RCState getState() {
		return _state;
	}

	public void setSynchronized(boolean sync) {
		_sync = sync;
	}

	public boolean isSynchronized() {
		return _sync;
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
