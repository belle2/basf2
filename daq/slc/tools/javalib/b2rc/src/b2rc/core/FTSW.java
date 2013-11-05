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
	private int _trigger_mode = TRIG_NORMAL;
	private int _dummy_rate;
	private int _trigger_limit;
	
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

	public void setChannel(int channel) {
		_channel = channel;
	}

	public void setFirmware(String firmware) {
		_firmware = firmware;
	}

	public void setUsed(boolean used) {
		_used = used;
	}

	@Override
	public String getSQLFields() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLFields());
		buf.append(", used boolean, channel smallint, firmware text");
		buf.append(", trigger_mode int, dummy_rate int, trigger_limit int"); 
		return buf.toString();
	}

	@Override
	public String getSQLLabels() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLLabels());
		buf.append(", used, channel, firmware");
		buf.append(", trigger_mode, dummy_rate, trigger_limit"); 
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
		buf.append(", "+ _dummy_rate); 
		buf.append(", "+ _trigger_limit); 
		return buf.toString();
	}

	@Override
	public String getTag() {
		return TAG;
	}

	public int getTriggerMode() {
		return _trigger_mode;
	}
	
	public void setTriggerMode(int mode) {
		_trigger_mode = mode;
	}

	public void setDummyRate(int rate) {
		_dummy_rate = rate;
	}

	public void setTriggerLimit(int limit) {
		_trigger_limit = limit;
	}

	public int getDummyRate() {
		return _dummy_rate;
	}

	public int getTriggerLimit() {
		return _trigger_limit;
	}

}
