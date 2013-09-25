package b2daq.graphics;

public class GArc extends GShape {

	private double _x, _y, _r_x, _r_y, _angle0, _angle1;

	public GArc() {
		this(1, 1, 1, 1);
	}

	public GArc(double x, double y, double r_x, double r_y) {
		this(x, y, r_x, r_y, 0, 0,null, HtmlColor.BLACK);
	}

	public GArc(double x, double y, double r_x, double r_y,
				HtmlColor fill_color, HtmlColor line_color,
				double angle0, double angle1) {
		super(fill_color, line_color);
		set(x, y, r_x, r_y);
		_angle0 = angle0;
		_angle1 = angle1;
	}
	
	public GArc(double x, double y, double r_x, double r_y,
			double angle0, double angle1,
			HtmlColor fill_color, HtmlColor line_color) {
		super(fill_color, line_color);
		set(x, y, r_x, r_y);
		_angle0 = angle0;
		_angle1 = angle1;
	}

	public void set(double x, double y, double r_x, double r_y) {
		_x = x; _y = y;
		_r_x = r_x;
		_r_y = r_y;
	}
	
	public void setPosition(double x, double y) {
		_x = x; _y = y;
	}
	
	public void setRX(double r_x) {
		_r_x = r_x;
	}

	public void setRY(double r_y) {
		_r_y = r_y;
	}
	
	public void setX(double x) {
		_x = x;
	}

	public void setY(double y) {
		_y = y;
	}

	public void setAngle0(double angle) {
		_angle0 = angle;
	}

	public void setAngle1(double angle) {
		_angle1 = angle;
	}

	public double getAngle0() {
		return _angle0;
	}

	public double getAngle1() {
		return _angle1;
	}

	public void draw(GCanvas c) {
		c.store();
		c.setLine(getLine());
		c.setFill(getFill());
		c.drawCircle(_x, _y, _r_x, _r_y, _angle0, _angle1);
		c.restore();
	}

	@Override
	public boolean hit(double x, double y) {
		if (_r_x == 0 || _r_y == 0) return false;
		if ( (x - _x)*(x - _x)/(_r_x*_r_x) + (y-_y)*(y-_y)/(_r_y*_r_y) < 1 ) {
			return true;
		}
		return false;
	}

	public void setAngle(double angle0, double angle1) {
		_angle0 = angle0;
		_angle1 = angle1;
	}
	
}
