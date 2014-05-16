package b2daq.core;

public class Enum {

	final static public Enum UNKNOWN = new Enum(0, "UNKNOWN");
	
	protected String _label;
	protected int _id;
	
	protected Enum() {}
	
	protected Enum(int id, String label) {
		_id = id;
		_label = label;
	}
	
	public String getLabel() {
		return _label;
	}
	
	public int getId() {
		return _id;
	}
	
	public void copy(Enum msg) {
		_id = msg._id;
		_label = msg._label;
	}
	
	public boolean equal(Enum msg) {
		return ( msg._id == _id );
	}

	public boolean equals(Enum msg) {
		return ( msg._id == _id );
	}
}
