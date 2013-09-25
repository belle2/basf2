package b2dqm.graphics;

import b2daq.graphics.GText;
import b2daq.graphics.HtmlColor;

public class GValuedText extends GText {

	private double _value = 0;
	
	public GValuedText() {}

	public GValuedText(String str, double value) {
		super(str, 0, 0, "left top");
		_value = value;
	}
	public GValuedText(String str, double value, String align) {
		super(str, 0, 0, align);
		_value = value;
	}

	public GValuedText(String str, double value, String align,
			HtmlColor font_color) {
		super(str, 0, 0, align, font_color);
		_value = value;
	}

	public GValuedText(String str, double value, String align, 
				double angle, HtmlColor font_color) {
		super(str, 0, 0, align, angle, font_color);
		_value = value;
	}

	public void setValue(double value) {
		_value = value;
	}
	
	public double getValue() {
		return _value;
	}
	
}
