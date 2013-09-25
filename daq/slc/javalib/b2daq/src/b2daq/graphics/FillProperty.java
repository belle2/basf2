package b2daq.graphics;


public class FillProperty {

	private HtmlColor _color;
	private double _transparency;
	
	public FillProperty() {
		this(HtmlColor.NULL, 1);
	}
	
	public FillProperty(HtmlColor c) {
		this(c, 1);
	}
	
	public FillProperty(HtmlColor c, int t) {
		_color = c;
		_transparency = t;
	}

	public void setColor(HtmlColor c) {
		_color = c;
	}
	
	public void setTransparency(double t) {
		_transparency = t;
	}
	
	public HtmlColor getColor() {
		return _color;
	}
	
	public double getTransparency() {
		return _transparency;
	}

	public String getStyle() {
		return "";
	}

	public void setStyle(String str) {
	}
	
}
