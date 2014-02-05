package b2dqm.core;

public class VerticalRange extends Range {

	public VerticalRange(String name, double min, double max) {
		super(name, min, max);
	}

	public VerticalRange(String name, double min) {
		super(name, min, min);
	}

	public VerticalRange() {
		super("", 0, 0);
	}

	@Override
	public String getDataType() {
		return "R1V";
	}
	
}
