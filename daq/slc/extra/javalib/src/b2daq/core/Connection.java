package b2daq.core;

public class Connection extends Enum {

	final static public Connection ONLINE = new Connection(10000, "ONLINE"); 
	final static public Connection OFFLINE = new Connection(20000, "OFFLINE"); 
	
	public Connection() {
		
	}
	
	public Connection(int id, String label) {
		super(id, label);
	}

	public Connection(Connection connection) {
		copy(connection);
	}

	public void copy(int id) {
		  if ( id == ONLINE._id ) copy(ONLINE);
		  else if ( id == OFFLINE._id ) copy(OFFLINE);
	}

}
