package b2daq.graphics;

public class GPolygon extends GShape {

	private double[] _x, _y;

	public GPolygon() {
	}

	public GPolygon(double[] x, double[] y) {
		this(x, y, null, HtmlColor.BLACK);
	}

	public GPolygon(double[] x, double[] y, HtmlColor fill_color, HtmlColor line_color) {
		super(fill_color, line_color);
		set(x, y);
	}
	
	public void set(double[] x, double[] y) {
		_x = x; _y = y;
	}
	
	public void draw(GCanvas c) {
		if ( _x.length > 0 && _y.length > 0 ) {
			c.store();
			c.setLine(getLine());
			c.setFill(getFill());
			c.drawPolygon(_x, _y);
			c.restore();
		}
	}

}
