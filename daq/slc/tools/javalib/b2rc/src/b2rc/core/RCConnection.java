package b2rc.core;

public class RCConnection extends RCEnum {

	final static public RCConnection ONLINE = new RCConnection(10000, "ONLINE", "ONLINE"); 
	final static public RCConnection OFFLINE = new RCConnection(20000, "OFFLINE", "OFFLINE"); 
	
	public RCConnection() {
		
	}
	
	public RCConnection(int id, String label, String alias) {
		super(id, label, alias);
	}

	public RCConnection(RCConnection connection) {
		copy(connection);
	}

	public void copy(int id) {
		  if ( id == ONLINE._id ) copy(ONLINE);
		  else if ( id == OFFLINE._id ) copy(OFFLINE);
	}

}
