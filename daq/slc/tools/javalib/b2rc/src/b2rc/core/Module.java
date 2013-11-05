package b2rc.core;

public abstract class Module extends SCObject {

	private int _product_id;
	private String _location;

	public Module() {
		_product_id = -1;
	}

	public int getProductID() {
		return _product_id;
	}

	public void setProductID(int id) {
		_product_id = id;
	}

	public String getLocation() {
		return _location;
	}

	public void setLocation(String location) {
		_location = location;
	}

	@Override
	public String getSQLFields() {
		return super.getSQLFields() + ", product_id int, location text";
	}

	@Override
	public String getSQLLabels() {
		return super.getSQLLabels() + ", product_id, location";
	}

	@Override
	public String getSQLValues() {
		return super.getSQLValues() +  ", " + _product_id + ", '" + _location + "'";
	}

};
