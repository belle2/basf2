package b2dqm.graphics;

import b2daq.graphics.GCanvas;
import b2daq.graphics.GCircle;
import b2daq.graphics.HtmlColor;
import b2dqm.core.MonCircle;

public class GMonCircle extends GMonShape {

	private GCircle _circle = new GCircle();
	private MonCircle _mon_circle = null;
	
	public GMonCircle(Canvas canvas, MonCircle obj) {
		super(canvas, obj);
		_mon_circle = obj;
		_circle.setLineWidth(2);
	}
	
	public MonCircle get() {
		return _mon_circle;
	}
	
	public GCircle getCircle() {
		return _circle;
	}

	public void setLineWidth(int width) {
		_circle.setLineWidth(width);
	}
	
	public void setLineColor(HtmlColor color) {
		_circle.setLineColor(color);
	}
	
	public void setFillColor(HtmlColor color) {
		_circle.setFillColor(color);
	}
	
	public void setBounds(double x, double y, double rx, double ry) {
		_circle.setPosition(x, y);
		_circle.setRX(rx);
		_circle.setRY(ry);
	}
	
	public void draw(GCanvas c) {
		c.store();
		update();
		_circle.draw(c);
		c.restore();
	}

	public void update() {
		_circle.setFillColor(_mon_circle.getFillColor());
		_circle.setLineColor(_mon_circle.getLineColor());
		_circle.setPosition(_mon_circle.getX(), _mon_circle.getY());
		_circle.setRX(_mon_circle.getRX());
		_circle.setRY(_mon_circle.getRY());
	}

}
