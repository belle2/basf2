package b2daq.runcontrol.core;

import b2daq.nsm.NSMCommand;
import b2daq.nsm.NSMState;

public class RCCommand extends NSMCommand {

    final static public RCCommand BOOT = new RCCommand(101, "BOOT");
    final static public RCCommand LOAD = new RCCommand(102, "LOAD");
    final static public RCCommand START = new RCCommand(103, "START");
    final static public RCCommand STOP = new RCCommand(104, "STOP");
    final static public RCCommand RESUME = new RCCommand(105, "RESUMEN");
    final static public RCCommand PAUSE = new RCCommand(106, "PAUSE");
    final static public RCCommand RECOVER = new RCCommand(108, "RECOVER");
    final static public RCCommand ABORT = new RCCommand(107, "ABORT");
    final static public RCCommand TRGIFT = new RCCommand(301, "TRIGFT");
    final static public RCCommand MASK = new RCCommand(404, "MASK");
    final static public RCCommand INMASK = new RCCommand(405, "INMASK");

    public RCCommand() {
        copy(RCCommand.UNKNOWN);
    }

    private RCCommand(int id, String label) {
        super(id, label);
    }

    public RCCommand(RCCommand command) {
        copy(command);
    }

    public boolean copy(String msg) {
        if (super.copy(msg)) {
            return true;
        } else if (msg.equals(BOOT.getLabel())) {
            copy(BOOT);
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
        } else if (msg.equals(TRGIFT.getLabel())) {
            copy(TRGIFT);
            return true;
        } else if (msg.equals(MASK.getLabel())) {
            copy(MASK);
            return true;
        } else if (msg.equals(INMASK.getLabel())) {
            copy(INMASK);
            return true;
        }
        return false;
    }

    public int available(NSMState state) {
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
        } else if (equals(STOP)
                && (state.equals(RCState.ERROR_ES) || state.equals(RCState.FATAL_ES)
                || state.equals(RCState.RUNNING_S) || state.equals(RCState.PAUSED_S))) {
            return SUGGESTED;
        } else if (equals(PAUSE) && state.equals(RCState.RUNNING_S)) {
            return ENABLED;
        } else if (equals(RESUME) && state.equals(RCState.PAUSED_S)) {
            return SUGGESTED;
        } else if (equals(STATECHECK) || equals(STATE) || equals(OK)
                || equals(TRGIFT) || equals(ERROR) || equals(ABORT) || equals(RECOVER)
                || equals(NSMGET) || equals(NSMSET) || equals(DBGET) || equals(DBSET)
                || equals(MASK) || equals(INMASK)) {
            return ENABLED;
        } else {
            return DISABLED;
        }
    }

    public RCState nextState() {
        if (equals(BOOT)) {
            return RCState.CONFIGURED_S;
        } else if (equals(LOAD)) {
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
            return RCState.INITIAL_S;
        } else {
            return new RCState();
        }
    }

    public RCState nextTState() {
        if (equals(BOOT)) {
            return RCState.BOOTING_TS;
        } else if (equals(LOAD)) {
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
        } else if (id == BOOT.getId()) {
            copy(BOOT);
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
        } else if (id == TRGIFT.getId()) {
            copy(TRGIFT);
            return true;
        } else if (id == MASK.getId()) {
            copy(MASK);
            return true;
        } else if (id == INMASK.getId()) {
            copy(INMASK);
            return true;
        }
        return false;
    }

}
