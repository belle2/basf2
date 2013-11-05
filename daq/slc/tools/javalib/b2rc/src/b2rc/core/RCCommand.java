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
	final static public RCCommand ABORT = new RCCommand(107, "ABORT", "ABORT");
	final static public RCCommand STATECHECK = new RCCommand(201, "STATECHECK", "STATECHECK");
	final static public RCCommand TRGIFT = new RCCommand(301, "TRIGFT", "TRIGFT");
	final static public RCCommand DATA = new RCCommand(401, "DATA", "DATA");

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
		else if (msg.equals(ABORT.getLabel())) copy(ABORT);
		else if (msg.equals(STATECHECK.getLabel())) copy(STATECHECK);
		else if (msg.equals(TRGIFT.getLabel())) copy(TRGIFT);
		else if (msg.equals(DATA.getLabel())) copy(DATA);
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
		} else if (equals(STATECHECK) || equals(OK) || equals(TRGIFT) || 
				equals(ERROR) || equals(ABORT) || equals(DATA)) {
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
		else return RCState.UNKNOWN;
	}
}
