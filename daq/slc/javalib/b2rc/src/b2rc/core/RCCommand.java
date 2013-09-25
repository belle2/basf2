package b2rc.core;

public class RCCommand extends RCEnum {

	final static public int DISABLED = 0;
	final static public int ENABLED = 1;
	final static public int SUGGESTED = 2;
	
	final static public RCCommand UNKNOWN = new RCCommand(-1, "UNKNOWN", "UNKNOWN");
	final static public RCCommand OK = new RCCommand(1, "OK", "OK");
	final static public RCCommand ERROR = new RCCommand(2, "ERROR", "ERROR");
	final static public RCCommand BOOT = new RCCommand(101, "RC_BOOT", "BOOT");
	final static public RCCommand REBOOT = new RCCommand(102, "RC_REBOOT", "REBOOT");
	final static public RCCommand LOAD = new RCCommand(103, "RC_LOAD", "LOAD");
	final static public RCCommand RELOAD = new RCCommand(104, "RC_RELOAD", "RELOAD");
	final static public RCCommand START = new RCCommand(105, "RC_START", "START");
	final static public RCCommand STOP = new RCCommand(106, "RC_STOP", "STOP");
	final static public RCCommand RESUME = new RCCommand(107, "RC_RESUMEN", "RESUME");
	final static public RCCommand PAUSE = new RCCommand(108, "RC_PAUSE", "PAUSE");
	final static public RCCommand RECOVER = new RCCommand(202, "RC_RECOVER", "RECOVER");
	final static public RCCommand ABORT = new RCCommand(203, "RC_ABORT", "ABORT");
	final static public RCCommand STATECHECK = new RCCommand(301, "RC_STATECHECK", "STATECHECK");
	final static public RCCommand STATE = new RCCommand(302, "RC_STATE", "STATE");
	final static public RCCommand SET = new RCCommand(401, "RC_SET", "SET");
	final static public RCCommand GET = new RCCommand(401, "RC_GET", "GET");
	
	final static public RCCommand [] CommandList = {
		BOOT, REBOOT,LOAD, RELOAD,
		RELOAD, START, STOP, RESUME, PAUSE};

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
		else if (msg.equals(REBOOT.getLabel())) copy(REBOOT);
		else if (msg.equals(LOAD.getLabel())) copy(LOAD);
		else if (msg.equals(RELOAD.getLabel())) copy(RELOAD);
		else if (msg.equals(START.getLabel())) copy(START);
		else if (msg.equals(STOP.getLabel())) copy(STOP);
		else if (msg.equals(RESUME.getLabel())) copy(RESUME);
		else if (msg.equals(PAUSE.getLabel())) copy(PAUSE);
		else if (msg.equals(GET.getLabel())) copy(GET);
		else if (msg.equals(SET.getLabel())) copy(SET);
		else if (msg.equals(STATE.getLabel())) copy(STATE);
		else if (msg.equals(STATECHECK.getLabel())) copy(STATECHECK);
		else if (msg.equals(RECOVER.getLabel())) copy(RECOVER);
		else if (msg.equals(ABORT.getLabel())) copy(ABORT);
		else throw new RCIllegalCommandException("Illegal command = " + msg); 
	}
	
	public int available(RCState state) {
		  if ( equals(BOOT) && state.equals(RCState.INITIAL_S)) {
			    return SUGGESTED;
			  } else if ( equals(REBOOT) && state.equals(RCState.CONFIGURED_S)) {
			    return ENABLED;
			  } else if ( equals(LOAD) && state.equals(RCState.CONFIGURED_S)) {
			    return SUGGESTED;
			  } else if ( equals(RELOAD) && state.equals(RCState.READY_S)) {
			    return ENABLED;
			  } else if ( equals(START) && state.equals(RCState.READY_S)) {
			    return SUGGESTED;
			  } else if ( equals(STOP) && (state.equals(RCState.RUNNING_S) ||
			                                state.equals(RCState.PAUSED_S)) ) {
			    return SUGGESTED;
			  } else if ( equals(PAUSE) && state.equals(RCState.RUNNING_S)) {
			    return ENABLED;
			  } else if ( equals(RESUME) && state.equals(RCState.PAUSED_S)) {
			    return SUGGESTED;
			  } else if ( equals(STATE) || equals(SET) || equals(STATECHECK) || equals(OK) ||
			              equals(ERROR) || equals(RECOVER) || equals(ABORT) ){
			    return ENABLED;
			  } else {
			    return DISABLED;
			  }
	}
	
	public RCState nextState() {
		  if ( equals(BOOT) ) return RCState.CONFIGURED_S;
		  else if ( equals(REBOOT) ) return RCState.CONFIGURED_S;
		  else if ( equals(LOAD) ) return RCState.READY_S;
		  else if ( equals(RELOAD) ) return RCState.READY_S;
		  else if ( equals(START) ) return RCState.RUNNING_S;
		  else if ( equals(STOP) ) return RCState.READY_S;
		  else if ( equals(RESUME) ) return RCState.RUNNING_S;
		  else if ( equals(PAUSE) ) return RCState.PAUSED_S;
		  else return RCState.UNKNOWN;
	}
}
