package b2dqm.property;

import b2daq.graphics.FontProperty;
import b2daq.graphics.GObject;
import b2daq.graphics.GShape;
import b2daq.graphics.HtmlColor;
import b2dqm.core.HistoPackage;
import b2dqm.graphics.Canvas;

public abstract class GShapeProperty extends GProperty {

	protected GShape _shape = null;

	public GShapeProperty(Canvas canvas) {
		super(canvas);
	}

	@Override
	public void put(String key, String value) {
		if ( key.matches("name") ) {
			if ( value.length() > 0 ) {
				_shape.setName(value);
			}
		} else if ( key.matches("line-color") ) {
			if (value.matches("none")) _shape.setLineColor(HtmlColor.NULL);
			else _shape.setLineColor(new HtmlColor(value));
		} else if ( key.matches("line-width") ) {
			_shape.setLineWidth(Integer.parseInt(value));
		} else if ( key.matches("fill-color") ) {
			if (value.matches("none")) _shape.setFillColor(HtmlColor.NULL);
			else _shape.setFillColor(new HtmlColor(value));
		} else if ( key.matches("font-color") ) {
			if (value.matches("none")) _shape.setFontColor(HtmlColor.NULL);
			else _shape.setFontColor(new HtmlColor(value));
		} else if ( key.matches("font-size") ) {
			_shape.setFontSize(Double.parseDouble(value));
		} else if ( key.matches("font-family") ) {
			_shape.setFontFamily(value);
		} else if ( key.matches("font-weight") ) {
			if ( value.matches("normal") ) {
				_shape.setFontWeight(FontProperty.WEIGHT_NORMAL);
			} else if ( value.matches("bold") ) {
				_shape.setFontWeight(FontProperty.WEIGHT_BOLD);
			}
		} else if ( key.matches("visible") ) {
			if ( value.matches("false")) {
				_shape.setVisible(false);
			} else if ( value.matches("true")) {
				_shape.setVisible(false);
			}
		}
	}

	public GObject set(Canvas canvas, HistoPackage pack) {
		return _shape;
	}
	
}
