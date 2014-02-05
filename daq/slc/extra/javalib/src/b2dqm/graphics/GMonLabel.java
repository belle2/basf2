package b2dqm.graphics;

import b2daq.graphics.GCanvas;
import b2daq.graphics.GRect;
import b2daq.graphics.GText;
import b2daq.graphics.HtmlColor;
import b2dqm.core.MonLabel;
import b2dqm.core.MonShape;

public class GMonLabel extends GMonShape {

	private GRect _rect = new GRect();
	private GText _text = new GText();
	private MonLabel _label = null;
	private double _position_ratio;
	
	public GMonLabel(Canvas canvas, MonShape shape) {
		super(canvas, shape);
		_label = (MonLabel)shape;
		_text.setAligment("center middle");
		_rect.setLineWidth(2);
		_rect.setLineColor(HtmlColor.NULL);
		_position_ratio = 0.5;
	}
	
	public MonLabel get() {
		return _label;
	}
	
	public HtmlColor getFontColor() {
		return _label.getFontColor();
	}
	
	public void setFontColor(HtmlColor color) {
		_label.setFontColor(color);
	}
	
	public GRect getRect() {
		return _rect;
	}

	public GText getText() {
		return _text;
	}

	public void setBounds(double x, double y, double w, double h) {
		_text.setPosition(x + w * _position_ratio, y + h * 0.5);
		_rect.setPosition(x, y);
		_rect.setWidth(w);
		_rect.setHeight(h);
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
	
	public void draw(GCanvas c) {
		c.store();
		update();
		_rect.draw(c);
		_text.draw(c);
		c.restore();
	}

	public void update() {
		_rect.setFillColor(_label.getFillColor());
		_rect.setLineColor(_label.getLineColor());
		_text.setText(_label.getText());
		_text.setFontColor(_label.getFontColor());
	}

}
