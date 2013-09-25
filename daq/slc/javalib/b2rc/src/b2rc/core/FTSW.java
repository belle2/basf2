package b2rc.core;

public class FTSW extends Module {

	public static String TAG = "ftsw";

	private int _channel;
	private String _firmware;
	private boolean _used = true;
	private FEEModule[] _module_v = new FEEModule[16];
	private int _module_i = 0;

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

	@Override
	public String getSQLFields() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLFields());
		buf.append(", used boolean "); 
		for (int i = 0; i < _module_v.length; i++) {
			buf.append(", module_" + i + " smallint");
		}
		return buf.toString();
	}

	@Override
	public String getSQLLabels() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLLabels());
		buf.append(", used "); 
		for (int i = 0; i < _module_v.length; i++) {
			buf.append(", module_" + i);
		}
		return buf.toString();
	}

	@Override
	public String getSQLValues() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLValues());
		buf.append(", "+ _used); 
		for (int i = 0; i < _module_v.length; i++) {
			if (_module_v[i] != null) {
				buf.append(", " + _module_v[i].getId());
			} else {
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
