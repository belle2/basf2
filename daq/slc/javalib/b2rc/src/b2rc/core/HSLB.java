package b2rc.core;

public class HSLB extends Module {

	static public final String TAG = "hslb";

	private int _trg_mode;
	private String _firmware;
	private FEEModule _board;
	private boolean _used = true;

	public HSLB() {
	}

	public int getTrgMode() {
		return _trg_mode;
	}

	public String getFirmware() {
		return _firmware;
	}

	public FEEModule getFEEModule() {
		return _board;
	}

	public void setTrgMode(int mode) {
		_trg_mode = mode;
	}

	public void setFirmware(String firmware) {
		_firmware = firmware;
	}

	public void setFEEModule(FEEModule board) {
		_board = board;
	}

	public boolean isUsed() {
		return _used;
	}

	public void setUsed(boolean used) {
		_used = used;
	}

	@Override
	public String getSQLFields() {
		return super.getSQLFields() + ", used boolean, firmware text, module_type text, module_id int";
	}

	@Override
	public String getSQLLabels() {
		return super.getSQLLabels() + ", used, firmware, module_type, module_id";
	}

	@Override
	public String getSQLValues() {
		return super.getSQLValues() + ", " + _used + ", '" + _firmware + "', '" + _board.getType() + "', " + _board.getId();
	}

	@Override
	public String getTag() {
		return TAG;
	}

}
