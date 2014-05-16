package b2daq.dqm.core;

public abstract class DQMEnum {

	protected String _label;
	protected String _alias;
	protected int _id;
	
	protected DQMEnum() {}
	
	protected DQMEnum(int id, String label, String alias) {
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
	
	public void copy(DQMEnum msg) {
		_id = msg._id;
		_label = msg._label;
		_alias = msg._alias;
	}
	
	public boolean equal(DQMEnum msg) {
		return ( msg._id == _id );
	}

	public boolean equals(DQMEnum msg) {
		return ( msg._id == _id );
	}
}
