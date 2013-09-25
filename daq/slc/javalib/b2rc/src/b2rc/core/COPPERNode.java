package b2rc.core;

public class COPPERNode extends RCNode {

	static public final String TAG = "copper_node";

	private HSLB[] _hslb_v = new HSLB[4];
	private DataSender _sender = new DataSender();
	private String _module_class;

	public COPPERNode() {
	}

	public COPPERNode(String name) {
		super(name);
	}

	public DataSender getSender() {
		return _sender;
	}

	public HSLB getHSLB(int slot) {
		return _hslb_v[slot];
	}

	public String getModuleClass() {
		return _module_class;
	}

	public void setHSLB(int slot, HSLB hslb) {
		_hslb_v[slot] = hslb;
	}

	public void setModuleClass(String module_class) {
		_module_class = module_class;
	}

	@Override
	public String getTag() {
		return TAG;
	}

	@Override
	public String getSQLFields() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLFields());
		for ( int i = 0; i < _hslb_v.length; i++ ) {
			buf.append(", hslb_id_" + i + " smallint");
		}
		buf.append(_sender.getSQLFields());
		return buf.toString();
	}

	@Override
	public String getSQLLabels() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLLabels());
		for ( int i = 0; i < _hslb_v.length; i++ ) {
			if ( _hslb_v[i]!= null ) {
				buf.append(", hslb_id_" + i);
			}
		}
		buf.append(_sender.getSQLLabels());
		return buf.toString();
	}

	@Override
	public String getSQLValues() {
		StringBuffer buf = new StringBuffer();
		buf.append(super.getSQLValues());
		for ( int i = 0; i < _hslb_v.length; i++ ) {
			if ( _hslb_v[i]!= null ) {
				buf.append(", " + _hslb_v[i].getId());
			} else {
				buf.append(", -1");
			}
		}
		buf.append(_sender.getSQLValues());
		return buf.toString();
	}

}
