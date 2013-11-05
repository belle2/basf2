package b2rc.core;

public abstract class RCEnum {

	protected String _label;
	protected String _alias;
	protected int _id;
	
	protected RCEnum() {}
	
	protected RCEnum(int id, String label, String alias) {
		_id = id;
		_label = label;
		_alias = alias;
	}
	
	public String getLabel() {
		return _label;
	}
	
	public String getAlias() {
		return _alias;
	}
	
	public int getId() {
		return _id;
	}
	
	public void copy(RCEnum msg) {
		_id = msg._id;
		_label = msg._label;
		_alias = msg._alias;
	}
	
	public boolean equal(RCEnum msg) {
		return ( msg._id == _id );
	}

	public boolean equals(RCEnum msg) {
		return ( msg._id == _id );
	}
}
