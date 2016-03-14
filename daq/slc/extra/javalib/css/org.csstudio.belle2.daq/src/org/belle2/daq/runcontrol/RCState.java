package org.belle2.daq.runcontrol;

import org.belle2.daq.nsm.NSMState;

public class RCState extends NSMState {

    final static public RCState OFF_S = new RCState(1, "OFF");
    final static public RCState NOTREADY_S = new RCState(2, "NOTREADY");
    final static public RCState READY_S = new RCState(3, "READY");
    final static public RCState RUNNING_S = new RCState(4, "RUNNING");
    final static public RCState PAUSED_S = new RCState(5, "PAUSED");
    final static public RCState LOADING_TS = new RCState(6, "LOADING");
    final static public RCState STARTING_TS = new RCState(7, "STARTING");
    final static public RCState STOPPING_TS = new RCState(8, "STOPPING");
    final static public RCState BOOTING_TS = new RCState(9, "BOOTING");
    final static public RCState ERROR_ES = new RCState(10, "ERROR");
    final static public RCState FATAL_ES = new RCState(11, "FATAL");
    final static public RCState RECOVERING_RS = new RCState(12, "RECOVERING");
    final static public RCState ABORTING_RS = new RCState(13, "ABORTING");

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

    public RCState(String state) {
        copy(state);
    }

    @Override
    public void copy(String label) {
        if (label.equals(OFF_S.getLabel())) {
            copy(OFF_S);
        } else if (label.equals(NOTREADY_S.getLabel())) {
            copy(NOTREADY_S);
        } else if (label.equals(READY_S.getLabel())) {
            copy(READY_S);
        } else if (label.equals(RUNNING_S.getLabel())) {
            copy(RUNNING_S);
        } else if (label.equals(PAUSED_S.getLabel())) {
            copy(PAUSED_S);
        } else if (label.equals(LOADING_TS.getLabel())) {
            copy(LOADING_TS);
        } else if (label.equals(STARTING_TS.getLabel())) {
            copy(STARTING_TS);
        } else if (label.equals(STOPPING_TS.getLabel())) {
            copy(STOPPING_TS);
        } else if (label.equals(BOOTING_TS.getLabel())) {
            copy(BOOTING_TS);
        } else if (label.equals(ERROR_ES.getLabel())) {
            copy(ERROR_ES);
        } else if (label.equals(FATAL_ES.getLabel())) {
            copy(FATAL_ES);
        } else if (label.equals(RECOVERING_RS.getLabel())) {
            copy(RECOVERING_RS);
        } else if (label.equals(ABORTING_RS.getLabel())) {
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
        if (id == OFF_S._id) {
            copy(OFF_S);
        } else if (id == NOTREADY_S._id) {
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
        } else if (id == BOOTING_TS._id) {
            copy(BOOTING_TS);
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
