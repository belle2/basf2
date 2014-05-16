package b2daq.dqm.core;

public class HorizontalRange extends Range {

	public HorizontalRange(String name, double min, double max) {
		super(name, min, max);
	}

	public HorizontalRange(String name, double min) {
		super(name, min, min);
	}

	public HorizontalRange() {
		super("", 0, 0);
	}

	@Override
	public String getDataType() {
		return "R1H";
	}

}
