package b2dqm.core;

public class BevelRange extends Range {

	public BevelRange(String name, double a, double b) {
		super(name, a, b); //y = a*x + b
	}

	public BevelRange(String name, double a) {
		super(name, a, 0);//y = a*x 
	}

	public BevelRange() {
		super("", 0, 0);
	}

	@Override
	public String getDataType() {
		return "R1B";
	}

}
