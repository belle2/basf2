package org.belle2.daq.dqm.graphics;

import org.belle2.daq.graphics.GText;
import org.belle2.daq.graphics.HtmlColor;

public class GValuedText extends GText {

	private double value = 0;

	public GValuedText() {
	}

	public GValuedText(String str, double value) {
		super(str, 0, 0, "left top");
		this.value = value;
	}

	public GValuedText(String str, double value, String align) {
		super(str, 0, 0, align);
		this.value = value;
	}

	public GValuedText(String str, double value, String align, HtmlColor font_color) {
		super(str, 0, 0, align, font_color);
		this.value = value;
	}

	public GValuedText(String str, double value, String align, double angle, HtmlColor font_color) {
		super(str, 0, 0, align, angle, font_color);
		this.value = value;
	}

	public void setValue(double value) {
		this.value = value;
	}

	public double getValue() {
		return value;
	}

}
