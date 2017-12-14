package b2daq.nsm;

import b2daq.core.Connection;
import b2daq.runcontrol.core.RCCommand;

public class NSMNode {

    private String _name = "";
    private boolean _used = true;
    private boolean _excluded = false;
    private final NSMState _state = new NSMState();
    private final Connection _connection = new Connection(Connection.OFFLINE);
    private int _node_id = -1;
    private int _index = -1;
    private boolean _sync;

    public NSMNode() {
        _state.copy(RCCommand.UNKNOWN);
    }

    public NSMNode(String name) {
        _name = name;
    }

    public String getName() {
        return _name;
    }

    public boolean isUsed() {
        return _used;
    }

    public boolean isExcluded() {
        return _excluded;
    }

    public NSMState getState() {
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

    public void setExcluded(boolean excluded) {
        _excluded = excluded;
    }

    public void setState(NSMState state) {
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

    public Connection getConnection() {
        return _connection;
    }

    public void setConnection(Connection connection) {
        _connection.copy(connection);
    }

}
