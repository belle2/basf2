package b2daq.hvcontrol.core;

import b2daq.logger.core.LogMessage;
import b2daq.nsm.NSMState;

public class HVState extends NSMState {

    final static public HVState OFF_S = new HVState(2, "OFF");
    final static public HVState STABLE_S = new HVState(11, "STABLE");
    final static public HVState STANDBY_S = new HVState(12, "STANDBY");
    final static public HVState SHOULDER_S = new HVState(13, "SHOULDER");
    final static public HVState PEAK_S = new HVState(14, "PEAK");
    final static public HVState TRANSITION_TS = new HVState(31, "TRANSITION");
    final static public HVState RAMPINGUP_TS = new HVState(32, "RAMPINGUP");
    final static public HVState RAMPINGDOWN_TS = new HVState(33, "RAMPINGDOWN");
    final static public HVState ERROR_ES = new HVState(41, "ERROR");
    final static public HVState TRIP_ES = new HVState(42, "TRIP");
    final static public HVState OVER_CURRENT_ES = new HVState(43, "OVER_CURRENT");
    final static public HVState OVER_VOLTAGE_ES = new HVState(44, "OVER_VOLTAGE");

    static public final String getCSSPath() {
        return HVState.class.getResource("HVState.css").toExternalForm();
    }

    public static HVState get(int value) {
        HVState state = new HVState();
        state.copy(value);
        return state;
    }
    
    public boolean isStable() {
        return getId() > 0 && getId() <= PEAK_S.getId();
    }

    public boolean isTransition() {
        return getId() > PEAK_S.getId() && getId() <= RAMPINGDOWN_TS.getId();
    }

    public boolean isError() {
        return getId() >= ERROR_ES.getId() && getId() <= OVER_VOLTAGE_ES.getId();
    }

    public HVState() {
        this(UNKNOWN.getId(), UNKNOWN.getLabel());
    }

    protected HVState(int id, String label) {
        super(id, label);
    }

    public HVState(HVState state) {
        super(state.getId(), state.getLabel());
    }

    @Override
    public void copy(String label) {
        if (label.matches(OFF_S._label)) {
            copy(OFF_S);
        } else if (label.matches(STABLE_S._label)) {
            copy(STABLE_S);
        } else if (label.matches(STANDBY_S._label)) {
            copy(STANDBY_S);
        } else if (label.matches(SHOULDER_S._label)) {
            copy(SHOULDER_S);
        } else if (label.matches(PEAK_S._label)) {
            copy(PEAK_S);
        } else if (label.matches(TRANSITION_TS._label)) {
            copy(TRANSITION_TS);
        } else if (label.matches(RAMPINGUP_TS._label)) {
            copy(RAMPINGUP_TS);
        } else if (label.matches(RAMPINGDOWN_TS._label)) {
            copy(RAMPINGDOWN_TS);
        } else if (label.matches(ERROR_ES._label)) {
            copy(ERROR_ES);
        } else if (label.matches(TRIP_ES._label)) {
            copy(TRIP_ES);
        } else if (label.matches(OVER_VOLTAGE_ES._label)) {
            copy(OVER_VOLTAGE_ES);
        } else if (label.matches(OVER_CURRENT_ES._label)) {
            copy(OVER_CURRENT_ES);
        } else {
            copy(UNKNOWN);
        }
    }

    @Override
    public void copy(int id) {
        if (id == OFF_S._id) {
            copy(OFF_S);
        } else if (id == STABLE_S._id) {
            copy(STABLE_S);
        } else if (id == STANDBY_S._id) {
            copy(STANDBY_S);
        } else if (id == SHOULDER_S._id) {
            copy(SHOULDER_S);
        } else if (id == PEAK_S._id) {
            copy(PEAK_S);
        } else if (id == TRANSITION_TS._id) {
            copy(TRANSITION_TS);
        } else if (id == RAMPINGUP_TS._id) {
            copy(RAMPINGUP_TS);
        } else if (id == RAMPINGDOWN_TS._id) {
            copy(RAMPINGDOWN_TS);
        } else if (id == ERROR_ES._id) {
            copy(ERROR_ES);
        } else if (id == TRIP_ES._id) {
            copy(TRIP_ES);
        } else if (id == OVER_VOLTAGE_ES._id) {
            copy(OVER_VOLTAGE_ES);
        } else if (id == OVER_CURRENT_ES._id) {
            copy(OVER_CURRENT_ES);
        } else {
            copy(UNKNOWN);
        }
    }

}
