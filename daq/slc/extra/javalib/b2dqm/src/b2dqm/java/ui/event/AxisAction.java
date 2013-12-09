package b2dqm.java.ui.event;

import b2dqm.graphics.GAxis;

public abstract class AxisAction {

	private GAxis _axis = null;

	public AxisAction(GAxis axis) {
		_axis = axis;
	}

	protected GAxis get() {
		return _axis;
	}

}
