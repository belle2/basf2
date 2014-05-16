package b2daq.dqm.graphics;

import b2daq.dqm.core.MonArrow;
import b2daq.dqm.core.MonShape;
import b2daq.graphics.GArrow;
import b2daq.graphics.GCanvas;

public class GMonArrow extends GMonShape {

	private GArrow _arrow = new GArrow();
	private MonArrow _mon_arrow = null;
	
	public GMonArrow(Canvas canvas, MonShape shape) {
		super(canvas, shape);
		_mon_arrow = (MonArrow)shape;
		_arrow.setLineWidth(2);
	}
	
	public MonArrow get() {
		return _mon_arrow;
	}
	
	public GArrow getArrow() {
		return _arrow;
	}

	public void setPosition(double x, double y) {
		_arrow.setPosition(x, y);
	}
	
	public void setPositionX(double x) {
		_arrow.setPositionX(x);
	}
	
	public void setPositionY(double y) {
		_arrow.setPositionY(y);
	}
	
	public void draw(GCanvas c) {
		c.store();
		update();
		_arrow.draw(c);
		c.restore();
	}

	public void update() {
		_arrow.setFillColor(_mon_arrow.getFillColor());
		_arrow.setLineColor(_mon_arrow.getLineColor());
	}

}
