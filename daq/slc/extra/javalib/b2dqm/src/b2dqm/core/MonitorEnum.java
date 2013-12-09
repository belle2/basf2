package b2dqm.core;

public abstract class MonitorEnum {

	protected int _value = 0;
	protected String _text = "";

	public int getValue() {
		return _value;
	}

	public String getText() {
		return _text;
	}

	public abstract void setValue(int value);
	
	public String toString() {
		return _text;
	}
	
	public boolean equal(MonitorEnum e) {
		return (getValue() == e.getValue());
	}
	
}
