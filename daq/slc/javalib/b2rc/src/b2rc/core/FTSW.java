package b2rc.core;

public class FTSW extends Module {

	public static final String TAG = "ftsw";
	public static final int TRIG_NORMAL = 0;
	public static final int TRIG_IN = 1;
	public static final int TRIG_TLU = 2;
	public static final int TRIG_PULSE = 4;
	public static final int TRIG_REVO = 5;
	public static final int TRIG_RANDOM = 6;
	public static final int TRIG_POSSION = 7;
	public static final int TRIG_ONCE = 8;
	public static final int TRIG_STOP = 9;
	
	private int _channel;
	private String _firmware;
	private boolean _used = true;
	private FEEModule[] _module_v = new FEEModule[16];
	private int _module_i = 0;
	private int _trigger_mode = TRIG_NORMAL;
	
	public FTSW() {

	}

	public int getChannel() {
		return _channel;
	}

	public String getFirmware() {
		return _firmware;
	}

	public boolean isUsed() {
		return _used;
	}

	public FEEModule[] getFEEModules() {
		return _module_v;
	}

	public void setChannel(int channel) {
		_channel = channel;
	}

	public void setFirmware(String firmware) {
		_firmware = firmware;
	}

	public void setUsed(boolean used) {
		_used = used;
	}

	public void clearModules() {
		_module_i = 0;
	}

	public void addFEEModule(FEEModule module) {
		if (_module_i < _module_v.length) {
			_module_v[_module_i] = module;
			_module_i++;
		}
	}

	public int getTriggerMode() {
		return _trigger_mode;
	}
	
	public void setTriggerMode(int mode) {
		_trigger_mode = mode;
	}

	@Override
	public String getSQLFields() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLFields());
		buf.append(", used boolean, channel smallint, firmware text, trigger_mode int"); 
		for (int i = 0; i < _module_v.length; i++) {
			buf.append(", module_type_" + i + " text");
			buf.append(", module_id_" + i + " smallint");
		}
		return buf.toString();
	}

	@Override
	public String getSQLLabels() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLLabels());
		buf.append(", used, channel, firmware, trigger_mode"); 
		for (int i = 0; i < _module_v.length; i++) {
			buf.append(", module_type_" + i);
			buf.append(", module_id_" + i);
		}
		return buf.toString();
	}

	@Override
	public String getSQLValues() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLValues());
		buf.append(", "+ _used); 
		buf.append(", "+ _channel); 
		buf.append(", '"+ _firmware+"'"); 
		buf.append(", "+ _trigger_mode); 
		for (int i = 0; i < _module_v.length; i++) {
			if (_module_v[i] != null) {
				buf.append(", '" + _module_v[i].getType()+"'");
				buf.append(", " + _module_v[i].getId());
			} else {
				buf.append(", ''");
				buf.append(", -1");
			}
		}
		return buf.toString();
	}

	@Override
	public String getTag() {
		return TAG;
	}

}
