package b2dqm.graphics;

import b2daq.graphics.GCanvas;
import b2daq.graphics.GRect;
import b2daq.graphics.GShape;
import b2daq.graphics.GText;
import b2daq.graphics.HtmlColor;

public class GEnumLabel extends GShape {

	protected GRect _rect = new GRect();
	protected GText _text = new GText();
	protected double _position_ratio;

	public GEnumLabel() {
		_text.setFontSize(1.75);
		_text.setAligment("center middle");
		_rect.setLineWidth(2);
		_position_ratio = 0.5;
	}

	public void setBounds(double x, double y, double w, double h) {
		_text.setPosition(x + w * _position_ratio, y + h * 0.5);
		_rect.setPosition(x, y);
		_rect.setWidth(w);
		_rect.setHeight(h);
	}

	public HtmlColor getFontColor() {
		return _text.getFontColor();
	}
	
	public void setFontColor(HtmlColor color) {
		_text.setFontColor(color);
	}
	
	public void setFontSize(double size) {
		_text.setFontSize(size);
	}
	
	public GRect getRect() {
		return _rect;
	}

	public GText getText() {
		return _text;
	}
	
	public void setTextAlign(String align) {
		if (align.matches("left")) {
			_position_ratio = 0.02;
		} else if (align.matches("rigtht")) {
			_position_ratio = 0.98;
		} else {
			_position_ratio = 0.5;
		}
		if ( !align.matches("top") || !align.matches("bottom")) {
			align += " middle";
		}
		_text.setAligment(align);
		setBounds(_rect.getX(), _rect.getY(), 
				 _rect.getWidth(), _rect.getHeight());
	}
	
	@Override
	public void draw(GCanvas c) {
		_rect.draw(c);
		_text.draw(c);
	}

}
