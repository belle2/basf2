package b2rc.core;

public class RCCommand extends RCEnum {

	final static public int DISABLED = 0;
	final static public int ENABLED = 1;
	final static public int SUGGESTED = 2;

	final static public RCCommand UNKNOWN = new RCCommand(-1, "UNKNOWN", "UNKNOWN");
	final static public RCCommand OK = new RCCommand(1, "OK", "OK");
	final static public RCCommand ERROR = new RCCommand(2, "ERROR", "ERROR");
	final static public RCCommand BOOT = new RCCommand(101, "BOOT", "BOOT");
	final static public RCCommand LOAD = new RCCommand(102, "LOAD", "LOAD");
	final static public RCCommand START = new RCCommand(103, "START", "START");
	final static public RCCommand STOP = new RCCommand(104, "STOP", "STOP");
	final static public RCCommand RESUME = new RCCommand(105, "RESUMEN", "RESUME");
	final static public RCCommand PAUSE = new RCCommand(106, "PAUSE", "PAUSE");
	final static public RCCommand RECOVER = new RCCommand(108, "RECOVER", "RECOVER");
	final static public RCCommand ABORT = new RCCommand(107, "ABORT", "ABORT");
	final static public RCCommand STATECHECK = new RCCommand(201, "STATECHECK", "STATECHECK");
	final static public RCCommand STATE = new RCCommand(202, "STATE", "STATE");
	final static public RCCommand TRGIFT = new RCCommand(301, "TRIGFT", "TRIGFT");
	final static public RCCommand DATA = new RCCommand(401, "DATA", "DATA");
	final static public RCCommand SAVE = new RCCommand(402, "SAVE", "SAVE");
	final static public RCCommand RECALL = new RCCommand(403, "RECALL", "RECALL");
	final static public RCCommand ACTIVATE = new RCCommand(404, "ACTIVATE", "ACTIVATE");
	final static public RCCommand INACTIVATE = new RCCommand(405, "INACTIVATE", "INACTIVATE");

	public RCCommand() {
		copy(RCCommand.UNKNOWN);
	}

	private RCCommand(int id, String label, String alias) {
		super(id, label, alias);
	}

	public RCCommand(RCCommand command) {
		copy(command);
	}

	public void copy(String msg) throws Exception {
		if (msg.equals(UNKNOWN.getLabel())) copy(UNKNOWN);
		else if (msg.equals(OK.getLabel())) copy(OK);
		else if (msg.equals(ERROR.getLabel())) copy(ERROR);
		else if (msg.equals(BOOT.getLabel())) copy(BOOT);
		else if (msg.equals(LOAD.getLabel())) copy(LOAD);
		else if (msg.equals(START.getLabel())) copy(START);
		else if (msg.equals(STOP.getLabel())) copy(STOP);
		else if (msg.equals(RESUME.getLabel())) copy(RESUME);
		else if (msg.equals(PAUSE.getLabel())) copy(PAUSE);
		else if (msg.equals(RECOVER.getLabel())) copy(RECOVER);
		else if (msg.equals(ABORT.getLabel())) copy(ABORT);
		else if (msg.equals(STATECHECK.getLabel())) copy(STATECHECK);
		else if (msg.equals(STATE.getLabel())) copy(STATE);
		else if (msg.equals(TRGIFT.getLabel())) copy(TRGIFT);
		else if (msg.equals(DATA.getLabel())) copy(DATA);
		else if (msg.equals(SAVE.getLabel())) copy(SAVE);
		else if (msg.equals(RECALL.getLabel())) copy(RECALL);
		else if (msg.equals(ACTIVATE.getLabel())) copy(ACTIVATE);
		else if (msg.equals(INACTIVATE.getLabel())) copy(INACTIVATE);
		else {
			copy(UNKNOWN);
			//throw new RCIllegalCommandException("Illegal command = " + msg);
		}
	}

	public int available(RCState state) {
		if (equals(BOOT) && state.equals(RCState.INITIAL_S)) {
			return SUGGESTED;
		} else if (equals(BOOT) && state.equals(RCState.READY_S)) {
			return ENABLED;
		} else if (equals(LOAD) && state.equals(RCState.CONFIGURED_S)) {
			return SUGGESTED;
		} else if (equals(LOAD) && state.equals(RCState.READY_S)) {
			return ENABLED;
		} else if (equals(START) && state.equals(RCState.READY_S)) {
			return SUGGESTED;
		} else if (equals(STOP) && 
				(state.equals(RCState.RUNNING_S) || state.equals(RCState.PAUSED_S))) {
			return SUGGESTED;
		} else if (equals(PAUSE) && state.equals(RCState.RUNNING_S)) {
			return ENABLED;
		} else if (equals(RESUME) && state.equals(RCState.PAUSED_S)) {
			return SUGGESTED;
		} else if (equals(STATECHECK) || equals(STATE) || equals(OK) || 
				equals(TRGIFT) || equals(ERROR) || equals(ABORT) || equals(RECOVER) ||
				equals(DATA) || equals(SAVE) || equals(RECALL) || 
				equals(ACTIVATE) || equals(INACTIVATE) ) {
			return ENABLED;
		} else {
			return DISABLED;
		}
	}

	public RCState nextState() {
		if (equals(BOOT)) return RCState.CONFIGURED_S;
		else if (equals(LOAD)) return RCState.READY_S;
		else if (equals(START)) return RCState.RUNNING_S;
		else if (equals(STOP)) return RCState.READY_S;
		else if (equals(RESUME)) return RCState.RUNNING_S;
		else if (equals(PAUSE)) return RCState.PAUSED_S;
		else if (equals(RECOVER)) return RCState.READY_S;
		else if (equals(ABORT)) return RCState.INITIAL_S;
		else return RCState.UNKNOWN;
	}

	public RCState nextTState() {
		if (equals(BOOT)) return RCState.BOOTING_TS;
		else if (equals(LOAD)) return RCState.LOADING_TS;
		else if (equals(START)) return RCState.STARTING_TS;
		else if (equals(STOP)) return RCState.STOPPING_TS;
		else if (equals(RESUME)) return RCState.RUNNING_S;
		else if (equals(PAUSE)) return RCState.PAUSED_S;
		else if (equals(RECOVER)) return RCState.RECOVERING_RS;
		else if (equals(ABORT)) return RCState.ABORTING_RS;
		else return RCState.UNKNOWN;
	}

	public void copy(int id) {
		if (id == UNKNOWN.getId()) copy(UNKNOWN);
		else if (id == OK.getId()) copy(OK);
		else if (id == ERROR.getId()) copy(ERROR);
		else if (id == BOOT.getId()) copy(BOOT);
		else if (id == LOAD.getId()) copy(LOAD);
		else if (id == START.getId()) copy(START);
		else if (id == STOP.getId()) copy(STOP);
		else if (id == RESUME.getId()) copy(RESUME);
		else if (id == PAUSE.getId()) copy(PAUSE);
		else if (id == RECOVER.getId()) copy(RECOVER);
		else if (id == ABORT.getId()) copy(ABORT);
		else if (id == STATECHECK.getId()) copy(STATECHECK);
		else if (id == STATE.getId()) copy(STATE);
		else if (id == TRGIFT.getId()) copy(TRGIFT);
		else if (id == DATA.getId()) copy(DATA);
		else if (id == SAVE.getId()) copy(SAVE);
		else if (id == RECALL.getId()) copy(RECALL);
		else if (id == ACTIVATE.getId()) copy(ACTIVATE);
		else if (id == INACTIVATE.getId()) copy(INACTIVATE);
		else copy(UNKNOWN);
	}

}
