package b2dqm.property;

import b2dqm.graphics.Canvas;

public class GAxisXProperty extends GAxisProperty {

	public final static String ELEMENT_LABEL = "x-axis";
	
	public GAxisXProperty(Canvas canvas) {
		super(canvas);
		_axis = canvas.getAxisX();
		_shape = _axis;
	}

	@Override
	public String getType() {
		return ELEMENT_LABEL;
	}

}
