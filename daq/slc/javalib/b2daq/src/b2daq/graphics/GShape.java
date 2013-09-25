package b2daq.graphics;

public abstract class GShape implements GObject {
	
	protected String _name = "";
	protected FillProperty _fill_property = new FillProperty();
	protected LineProperty _line_property = new LineProperty();
	protected FontProperty _font = new FontProperty();
	protected boolean _focused = false;
	protected boolean _visible = true;
	
	public GShape() {}
	
	public GShape(HtmlColor fill_color, HtmlColor line_color) {
		_fill_property.setColor(fill_color);
		_line_property.setColor(line_color);
	}
	
	public void setFill(FillProperty fill) {
		_fill_property = fill;
	}
	
	public void setFillColor(HtmlColor color) {
		if ( _fill_property != null ) {
			_fill_property.setColor(color);
		}
	}

	public void setFillTransparency(double t) {
		if ( _fill_property != null ) {
			_fill_property.setTransparency(t);
		}
	}
	
	public FillProperty getFill() {
		return _fill_property;
	}
	
	public HtmlColor getFillColor() {
		if ( _fill_property == null ) return null; 
		else return _fill_property.getColor();
	}

	public double getFillTransparency() {
		if ( _fill_property == null ) return 0;
		else return _fill_property.getTransparency();
	}
	
	public void setLine(LineProperty line) {
		_line_property = line;
	}
	
	public void setLineColor(HtmlColor color) {
		if ( _line_property != null ) {
			_line_property.setColor(color);
		}
	}
	
	public void setLineWidth(int width) {
		if ( _line_property != null ) {
			_line_property.setWidth(width);
		}
	}
	
	public void setLineStyle(String style) {
		if ( _line_property != null ) {
			_line_property.setStyle(style);
		}
	}
	
	public LineProperty getLine() {
		return _line_property;
	}
	
	public HtmlColor getLineColor() {
		if ( _line_property == null ) return null; 
		else return _line_property.getColor();
	}

	public int getLineWidth() {
		if ( _line_property == null ) return 0; 
		else return _line_property.getWidth();
	}

	public String getLineStyle() {
		if ( _line_property == null ) return ""; 
		else return _line_property.getStyle();
	}

	public FontProperty getFont() {
		return _font;
	}

	public HtmlColor getFontColor() {
		return _font.getColor();
	}

	public double getFontSize() {
		return _font.getSize();
	}

	public String getFontFamily() {
		return _font.getFamily();
	}

	public int getWeight() {
		return _font.getWeight();
	}

	public void setFont(FontProperty font) {
		_font = font;
	}

	public void setFontColor(HtmlColor color) {
		_font.setColor(color);
	}

	public void setFontSize(double d) {
		_font.setSize(d);
	}

	public void setFontFamily(String family) {
		_font.setFamily(family);
	}

	public void setFontWeight(int weight) {
		_font.setWeight(weight);
	}

	public void setName(String name) {
		_name = name;
	}

	public String getName() {
		return _name;
	}
	
	public boolean hit(double x, double y) {
		return false;
	}

	public void setFocused(boolean focused) {
		_focused = focused;
	}

	public boolean isFocused() {
		return _focused;
	}
	
	public void setVisible(boolean visible) {
		_visible = visible;
	}

	public boolean isVisible() {
		return _visible;
	}
	
	public String getMessage(double x, double y) {
		return "";
	}
	
}
