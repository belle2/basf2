package b2rc.core;

abstract public class SCObject {

	protected int _version = -1;
	protected int _id = -1;

	public SCObject() {
	}

	public SCObject(int id) {
		_id = id;
	}

	public SCObject(int version, int id) {
		_version = version;
		_id = id;
	}

	public int getId() {
		return _id;
	}

	public int getVersion() {
		return _version;
	}

	public void setId(int id) {
		_id = id;
	}

	public void setVersion(int version) {
		_version = version;
	}

	public void copy(SCObject msg) {
		_version = msg._version;
		_id = msg._id;
	}

	public boolean equal(SCObject object) {
		return (object._id == _id && object._version == _version);
	}

	abstract public String getTag();

	public String getSQLFields() {
		return "version int, id int";
	}

	public String getSQLLabels() {
		return "version, id";
	}

	public String getSQLValues() {
		return _version + ", " + _id;
	}

}
