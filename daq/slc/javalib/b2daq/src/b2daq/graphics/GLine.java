package b2daq.graphics;

public class GLine extends GShape {

	private double _x1, _y1, _x2, _y2;
	
	public GLine() {
		this(0, 0, 1, 1, HtmlColor.BLACK);
	}

	public GLine(double x1, double y1, double x2, double y2) {
		this(x1, y1, x2, y2, HtmlColor.BLACK);
	}

	public GLine(HtmlColor line_color) {
		this(0, 0, 1, 1, line_color);
	}

	public GLine(double x1, double y1, double x2, double y2, HtmlColor line_color) {
		super(null, line_color);
		_x1 = x1; _y1 = y1;
		_x2 = x2; _y2 = y2;
	}

	public void set(double x1, double y1, double x2, double y2) {
		_x1 = x1; _y1 = y1;
		_x2 = x2; _y2 = y2;
	}
	
	public void draw(GCanvas c) {
		c.store();
		c.setLine(getLine());
		c.drawLine(_x1, _y1, _x2, _y2);
		c.restore();
	}

	public double getX1() { return _x1; }
	public double getX2() { return _x2; }
	public double getY1() { return _y1; }
	public double getY2() { return _y2; }
	
}
