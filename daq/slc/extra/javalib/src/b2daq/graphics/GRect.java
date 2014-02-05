package b2daq.graphics;

public class GRect extends GShape {

	protected double _x, _y, _width, _height;

	public GRect() {
		this(1, 1, 1, 1);
	}

	public GRect(double x, double y, double width, double height) {
		this(x, y, width, height, null, HtmlColor.BLACK);
	}

	public GRect(double x, double y, double width, double height,
				HtmlColor fill_color, HtmlColor line_color) {
		super(fill_color, line_color);
		set(x, y, width, height);
	}
	
	public void set(double x, double y, double width, double height) {
		_x = x; _y = y;
		_width = width;
		_height = height;
	}
	
	public void setPosition(double x, double y) {
		_x = x; _y = y;
	}
	
	public void setBounds(double width, double height) {
		_width = width;
		_height = height;
	}
	
	public void draw(GCanvas c) {
		c.store();
		c.setLine(getLine());
		c.setFill(getFill());
		c.drawRect(_x, _y, _width, _height);
		c.restore();
	}

	@Override
	public boolean hit(double x, double y) {
		if ( x >= _x && x <= _x + _width && y >= _y && y <= _y + _height ) {
			return true;
		}
		return false;
	}
	
	public void setWidth(double w) { _width = w; }
	public void setHeight(double h) { _height = h; }
	public void setX(double x) { _x = x; }
	public void setY(double y) { _y = y; }
	public double getWidth() { return _width; }
	public double getHeight() { return _height; }
	public double getX() { return _x; }
	public double getY() { return _y; }

}
