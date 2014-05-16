package b2daq.dqm.graphics;

import b2daq.dqm.core.MonShape;
import b2daq.graphics.HtmlColor;

public abstract class GMonShape extends GMonObject {

	private MonShape _shape; 
	
	public GMonShape(Canvas canvas, MonShape shape) {
		super(canvas);
		_shape = shape;
	}

	public HtmlColor getFillColor() {
		return _shape.getFillColor();
	}

	public HtmlColor getLineColor() {
		return _shape.getLineColor();
	}

}
