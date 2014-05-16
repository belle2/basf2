package b2daq.core;

public abstract class RCEnum {

	protected String _label;
	protected int _id;
	
	protected RCEnum() {}
	
	protected RCEnum(int id, String label) {
		_id = id;
		_label = label;
	}
	
	public String getLabel() {
		return _label;
	}
	
	public int getId() {
		return _id;
	}
	
	public void copy(RCEnum msg) {
		_id = msg._id;
		_label = msg._label;
	}
	
	public boolean equal(RCEnum msg) {
		return ( msg._id == _id );
	}

	public boolean equals(RCEnum msg) {
		return ( msg._id == _id );
	}
}
