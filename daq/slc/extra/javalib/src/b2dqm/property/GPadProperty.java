package b2dqm.property;

import b2dqm.graphics.Canvas;
import b2dqm.graphics.GPad;

public class GPadProperty extends GRectProperty {

	public final static String ELEMENT_LABEL = "pad";
	
	private GPad _pad = null;
	
	public GPadProperty(Canvas canvas) {
		super(canvas);
		_pad = canvas.getPad();
		_rect = _pad;
		_shape = _pad;
	}

	@Override
	public String getType() {
		return ELEMENT_LABEL;
	}
	
}
