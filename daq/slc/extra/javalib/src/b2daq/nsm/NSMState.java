package b2daq.nsm;

import b2daq.core.Enum;

public class NSMState extends Enum {

	final static public NSMState UNKNOWN = new NSMState(0, "UNKNOWN");
	final static public NSMState INITIAL_S = new NSMState(101, "INITIAL_S");
	final static public NSMState CONFIGURED_S = new NSMState(102, "CONFIGURED");
	final static public NSMState READY_S = new NSMState(103, "READY");
	final static public NSMState RUNNING_S = new NSMState(104, "RUNNING");
	final static public NSMState PAUSED_S = new NSMState(105, "PAUSED");
	final static public NSMState BOOTING_TS = new NSMState(201, "BOOTING");
	final static public NSMState LOADING_TS = new NSMState(202, "LOADING");
	final static public NSMState STARTING_TS = new NSMState(203, "STARTING");
	final static public NSMState STOPPING_TS = new NSMState(204, "STOPPING");
	final static public NSMState ERROR_ES = new NSMState(301, "ERROR");
	final static public NSMState FATAL_ES = new NSMState(302, "FATAL");
	final static public NSMState RECOVERING_RS = new NSMState(402, "RECOVERING");
	final static public NSMState ABORTING_RS = new NSMState(403, "ABORTING");
	
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
		this(UNKNOWN);
	}

	protected NSMState(int id, String label) {
		super(id, label);
	}

	public NSMState(NSMState state) {
		super(state.getId(), state.getLabel());
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

	public NSMState next() {
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