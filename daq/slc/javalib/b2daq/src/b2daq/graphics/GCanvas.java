package b2daq.graphics;

import java.util.ArrayList;
import java.util.Iterator;

public abstract class GCanvas {

	protected int _width = 1;
	protected int _height = 1;
	protected LineProperty _line = new LineProperty();
	protected FillProperty _fill= new FillProperty();
	protected FontProperty _font = new FontProperty();
	private LineProperty _line_copy;
	private FillProperty _fill_copy;
	private FontProperty _font_copy;
	private int _marker_size = 1;
	
	private ArrayList<GShape> _shape_v = new ArrayList<GShape>();
	
	abstract public void drawLine(double x_s, double y_s, double x_e, double y_e);
	abstract public void drawPolygon(double[] x, double[] y);
	abstract public void drawPolyline(double[] x, double[] y);
	abstract public void drawPolygon(int begin, int end, double[] x, double[] y);
	abstract public void drawPolyline(int begin, int end, double[] x, double[] y);
	abstract public void drawRect(double x_begin, double y_begin, double _width, double _height);	
	abstract public void drawString(String str, double x_begin, double y_begin, String align);
	abstract public void drawString(String str, double x, double y, String align, double angle);
	abstract public void drawMarker(double x, double y);
	public void drawMarkers(double[] x, double[] y) {
		drawMarkers(0, (x.length > y.length)? y.length: x.length, x, y);
	}

	public void drawMarkers(int begin, int end, double[] x, double[] y) {
		if ( end < begin ) end = (x.length > y.length)? y.length: x.length;
		for ( int n = begin; n < end; n++) {
			drawMarker(x[n], y[n]);
		}
	}

	abstract public void drawCircle(double x, double y, double r_x, double r_y);
	abstract public void drawCircle(double x, double y, double r_x, double r_y, double angle0, double angle1);
	abstract public String getTime(long time, String format);
	abstract public String getTime(String format);
	
	public GCanvas() {}
	
	public GCanvas(int width, int height) {
		_width = width;
		_height = height;
	}
	
	public int getWidth() { return _width; }
	public void setWidth(int width) { _width = width; }
	public int getHeight() { return _height; }
	public void setHeight(int height) { _height = height; }
	
	public void setLine(LineProperty line) { _line = line; } 
	public void setFill(FillProperty fill) { _fill = fill; } 
	public void setFont(FontProperty font) { _font = font; } 
	public void setFontSize(int size) { _font.setSize(size); } 
	public double getFontSize() { return _font.getSize(); } 

	public LineProperty getLine() {
		if ( _line == null ) 
			_line = new LineProperty();
		return _line;
	} 
	
	public FillProperty getFill() {
		if ( _fill == null ) 
			_fill = new FillProperty();
		return _fill;
	} 
	
	public FontProperty getFont() {
		if ( _font == null ) 
			_font = new FontProperty();
		return _font;
	} 

	public void restore() {
		_line = _line_copy;
		_fill = _fill_copy;
		_font = _font_copy;
	}
	
	public void store() {
		_line_copy = _line;
		_fill_copy = _fill;
		_font_copy = _font;
	}

	public void add(GShape shape) {
		_shape_v.add(shape);
	}
	
	public void remove(GShape shape) {
		_shape_v.remove(shape);
	}
	
	public void update() {
		Iterator<GShape> shape_it = _shape_v.iterator();
		while ( shape_it.hasNext() ) {
			GShape shape = shape_it.next();
			if ( shape != null ) {
				shape.draw(this);
			}
		}
	}
	public void setMarkerSize(int marker_size) {
		_marker_size = marker_size;
	}
	
	public int getMarkerSize() {
		return _marker_size;
	}

}
