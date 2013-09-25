package b2dqm.property;

import b2dqm.graphics.Canvas;

public class GAxisYProperty extends GAxisProperty {

	public final static String ELEMENT_LABEL = "y-axis";
	
	public GAxisYProperty(Canvas canvas) {
		super(canvas);
		_axis = canvas.getAxisY();
		_shape = _axis;
	}

	@Override
	public String getType() {
		return ELEMENT_LABEL;
	}

}
