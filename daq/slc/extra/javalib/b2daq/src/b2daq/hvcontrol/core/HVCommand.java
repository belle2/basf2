package b2daq.hvcontrol.core;

import b2daq.nsm.NSMCommand;

public class HVCommand extends NSMCommand {

        final static public HVCommand UNKNOWN = new HVCommand(0, "UNKNOWN");
        final static public HVCommand CONFIGURE = new HVCommand(101, "CONFIGURE");
        final static public HVCommand TURNON = new HVCommand(102, "TURNON");
        final static public HVCommand TURNOFF = new HVCommand(103, "TURNOFF");
        final static public HVCommand STANDBY = new HVCommand(111, "STANDBY");
        final static public HVCommand SHOULDER = new HVCommand(112, "SHOULDER");
        final static public HVCommand PEAK = new HVCommand(113, "PEAK");
        final static public HVCommand RECOVER = new HVCommand(121, "RECOVER");
        final static public HVCommand HVAPPLY = new HVCommand(131, "HVAPPLY");

	public HVCommand() {
		copy(HVCommand.UNKNOWN);
	}

	private HVCommand(int id, String label) {
		super(id, label);
	}

	public HVCommand(HVCommand command) {
		copy(command);
	}

        @Override
	public boolean copy(String msg) {
		if (super.copy(msg)) { return true; }
		else if (msg.equals(CONFIGURE.getLabel())) { copy(CONFIGURE); return true; } 
		else if (msg.equals(TURNON.getLabel())) { copy(TURNON); return true; }
		else if (msg.equals(TURNOFF.getLabel())) { copy(TURNOFF); return true; }
		else if (msg.equals(STANDBY.getLabel())) { copy(STANDBY); return true; }
		else if (msg.equals(SHOULDER.getLabel())) { copy(SHOULDER); return true; }
		else if (msg.equals(PEAK.getLabel())) { copy(PEAK); return true; }
		else if (msg.equals(RECOVER.getLabel())) { copy(RECOVER); return true; }
		else if (msg.equals(HVAPPLY.getLabel())) { copy(HVAPPLY); return true; }
		return false;
	}

        @Override
	public boolean copy(int id) {
		if (super.copy(id)) { return true; }
		else if (id == CONFIGURE.getId()) { copy(CONFIGURE); return true; } 
		else if (id == TURNON.getId()) { copy(TURNON); return true; }
		else if (id == TURNOFF.getId()) { copy(TURNOFF); return true; }
		else if (id == STANDBY.getId()) { copy(STANDBY); return true; }
		else if (id == SHOULDER.getId()) { copy(SHOULDER); return true; }
		else if (id == PEAK.getId()) { copy(PEAK); return true; }
		else if (id == RECOVER.getId()) { copy(RECOVER); return true; }
		else if (id == HVAPPLY.getId()) { copy(HVAPPLY); return true; }
		return false;
	}

}
