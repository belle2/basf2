package b2rc.core;

public class RCHost extends Module {

	static public final String TAG = "host";

	private String _name;
	private String _type;

	public RCHost() {}
	
	public RCHost(String name, String type) {
		_name = name;
		_type = type;
	}
	
	public String getName() {
		return _name;
	}

	public String getType() {
		return _type;
	}

	public void setName(String name) {
		_name = name;
	}

	public void setType(String type) {
		_type = type;
	}

	@Override
	public String getSQLFields() {
		return super.getSQLFields() + ", hostname text, type text";
	}

	@Override
	public String getSQLLabels() {
		return super.getSQLLabels() + ", hostname, type";
	}

	@Override
	public String getSQLValues() {
		return super.getSQLValues() + ", " + _name + ", " + _type;
	}

	@Override
	public String getTag() {
		return TAG;
	}

}
