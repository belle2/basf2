package b2rc.java.ui2;

public class RunParam {

	private String _att = "text";
	private String _value = "";
	
	public RunParam() {
	}

	public RunParam(String att, String value) {
		_att = att;
		_value = value;
	}

	public String getAttName() {
		return _att;
	}
	
	public String getValue() {
		return _value;
	}
	
	public int getValueInt() {
		try {
			return Integer.parseInt(_value);
		} catch (Exception e) {
			return 0;
		}
	}
	
	public double getValueFloat() {
		try {
			return Double.parseDouble(_value);
		} catch (Exception e) {
			return 0;
		}
	}

	public void setValue(String value) {
		_value = value;
	}
	
}
