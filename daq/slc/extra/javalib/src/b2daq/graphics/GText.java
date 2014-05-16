package b2daq.graphics;

public class GText extends GShape {

	private String _str;
	private String _align;
	private double _angle = 0;
	private double _x, _y;
	
	public GText() {
		this("", 0, 0, "left");
	}

	public GText(String str,double x, double y, String align) {
		this(str, x, y, align, HtmlColor.BLACK);
	}

	public GText(String str, double x, double y, String align, HtmlColor font_color) {
		set(str, x, y);
		_align = align;
		setFontColor(font_color);
	}
	
	public GText(String str, double x, double y, String align, double angle, HtmlColor font_color) {
		set(str, x, y);
		_align = align;
		_angle = angle;
		setFontColor(font_color);
	}
	
	public void setText(String str) {
		_str = str;
	}
	public String getText() {
		return _str;
	}
	public void set(String str, double x, double y) {
		_str = str;
		_x = x; _y = y;
	}
	
	public void setAngle(double angle) {
		_angle = angle;
	}
	
	public void setAligment(String align) {
		_align = align;
	}
	
	public void setPosition(double x, double y) {
		_x = x; _y = y;
	}
	
	public void setX(double x) {
		_x = x;
	}
	
	public void setY(double y) {
		_y = y;
	}
	
	public void draw(GCanvas c) {
		c.setFont(getFont());
		if (_angle == 0 )c.drawString(_str, _x, _y, _align);
		else c.drawString(_str, _x, _y, _align, _angle);
	}


}
