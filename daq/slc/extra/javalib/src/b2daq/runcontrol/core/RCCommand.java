package b2daq.runcontrol.core;

import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMState;

public class RCCommand extends NSMCommand {

	final static public int DISABLED = 0;
	final static public int ENABLED = 1;
	final static public int SUGGESTED = 2;

	final static public RCCommand BOOT = new RCCommand(101, "BOOT");
	final static public RCCommand LOAD = new RCCommand(102, "LOAD");
	final static public RCCommand START = new RCCommand(103, "START");
	final static public RCCommand STOP = new RCCommand(104, "STOP");
	final static public RCCommand RESUME = new RCCommand(105, "RESUMEN");
	final static public RCCommand PAUSE = new RCCommand(106, "PAUSE");
	final static public RCCommand RECOVER = new RCCommand(108, "RECOVER");
	final static public RCCommand ABORT = new RCCommand(107, "ABORT");
	final static public RCCommand STATECHECK = new RCCommand(201, "STATECHECK");
	final static public RCCommand STATE = new RCCommand(202, "STATE");
	final static public RCCommand TRGIFT = new RCCommand(301, "TRIGFT");
	final static public RCCommand ACTIVATE = new RCCommand(404, "ACTIVATE");
	final static public RCCommand INACTIVATE = new RCCommand(405, "INACTIVATE");
	final static public RCCommand PEAK = new RCCommand(406, "PEAK");

	public RCCommand() {
		copy(RCCommand.UNKNOWN);
	}

	private RCCommand(int id, String label) {
		super(id, label);
	}

	public RCCommand(RCCommand command) {
		copy(command);
	}

	public boolean copy(String msg) throws Exception {
		if (super.copy(msg)) { return true; }
		else if (msg.equals(BOOT.getLabel())) { copy(BOOT); return true; } 
		else if (msg.equals(LOAD.getLabel())) { copy(LOAD); return true; }
		else if (msg.equals(START.getLabel())) { copy(START); return true; }
		else if (msg.equals(STOP.getLabel())) { copy(STOP); return true; }
		else if (msg.equals(RESUME.getLabel())) { copy(RESUME); return true; }
		else if (msg.equals(PAUSE.getLabel())) { copy(PAUSE); return true; }
		else if (msg.equals(RECOVER.getLabel())) { copy(RECOVER); return true; }
		else if (msg.equals(ABORT.getLabel())) {copy(ABORT); return true; }
		else if (msg.equals(STATECHECK.getLabel())) { copy(STATECHECK); return true; }
		else if (msg.equals(STATE.getLabel())) { copy(STATE); return true; }
		else if (msg.equals(TRGIFT.getLabel())) { copy(TRGIFT); return true;}
		else if (msg.equals(ACTIVATE.getLabel())) {copy(ACTIVATE); return true; }
		else if (msg.equals(INACTIVATE.getLabel())) { copy(INACTIVATE); return true; }
		return false;
	}

	public int available(NSMState state) {
		if (equals(BOOT) && state.equals(NSMState.INITIAL_S)) {
			return SUGGESTED;
		} else if (equals(BOOT) && state.equals(NSMState.READY_S)) {
			return ENABLED;
		} else if (equals(LOAD) && state.equals(NSMState.CONFIGURED_S)) {
			return SUGGESTED;
		} else if (equals(LOAD) && state.equals(NSMState.READY_S)) {
			return ENABLED;
		} else if (equals(START) && state.equals(NSMState.READY_S)) {
			return SUGGESTED;
		} else if (equals(STOP) && 
				(state.equals(NSMState.ERROR_ES) || state.equals(NSMState.FATAL_ES) ||  
				 state.equals(NSMState.RUNNING_S) || state.equals(NSMState.PAUSED_S))) {
			return SUGGESTED;
		} else if (equals(PAUSE) && state.equals(NSMState.RUNNING_S)) {
			return ENABLED;
		} else if (equals(RESUME) && state.equals(NSMState.PAUSED_S)) {
			return SUGGESTED;
		} else if (equals(STATECHECK) || equals(STATE) || equals(OK) || 
				equals(TRGIFT) || equals(ERROR) || equals(ABORT) || equals(RECOVER) ||
				equals(NSMGET) || equals(NSMSET) || equals(DBGET) || equals(DBSET) || 
				equals(ACTIVATE) || equals(INACTIVATE) ) {
			return ENABLED;
		} else {
			return DISABLED;
		}
	}

	public NSMState nextState() {
		if (equals(BOOT)) return NSMState.CONFIGURED_S;
		else if (equals(LOAD)) return NSMState.READY_S;
		else if (equals(START)) return NSMState.RUNNING_S;
		else if (equals(STOP)) return NSMState.READY_S;
		else if (equals(RESUME)) return NSMState.RUNNING_S;
		else if (equals(PAUSE)) return NSMState.PAUSED_S;
		else if (equals(RECOVER)) return NSMState.READY_S;
		else if (equals(ABORT)) return NSMState.INITIAL_S;
		else return NSMState.UNKNOWN;
	}

	public NSMState nextTState() {
		if (equals(BOOT)) return NSMState.BOOTING_TS;
		else if (equals(LOAD)) return NSMState.LOADING_TS;
		else if (equals(START)) return NSMState.STARTING_TS;
		else if (equals(STOP)) return NSMState.STOPPING_TS;
		else if (equals(RESUME)) return NSMState.RUNNING_S;
		else if (equals(PAUSE)) return NSMState.PAUSED_S;
		else if (equals(RECOVER)) return NSMState.RECOVERING_RS;
		else if (equals(ABORT)) return NSMState.ABORTING_RS;
		else return NSMState.UNKNOWN;
	}

	public boolean copy(int id) {
		if (super.copy(id)) { return true; }
		else if (id == BOOT.getId()) { copy(BOOT); return true; } 
		else if (id == LOAD.getId()) { copy(LOAD); return true; }
		else if (id == START.getId()) { copy(START); return true; }
		else if (id == STOP.getId()) { copy(STOP); return true; }
		else if (id == RESUME.getId()) { copy(RESUME); return true; }
		else if (id == PAUSE.getId()) { copy(PAUSE); return true; }
		else if (id == RECOVER.getId()) { copy(RECOVER); return true; }
		else if (id == ABORT.getId()) {copy(ABORT); return true; }
		else if (id == STATECHECK.getId()) { copy(STATECHECK); return true; }
		else if (id == STATE.getId()) { copy(STATE); return true; }
		else if (id == TRGIFT.getId()) { copy(TRGIFT); return true;}
		else if (id == ACTIVATE.getId()) {copy(ACTIVATE); return true; }
		else if (id == INACTIVATE.getId()) { copy(INACTIVATE); return true; }
		return false;
	}

}
