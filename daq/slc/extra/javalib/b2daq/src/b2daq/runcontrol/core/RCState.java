package b2daq.runcontrol.core;

import b2daq.nsm.NSMState;

public class RCState extends NSMState {

    final static public RCState OFF_S = new RCState(1, "OFF");
    final static public RCState NOTREADY_S = new RCState(2, "NOTREADY");
    final static public RCState READY_S = new RCState(3, "READY");
    final static public RCState RUNNING_S = new RCState(4, "RUNNING");
    final static public RCState PAUSED_S = new RCState(5, "PAUSED");
    final static public RCState LOADING_TS = new RCState(6, "LOADING");
    final static public RCState STARTING_TS = new RCState(7, "STARTING");
    final static public RCState STOPPING_TS = new RCState(8, "STOPPING");
    final static public RCState ERROR_ES = new RCState(9, "ERROR");
    final static public RCState FATAL_ES = new RCState(10, "FATAL");
    final static public RCState RECOVERING_RS = new RCState(11, "RECOVERING");
    final static public RCState ABORTING_RS = new RCState(12, "ABORTING");

    public static RCState get(int id) {
        RCState state = new RCState();
        state.copy(id);
        return state;
    }

    public boolean isStable() {
        return getId() > 0 && getId() <= PAUSED_S.getId();
    }

    public boolean isTransition() {
        return getId() > PAUSED_S.getId() && getId() <= STOPPING_TS.getId();
    }

    public boolean isError() {
        return getId() >= ERROR_ES.getId() && getId() <= FATAL_ES.getId();
    }

    public boolean isRecover() {
        return getId() >= RECOVERING_RS.getId() && getId() <= ABORTING_RS.getId();
    }

    public RCState() {
        this(UNKNOWN.getId(), UNKNOWN.getLabel());
    }

    protected RCState(int id, String label) {
        super(id, label);
    }

    public RCState(RCState state) {
        super(state.getId(), state.getLabel());
    }

    @Override
    public void copy(String label) {
        if (label.matches(NOTREADY_S._label)) {
            copy(NOTREADY_S);
        } else if (label.matches(READY_S._label)) {
            copy(READY_S);
        } else if (label.matches(RUNNING_S._label)) {
            copy(RUNNING_S);
        } else if (label.matches(PAUSED_S._label)) {
            copy(PAUSED_S);
        } else if (label.matches(LOADING_TS._label)) {
            copy(LOADING_TS);
        } else if (label.matches(STARTING_TS._label)) {
            copy(STARTING_TS);
        } else if (label.matches(STOPPING_TS._label)) {
            copy(STOPPING_TS);
        } else if (label.matches(ERROR_ES._label)) {
            copy(ERROR_ES);
        } else if (label.matches(FATAL_ES._label)) {
            copy(FATAL_ES);
        } else if (label.matches(RECOVERING_RS._label)) {
            copy(RECOVERING_RS);
        } else if (label.matches(ABORTING_RS._label)) {
            copy(ABORTING_RS);
        } else {
            copy(UNKNOWN);
        }
    }

    public RCState next() {
        if (equals(LOADING_TS)) {
            return READY_S;
        } else if (equals(STARTING_TS)) {
            return RUNNING_S;
        } else if (equals(STOPPING_TS)) {
            return READY_S;
        } else {
            return new RCState();
        }
    }

    @Override
    public void copy(int id) {
        if (id == NOTREADY_S._id) {
            copy(NOTREADY_S);
        } else if (id == READY_S._id) {
            copy(READY_S);
        } else if (id == RUNNING_S._id) {
            copy(RUNNING_S);
        } else if (id == PAUSED_S._id) {
            copy(PAUSED_S);
        } else if (id == LOADING_TS._id) {
            copy(LOADING_TS);
        } else if (id == STARTING_TS._id) {
            copy(STARTING_TS);
        } else if (id == STOPPING_TS._id) {
            copy(STOPPING_TS);
        } else if (id == ERROR_ES._id) {
            copy(ERROR_ES);
        } else if (id == FATAL_ES._id) {
            copy(FATAL_ES);
        } else if (id == RECOVERING_RS._id) {
            copy(RECOVERING_RS);
        } else if (id == ABORTING_RS._id) {
            copy(ABORTING_RS);
        } else {
            copy(UNKNOWN);
        }
    }

}
