package b2daq.core;

public class RCState extends RCEnum {

	final static public RCState UNKNOWN = new RCState(0, "UNKNOWN", "UNKNOWN");
	final static public RCState INITIAL_S = new RCState(101, "INITIAL_S", "INITIAL");
	final static public RCState CONFIGURED_S = new RCState(102, "CONFIGURED_S", "CONFIGURED");
	final static public RCState READY_S = new RCState(103, "READY_S", "READY");
	final static public RCState RUNNING_S = new RCState(104, "RUNNING_S", "RUNNING");
	final static public RCState PAUSED_S = new RCState(105, "PAUSED_S", "PAUSED");
	final static public RCState BOOTING_TS = new RCState(201, "BOOTING_TS", "BOOTING");
	final static public RCState LOADING_TS = new RCState(202, "LOADING_TS", "LOADING");
	final static public RCState STARTING_TS = new RCState(203, "STARTING_TS", "STARTING");
	final static public RCState STOPPING_TS = new RCState(204, "STOPPING_TS", "STOPPING");
	final static public RCState ERROR_ES = new RCState(301, "ERROR_ES", "ERROR");
	final static public RCState FATAL_ES = new RCState(302, "FATAL_ES", "FATAL");
	final static public RCState RECOVERING_RS = new RCState(402, "RECOVERING_RS", "RECOVERING");
	final static public RCState ABORTING_RS = new RCState(403, "ABORTING_RS", "ABORTING");
	
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
	
	public RCState() {
		this(UNKNOWN);
	}

	protected RCState(int id, String label, String alias) {
		super(id, label, alias);
	}

	public RCState(RCState state) {
		super(state.getId(), state.getLabel(), state.getAlias());
	}
	
	public void copy(String label) {
		  if ( label.matches(INITIAL_S._label) ) copy(INITIAL_S);
		  else if ( label.matches(CONFIGURED_S._label) ) copy(CONFIGURED_S);
		  else if ( label.matches(READY_S._label) ) copy(READY_S);
		  else if ( label.matches(RUNNING_S._label) ) copy(RUNNING_S);
		  else if ( label.matches(PAUSED_S._label) ) copy(PAUSED_S);
		  else if ( label.matches(BOOTING_TS._label) ) copy(BOOTING_TS);
		  else if ( label.matches(LOADING_TS._label) ) copy(LOADING_TS);
		  else if ( label.matches(STARTING_TS._label) ) copy(STARTING_TS);
		  else if ( label.matches(STOPPING_TS._label) ) copy(STOPPING_TS);
		  else if ( label.matches(ERROR_ES._label) ) copy(ERROR_ES);
		  else if ( label.matches(FATAL_ES._label) ) copy(FATAL_ES);
		  else if ( label.matches(RECOVERING_RS._label) ) copy(RECOVERING_RS);
		  else if ( label.matches(ABORTING_RS._label) ) copy(ABORTING_RS);
		  else copy(UNKNOWN);
	}

	public RCState next() {
	  if ( equals(BOOTING_TS) ) return CONFIGURED_S;
	  else if ( equals(LOADING_TS) ) return READY_S;
	  else if ( equals(STARTING_TS) ) return RUNNING_S;
	  else if ( equals(STOPPING_TS) ) return READY_S;
	  else return UNKNOWN;
	}

	public void copy(int id) {
		  if ( id == INITIAL_S._id ) copy(INITIAL_S);
		  else if ( id == CONFIGURED_S._id ) copy(CONFIGURED_S);
		  else if ( id == READY_S._id ) copy(READY_S);
		  else if ( id == RUNNING_S._id ) copy(RUNNING_S);
		  else if ( id == PAUSED_S._id ) copy(PAUSED_S);
		  else if ( id == BOOTING_TS._id ) copy(BOOTING_TS);
		  else if ( id == LOADING_TS._id ) copy(LOADING_TS);
		  else if ( id == STARTING_TS._id ) copy(STARTING_TS);
		  else if ( id == STOPPING_TS._id ) copy(STOPPING_TS);
		  else if ( id == ERROR_ES._id ) copy(ERROR_ES);
		  else if ( id == FATAL_ES._id ) copy(FATAL_ES);
		  else if ( id == RECOVERING_RS._id ) copy(RECOVERING_RS);
		  else if ( id == ABORTING_RS._id ) copy(ABORTING_RS);
		  else copy(UNKNOWN);
	}

	public boolean isError() {
		return equals(ERROR_ES) || equals(FATAL_ES);
	}

}