package b2daq.graphics;


public class LineProperty {
	
	private HtmlColor _color = HtmlColor.NULL;
	private int _width = 1;
	private String _style = ""; 
	
	public LineProperty() {
	}
	
	public LineProperty(HtmlColor c) {
		this(c, 1);
	}
	
	public LineProperty(HtmlColor c, int w) {
		this(c, w, "solid");
	}
	
	public LineProperty(HtmlColor c, int w, String style ) {
		_color = c;
		_width = w;
		_style = style;
	}
	
	public void setColor(HtmlColor c) {
		_color = c;
	}
	
	public void setWidth(int width) {
		_width = width;
	}
	
	public void setStyle(String s) {
		_style = s;
	}
	
	public HtmlColor getColor() {
		return _color;
	}
	
	public int getWidth() {
		return _width;
	}
	
	public String getStyle() {
		return _style;
	}
	
}
