package b2rc.core;

public class TTDNode extends RCNode {

	static public final String TAG = "ttd_node";

	private FTSW[] _ftsw_v = new FTSW[16];
	private int _ftsw_i = 0;

	public TTDNode(String name) {
		super(name);
	}

	public TTDNode() {
	}

	public FTSW getFTSW(int i) {
		return _ftsw_v[i];
	}

	public FTSW[] getFTSWs() {
		return _ftsw_v;
	}

	public void addFTSW(FTSW ftsw) {
		if (_ftsw_i < _ftsw_v.length) {
			_ftsw_v[_ftsw_i] = ftsw;
			_ftsw_i++;
		}
	}

	public void clearFTSWs() {
		for (int i = 0; i < _ftsw_v.length; i++)
			_ftsw_v[i] = null;
		_ftsw_i = 0;
	}

	@Override
	public String getSQLFields() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLFields());
		for (int i = 0; i < _ftsw_v.length; i++) {
			buf.append(", ftsw_id_" + i + " smallint");
		}
		return buf.toString();
	}

	@Override
	public String getSQLLabels() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLLabels());
		for (int i = 0; i < _ftsw_v.length; i++) {
			buf.append(", ftsw_id_" + i);
		}
		return buf.toString();
	}

	@Override
	public String getSQLValues() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLValues());
		for (int i = 0; i < _ftsw_v.length; i++) {
			if (_ftsw_v[i] != null) {
				buf.append(", " + _ftsw_v[i].getId());
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
