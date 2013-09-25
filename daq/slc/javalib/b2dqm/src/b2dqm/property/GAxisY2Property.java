package b2dqm.property;

import b2dqm.graphics.Canvas;
import b2dqm.graphics.GAxisY;

public class GAxisY2Property extends GAxisProperty {

	public final static String ELEMENT_LABEL = "y2-axis";

	public GAxisY2Property(Canvas canvas) {
		super(canvas);
		_axis = new GAxisY();
		canvas.setAxisY2(_axis);
		_shape = _axis;
	}

	@Override
	public String getType() {
		return ELEMENT_LABEL;
	}

}
