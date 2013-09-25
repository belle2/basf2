package b2dqm.property;

import b2daq.graphics.GObject;
import b2daq.graphics.GRect;
import b2dqm.core.HistoPackage;
import b2dqm.graphics.Canvas;

public class GRectProperty extends GShapeProperty {

	public final static String ELEMENT_LABEL = "rect";
	
	protected GRect _rect = new GRect();
	
	public GRectProperty(Canvas canvas) {
		super(canvas);
		canvas.addShape(_rect);
		_shape = _rect;
	}

	@Override
	public String getType() {
		return ELEMENT_LABEL;
	}

	@Override
	public void put(String key, String value) {
		super.put(key, value);
		if ( key.matches("x") ) {
			_rect.setX(Double.parseDouble(value));
		} else if ( key.matches("y") ) {
			_rect.setY(Double.parseDouble(value));
		} else if ( key.matches("width") ) {
			_rect.setWidth(Double.parseDouble(value));
		} else if ( key.matches("height") ) {
			_rect.setHeight(Double.parseDouble(value));
		}
	}
	
	@Override
	public void put(GProperty pro) {}

	@Override
	public GObject set(Canvas canvas, HistoPackage pack) {
		return super.set(canvas, pack);
	}

}
