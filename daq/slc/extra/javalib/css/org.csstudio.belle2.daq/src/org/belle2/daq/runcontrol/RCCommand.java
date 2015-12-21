package org.belle2.daq.runcontrol;

import org.belle2.daq.nsm.NSMCommand;
import org.belle2.daq.nsm.NSMState;

public class RCCommand extends NSMCommand {

    final static public RCCommand CONFIGURE = new RCCommand(101, "RC_CONFIGURE");
    final static public RCCommand LOAD = new RCCommand(102, "RC_LOAD");
    final static public RCCommand START = new RCCommand(103, "RC_START");
    final static public RCCommand STOP = new RCCommand(104, "RC_STOP");
    final static public RCCommand RESUME = new RCCommand(105, "RC_RESUMEN");
    final static public RCCommand PAUSE = new RCCommand(106, "RC_PAUSE");
    final static public RCCommand RECOVER = new RCCommand(107, "RC_RECOVER");
    final static public RCCommand ABORT = new RCCommand(108, "RC_ABORT");

    public RCCommand() {
        copy(RCCommand.UNKNOWN);
    }

    private RCCommand(int id, String label) {
        super(id, label);
    }

    public RCCommand(RCCommand command) {
        copy(command);
    }

    public RCCommand(String command) {
      copy(command);
  }
    public boolean copy(String msg) {
    	if (!msg.startsWith("RC_")) msg = "RC_"+msg;
        if (super.copy(msg)) {
            return true;
        } else if (msg.equals(CONFIGURE.getLabel())) {
            copy(CONFIGURE);
            return true;
        } else if (msg.equals(LOAD.getLabel())) {
            copy(LOAD);
            return true;
        } else if (msg.equals(START.getLabel())) {
            copy(START);
            return true;
        } else if (msg.equals(STOP.getLabel())) {
            copy(STOP);
            return true;
        } else if (msg.equals(RESUME.getLabel())) {
            copy(RESUME);
            return true;
        } else if (msg.equals(PAUSE.getLabel())) {
            copy(PAUSE);
            return true;
        } else if (msg.equals(RECOVER.getLabel())) {
            copy(RECOVER);
            return true;
        } else if (msg.equals(ABORT.getLabel())) {
            copy(ABORT);
            return true;
        }
        return false;
    }

    public int available(NSMState state) {
        if (equals(CONFIGURE) && state.equals(RCState.NOTREADY_S)) {
            return ENABLED;
        } else if (equals(LOAD) && state.equals(RCState.NOTREADY_S)) {
            return SUGGESTED;
        } else if (equals(LOAD) && state.equals(RCState.READY_S)) {
            return ENABLED;
        } else if (equals(START) && state.equals(RCState.READY_S)) {
            return SUGGESTED;
        } else if (equals(STOP)
                && (state.equals(RCState.ERROR_ES) || state.equals(RCState.FATAL_ES)
                || state.equals(RCState.RUNNING_S) || state.equals(RCState.PAUSED_S))) {
            return SUGGESTED;
        } else if (equals(PAUSE) && state.equals(RCState.RUNNING_S)) {
            return ENABLED;
        } else if (equals(RESUME) && state.equals(RCState.PAUSED_S)) {
            return SUGGESTED;
        } else if (equals(OK) || equals(ERROR) || equals(ABORT) || equals(RECOVER)
                || equals(DATAGET) || equals(DATASET) || equals(DBGET) || equals(DBSET)) {
            return ENABLED;
        } else {
            return DISABLED;
        }
    }

    public RCState nextState() {
        if (equals(LOAD)) {
            return RCState.READY_S;
        } else if (equals(START)) {
            return RCState.RUNNING_S;
        } else if (equals(STOP)) {
            return RCState.READY_S;
        } else if (equals(RESUME)) {
            return RCState.RUNNING_S;
        } else if (equals(PAUSE)) {
            return RCState.PAUSED_S;
        } else if (equals(RECOVER)) {
            return RCState.READY_S;
        } else if (equals(ABORT)) {
            return RCState.NOTREADY_S;
        } else {
            return new RCState();
        }
    }

    public RCState nextTState() {
        if (equals(LOAD)) {
            return RCState.LOADING_TS;
        } else if (equals(START)) {
            return RCState.STARTING_TS;
        } else if (equals(STOP)) {
            return RCState.STOPPING_TS;
        } else if (equals(RESUME)) {
            return RCState.RUNNING_S;
        } else if (equals(PAUSE)) {
            return RCState.PAUSED_S;
        } else if (equals(RECOVER)) {
            return RCState.RECOVERING_RS;
        } else if (equals(ABORT)) {
            return RCState.ABORTING_RS;
        } else {
            return new RCState();
        }
    }

    public boolean copy(int id) {
        if (super.copy(id)) {
            return true;
        } else if (id == CONFIGURE.getId()) {
            copy(CONFIGURE);
            return true;
        } else if (id == LOAD.getId()) {
            copy(LOAD);
            return true;
        } else if (id == START.getId()) {
            copy(START);
            return true;
        } else if (id == STOP.getId()) {
            copy(STOP);
            return true;
        } else if (id == RESUME.getId()) {
            copy(RESUME);
            return true;
        } else if (id == PAUSE.getId()) {
            copy(PAUSE);
            return true;
        } else if (id == RECOVER.getId()) {
            copy(RECOVER);
            return true;
        } else if (id == ABORT.getId()) {
            copy(ABORT);
            return true;
        }
        return false;
    }

}
