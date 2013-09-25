package b2dqm.graphics;

import b2daq.graphics.GCanvas;
import b2daq.graphics.GLine;
import b2dqm.core.MonLine;
import b2dqm.core.MonShape;

public class GMonLine extends GMonShape {

	private GLine _line = new GLine();
	private MonLine _mon_line = null;
	
	public GMonLine(Canvas canvas, MonShape shape) {
		super(canvas, shape);
		_mon_line = (MonLine)shape;
		_line.setLineWidth(2);
	}
	
	public MonLine get() {
		return _mon_line;
	}
	
	public GLine getGLine() {
		return _line;
	}

	public void set(double x1, double y1, double x2, double y2) {
		_line.set(x1, y1, x2, y2);
	}
	
	public void draw(GCanvas c) {
		c.store();
		update();
		_line.draw(c);
		c.restore();
	}

	public void update() {
		_line.setFillColor(_mon_line.getFillColor());
		_line.setLineColor(_mon_line.getLineColor());
		set(_mon_line.getX1(), _mon_line.getY1(),
			_mon_line.getX2(), _mon_line.getY2());
	}

}
