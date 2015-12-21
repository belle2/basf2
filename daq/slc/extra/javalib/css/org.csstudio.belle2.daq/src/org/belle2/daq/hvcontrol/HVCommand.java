package org.belle2.daq.hvcontrol;

import org.belle2.daq.nsm.NSMCommand;

public class HVCommand extends NSMCommand {

	final static public HVCommand UNKNOWN = new HVCommand(0, "UNKNOWN");
	final static public HVCommand CONFIGURE = new HVCommand(101, "HV_CONFIGURE");
	final static public HVCommand TURNON = new HVCommand(102, "HV_TURNON");
	final static public HVCommand TURNOFF = new HVCommand(103, "HV_TURNOFF");
	final static public HVCommand STANDBY = new HVCommand(111, "HV_STANDBY");
	final static public HVCommand SHOULDER = new HVCommand(112, "HV_SHOULDER");
	final static public HVCommand PEAK = new HVCommand(113, "HV_PEAK");
	final static public HVCommand RECOVER = new HVCommand(121, "HV_RECOVER");

	public HVCommand() {
		copy(HVCommand.UNKNOWN);
	}

	private HVCommand(int id, String label) {
		super(id, label);
	}

	public HVCommand(HVCommand command) {
		copy(command);
	}

	public HVCommand(String command) {
		copy(command);
	}

	@Override
	public boolean copy(String msg) {
    	if (!msg.startsWith("HV_")) msg = "HV_"+msg;
		if (super.copy(msg)) {
			return true;
		} else if (msg.equals(CONFIGURE.getLabel())) {
			copy(CONFIGURE);
			return true;
		} else if (msg.equals(TURNON.getLabel())) {
			copy(TURNON);
			return true;
		} else if (msg.equals(TURNOFF.getLabel())) {
			copy(TURNOFF);
			return true;
		} else if (msg.equals(STANDBY.getLabel())) {
			copy(STANDBY);
			return true;
		} else if (msg.equals(SHOULDER.getLabel())) {
			copy(SHOULDER);
			return true;
		} else if (msg.equals(PEAK.getLabel())) {
			copy(PEAK);
			return true;
		} else if (msg.equals(RECOVER.getLabel())) {
			copy(RECOVER);
			return true;
		}
		return false;
	}

	@Override
	public boolean copy(int id) {
		if (super.copy(id)) {
			return true;
		} else if (id == CONFIGURE.getId()) {
			copy(CONFIGURE);
			return true;
		} else if (id == TURNON.getId()) {
			copy(TURNON);
			return true;
		} else if (id == TURNOFF.getId()) {
			copy(TURNOFF);
			return true;
		} else if (id == STANDBY.getId()) {
			copy(STANDBY);
			return true;
		} else if (id == SHOULDER.getId()) {
			copy(SHOULDER);
			return true;
		} else if (id == PEAK.getId()) {
			copy(PEAK);
			return true;
		} else if (id == RECOVER.getId()) {
			copy(RECOVER);
			return true;
		}
		return false;
	}

}
