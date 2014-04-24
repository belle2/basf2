package b2daq.java.graphics;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Font;
import java.awt.FontMetrics;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.Paint;
import java.awt.RenderingHints;
import java.awt.geom.AffineTransform;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

import b2daq.graphics.FontProperty;
import b2daq.graphics.GCanvas;
import b2daq.graphics.HtmlColor;


public class JavaGCanvas extends GCanvas {
	
	static private Color [][][] __color_list = new Color [256][256][256];

	private Graphics2D _graphics;
	private Font _j_font;
	private int _font_real_size = 10;

	public static Color convert(HtmlColor color) {
		if ( color.isNull() ) return null;
		int r = color.getRed();
		int g = color.getGreen();
		int b = color.getBlue();
		Color c = __color_list[r][g][b];
		if ( c != null) return c;
		else __color_list[r][g][b] = new Color(r, g, b);
		return __color_list[r][g][b];
	}
	
	public JavaGCanvas(int width, int height) {
		this(width, height, 12);
	}

	public JavaGCanvas(int width, int height, int font_size) {
		super(width, height);
		_font = new FontProperty(HtmlColor.BLACK, "Arial", font_size, Font.BOLD);
		_j_font = new Font(_font.getFamily(), _font.getWeight(), (int)(_font_real_size*_font.getSize()));
	}
	
	public void setFont(FontProperty font) {
		_font = font;
		_j_font = new Font(_font.getFamily(), _font.getWeight(), (int)(_font_real_size*_font.getSize()));
	}
	
	public void setFontSize(int size) {
		_font.setSize(size);
		_j_font = new Font(_font.getFamily(), _font.getWeight(), (int)(_font_real_size*_font.getSize()));
	}

	public void setFontRealSize(int size) {
		_font_real_size = size;
	}
	
	public JavaGCanvas(){
		this(100, 100, 12);
	}
	
	public void setGraphics(Graphics g){
		_graphics = (Graphics2D)g;
		_graphics.setRenderingHint(RenderingHints.KEY_TEXT_ANTIALIASING, 
					RenderingHints.VALUE_TEXT_ANTIALIAS_ON);
		_graphics.setFont(_j_font);
		_graphics.setStroke(new BasicStroke(2.0f));
	}

	public Graphics getGraphics(){
		return _graphics;
	}
	
	public void drawLine(double x1, double y1, double x2, double y2) {
		Paint paint = _graphics.getPaint();
		if ( getLine().getColor() != null && !getLine().getColor().isNull() ) {
			_graphics.setPaint(convert(getLine().getColor()));
			_graphics.setStroke(new BasicStroke(getLine().getWidth()));
			_graphics.drawLine((int)(x1*getWidth()), (int)(y1*getHeight()),
					(int)(x2*getWidth()), (int)(y2*getHeight()));
		}
		_graphics.setPaint(paint);
	}
	
	public void drawPolygon(double[] x, double[] y) {
		drawPolygon(0, -1, x, y);
	}
	
	public void drawPolygon(int begin, int end, double[] x, double[] y) {
		drawPolyline(begin, end, x, y);
	}

	public void drawPolyline(double[] x, double[] y) {
		drawPolyline(0, -1, x, y);
	}
	
	public void drawPolyline(int begin, int end, double[] x, double[] y) {
		if ( end < begin ) end = (x.length > y.length)? y.length: x.length;
		int npoints = end - begin;
		int[] X = new int [npoints]; 
		int[] Y = new int [npoints];
		for ( int n = 0; n < npoints; n++) {
			X[n] = (int)(x[n+begin]*getWidth());
			Y[n] = (int)(y[n+begin]*getHeight());
		}
		Paint paint = _graphics.getPaint();
		if ( getFill().getColor() != null && !getFill().getColor().isNull()  ) {
			_graphics.setPaint(convert(getFill().getColor()));
			_graphics.fillPolygon(X, Y, npoints);
		}
		if ( getLine().getColor() != null && !getLine().getColor().isNull() ) {
			_graphics.setPaint(convert(getLine().getColor()));
			_graphics.setStroke(new BasicStroke(getLine().getWidth()));
			_graphics.drawPolygon(X, Y, npoints);
		}
		_graphics.setPaint(paint);
	}

	public void drawMarker(double x, double y) {
		int X = (int)(x*getWidth());
		int Y = (int)(y*getHeight());
		int R = (int)(getMarkerSize()*getWidth()/50);
		Paint paint = _graphics.getPaint();
		if ( getLine().getColor() != null && !getLine().getColor().isNull() ) {
			_graphics.setPaint(convert(getLine().getColor()));
			_graphics.fillOval(X - R/2, Y - R/2, R, R);
		}
		_graphics.setPaint(paint);
	}

	public void drawRect(double x, double y, double width, double height) {
		Paint paint = _graphics.getPaint();
		int X = (int)(x*getWidth());
		int Y = (int)(y*getHeight());
		int W = (int)(width*getWidth());
		int H = (int)(height*getHeight());
		if ( W == 0 && width > 0 ) W = 1;
		if ( H == 0 && height > 0 ) H = 1;
		if ( getFill().getColor() != null && !getFill().getColor().isNull() ) {
			_graphics.setPaint(convert(getFill().getColor()));
			_graphics.fillRect(X, Y, W, H);
		}
		if ( getLine().getColor() != null && !getLine().getColor().isNull() ) {
			_graphics.setPaint(convert(getLine().getColor()));
			_graphics.setStroke(new BasicStroke(getLine().getWidth()));
			_graphics.drawRect(X, Y, W, H);
		}
		_graphics.setPaint(paint);
	}
	
	public void drawString(String str, double x, double y, String align) {
		drawString(str, x, y, align, 0);
	}

	public void drawString(String str, double x, double y, String align, double angle) {
		if ( getFont().getColor() != null && !getFont().getColor().isNull() ) {
			Paint paint = _graphics.getPaint();
			AffineTransform t = _graphics.getTransform();
			_graphics.setColor(convert(getFont().getColor()));
			Font f = _graphics.getFont();
			_graphics.setFont(_j_font);
			FontMetrics	fm = _graphics.getFontMetrics();
			int X = (int)(x * getWidth()); 
			int Y = (int)(y * getHeight()); 
			_graphics.rotate(angle * Math.PI/180., X, Y);
			if (align.matches(".*right.*")) X -= fm.stringWidth(str); 
			if (align.matches(".*center.*")) X -= fm.stringWidth(str)*0.5; 
			if (align.matches(".*top.*")) Y += fm.getHeight()*0.6; 
			if (align.matches(".*middle.*")) Y += fm.getHeight()*0.3; 
			_graphics.drawString(str, X, Y);
			_graphics.setFont(f);
			_graphics.setTransform(t);
			_graphics.setPaint(paint);
		}
	}

	public void drawCircle(double x, double y, double r_x, double r_y) {
		Paint paint = _graphics.getPaint();
		if ( getFill().getColor() != null && !getFill().getColor().isNull() ) {
			_graphics.setPaint(convert(getFill().getColor()));
			_graphics.fillOval((int)((x - r_x)*getWidth()),
					(int)((y - r_y)*getHeight()), 
					(int)(r_x*getWidth()*2), (int)(r_y*getHeight()*2));
		}
		if ( getLine().getColor() != null && !getLine().getColor().isNull()) {
			_graphics.setPaint(convert(getLine().getColor()));
			_graphics.setStroke(new BasicStroke(getLine().getWidth()));
			_graphics.drawOval((int)((x - r_x)*getWidth()),
					(int)((y - r_y)*getHeight()), 
					(int)(r_x*getWidth()*2), (int)(r_y*getHeight()*2));
		}
		_graphics.setPaint(paint);
	}
	
	public String getTime(long time, String format) {
		DateFormat df = new SimpleDateFormat(format);
		return df.format(new Date(time));
	}

	public String getTime(String format) {
		DateFormat df = new SimpleDateFormat(format);
		return df.format(new Date());
	}

	@Override
	public void drawCircle(double x, double y, double r_x, double r_y,
			double angle0, double angle1) {
		Paint paint = _graphics.getPaint();
		if ( getFill().getColor() != null && !getFill().getColor().isNull() ) {
			_graphics.setPaint(convert(getFill().getColor()));
			_graphics.fillArc((int)((x - r_x)*getWidth()),
					(int)((y - r_y)*getHeight()), 
					(int)(r_x*getWidth()*2), (int)(r_y*getHeight()*2),
					(int)angle0, (int)angle1);
		}
		if ( getLine().getColor() != null && !getLine().getColor().isNull()) {
			_graphics.setPaint(convert(getLine().getColor()));
			_graphics.setStroke(new BasicStroke(getLine().getWidth()));
			_graphics.drawArc((int)((x - r_x)*getWidth()),
					(int)((y - r_y)*getHeight()), 
					(int)(r_x*getWidth()*2), (int)(r_y*getHeight()*2),
			(int)angle0, (int)angle1);
		}
		_graphics.setPaint(paint);
	}

}
