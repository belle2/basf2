package b2daq.nsm;

import b2daq.core.Enum;

public class NSMState extends Enum {

    final static public NSMState UNKNOWN = new NSMState(0, "UNKNOWN");

    private boolean _active = true;
    private boolean _masked = false;
    private boolean _enabled = true;

    public boolean isActive() {
        return _active;
    }

    public boolean isMasked() {
        return _masked;
    }

    public boolean isEnabled() {
        return _enabled;
    }

    public boolean isAvailable() {
        return isActive() && isEnabled() && !isMasked();
    }

    public void setActive(boolean active) {
        _active = active;
    }

    public void setEnabled(boolean enabled) {
        _enabled = enabled;
    }

    public void setMasked(boolean masked) {
        _masked = masked;
    }

    public NSMState() {
        this(UNKNOWN.getId(), UNKNOWN.getLabel());
    }

    protected NSMState(int id, String label) {
        super(id, label);
    }

    public NSMState(NSMState state) {
        super(state.getId(), state.getLabel());
    }

    public void copy(String label) {
        copy(UNKNOWN);
    }

    public void copy(int id) {
        copy(UNKNOWN);
    }

}
