package b2daq.dqm.core;

public class MonArrow extends MonShape {

	public MonArrow(String name) {
		super(name);
	}

	public MonArrow() {
		super();
	}

	@Override
	public String getDataType() {
		return "MAW";
	}
	
	protected String getXML() {
		return ">\n";
	}

}
