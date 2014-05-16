package b2daq.graphics;

public class FontProperty {

	static public int WEIGHT_NORMAL = 0;
	static public int WEIGHT_BOLD = 1;
	
	private double _size = 1;
	private int _weight;
	private String _family;
	private HtmlColor _color = HtmlColor.BLACK;
	private HtmlColor _background_color = HtmlColor.NULL;
	
	public FontProperty() {
		this(HtmlColor.BLACK, "Helvetica", 1, WEIGHT_BOLD);
	}
	
	public FontProperty(HtmlColor color, HtmlColor bcolor, String family, double size, int weight) {
		_family = family;
		_color = color;
		_background_color = bcolor;
		_size = size;
		_weight = weight;
	}

	public FontProperty(HtmlColor color, String family, double size, int weight) {
		this(color, HtmlColor.NULL, family, size, weight);
	}

	public FontProperty(HtmlColor color, HtmlColor bcolor) {
		this(color, bcolor, "Helvetica", 1, WEIGHT_NORMAL);
	}

	public double getSize() { return _size; }
	public int getWeight() { return _weight; }
	public String getFamily() { return _family; }
	public HtmlColor getColor() { return _color; }
	public HtmlColor getBackgroundColor() { return _background_color; }
	public void setSize(double d) { _size = d; }
	public void setWeight(int weight) { _weight = weight; }
	public void setColor(HtmlColor c) { _color = c; }	
	public void setBackgroundColor(HtmlColor c) { _background_color = c; }	
	public void setFamily(String family) { _family = family; }
	
	public void copy(FontProperty pro) {
		if (pro == null) return;
		_size = pro._size;
		_weight = pro._weight;
		_family = pro._family;
		_background_color.copy(pro._background_color);
		_color.copy(pro._color);
	}
	
}
