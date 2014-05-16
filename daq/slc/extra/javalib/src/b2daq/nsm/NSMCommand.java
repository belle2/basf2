package b2daq.nsm;

import b2daq.core.Enum;

public class NSMCommand extends Enum {

	final static public int DISABLED = 0;
	final static public int ENABLED = 1;
	final static public int SUGGESTED = 2;

	final static public NSMCommand OK = new NSMCommand(1, "OK");
	final static public NSMCommand ERROR = new NSMCommand(2, "ERROR");
	final static public NSMCommand NSMSET = new NSMCommand(401, "NSMSET");
	final static public NSMCommand NSMGET = new NSMCommand(402, "NSMGET");
	final static public NSMCommand DBSET = new NSMCommand(403, "DBSET");
	final static public NSMCommand DBGET = new NSMCommand(404, "DBGET");

	public NSMCommand() {
		copy(NSMCommand.UNKNOWN);
	}

	protected NSMCommand(int id, String label) {
		super(id, label);
	}

	public NSMCommand(NSMCommand command) {
		copy(command);
	}

	public boolean copy(String msg) throws Exception {
		if (msg.equals(UNKNOWN.getLabel())) {
			copy(UNKNOWN); return true;
		} else if (msg.equals(OK.getLabel())) {
			copy(OK); return true;
		} else if (msg.equals(ERROR.getLabel())) {
			copy(ERROR); return true;
		} else if (msg.equals(NSMGET.getLabel())) {
			copy(NSMGET); return true;
		} else if (msg.equals(NSMSET.getLabel())) {
			copy(NSMSET); return true;
		} else if (msg.equals(DBGET.getLabel())) {
			copy(DBGET); return true;
		} else if (msg.equals(DBSET.getLabel())) {
			copy(DBSET);  return true;
		} 
		return false;
	}

	public boolean copy(int id) {
		if (id == UNKNOWN.getId()) {
			copy(UNKNOWN); return true;
		} else if (id == OK.getId()) {
			copy(OK); return true;
		} else if (id == ERROR.getId()) {
			copy(ERROR); return true;
		} else if (id == NSMGET.getId()) {
			copy(NSMGET); return true;
		} else if (id == NSMSET.getId()) {
			copy(NSMSET); return true;
		} else if (id == DBGET.getId()) {
			copy(DBGET); return true;
		} else if (id == DBSET.getId()) {
			copy(DBSET);  return true;
		} 
		return false;
	}

}
