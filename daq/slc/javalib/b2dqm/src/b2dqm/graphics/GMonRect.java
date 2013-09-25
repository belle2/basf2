package b2dqm.graphics;

import b2daq.graphics.GCanvas;
import b2daq.graphics.GRect;
import b2dqm.core.MonRect;
import b2dqm.core.MonShape;

public class GMonRect extends GMonShape {

	private GRect _rect = new GRect();
	private MonRect _mon_rect = null;
	
	public GMonRect(Canvas canvas, MonShape shape) {
		super(canvas, shape);
		_mon_rect = (MonRect)shape;
		_rect.setLineWidth(2);
	}
	
	public MonRect get() {
		return _mon_rect;
	}
	
	public GRect getRect() {
		return _rect;
	}

	public void setBounds(double x, double y, double w, double h) {
		_rect.setPosition(x, y);
		_rect.setWidth(w);
		_rect.setHeight(h);
	}
	
	public void draw(GCanvas c) {
		c.store();
		update();
		_rect.draw(c);
		c.restore();
	}

	public void update() {
		_rect.setFillColor(_mon_rect.getFillColor());
		_rect.setLineColor(_mon_rect.getLineColor());
		_rect.setPosition(_mon_rect.getX(), _mon_rect.getY());
		_rect.setWidth(_mon_rect.getWidth());
		_rect.setHeight(_mon_rect.getHeight());
	}

}
