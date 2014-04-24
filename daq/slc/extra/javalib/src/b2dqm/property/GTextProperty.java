package b2dqm.property;

import b2daq.graphics.FontProperty;
import b2daq.graphics.GObject;
import b2daq.graphics.GText;
import b2daq.graphics.HtmlColor;
import b2dqm.core.HistoPackage;
import b2dqm.graphics.Canvas;

public class GTextProperty extends GShapeProperty {

	public final static String ELEMENT_LABEL = "text";
	
	protected GText _text = new GText();
	
	public GTextProperty(Canvas canvas) {
		super(canvas);
		canvas.addShape(_text);
		_shape = _text;
	}

	@Override
	public String getType() {
		return ELEMENT_LABEL;
	}

	@Override
	public void put(String key, String value) {
		super.put(key, value);
		if ( key.matches("x") ) {
			_text.setX(Double.parseDouble(value));
		} else if ( key.matches("y") ) {
			_text.setY(Double.parseDouble(value));
		} else if ( key.matches("value") ) {
			value = value.replaceAll("\t", "").replaceAll("\n", "");
			_text.setText(value);
		} else if ( key.matches("align") ) {
			_text.setAligment(value);
		} else if ( key.matches("font.weight") ) {
			if ( value.matches("bold") ) _text.setFontWeight(FontProperty.WEIGHT_BOLD);
			if ( value.matches("normal") ) _text.setFontWeight(FontProperty.WEIGHT_NORMAL);
		} else if ( key.matches("font.color") ) {
			_text.setFontColor(new HtmlColor(value));
		}
	}
	
	@Override
	public void put(GProperty pro) {}

	@Override
	public GObject set(Canvas canvas, HistoPackage pack) {
		return super.set(canvas, pack);
	}

}
