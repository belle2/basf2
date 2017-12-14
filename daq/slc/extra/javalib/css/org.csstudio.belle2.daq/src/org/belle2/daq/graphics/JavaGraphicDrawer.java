package org.belle2.daq.graphics;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

import org.eclipse.draw2d.FigureUtilities;
import org.eclipse.draw2d.Graphics;
import org.eclipse.draw2d.geometry.Dimension;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.graphics.Font;
import org.eclipse.swt.graphics.GC;
import org.eclipse.swt.graphics.Image;
import org.eclipse.swt.graphics.Transform;
import org.eclipse.swt.widgets.Display;

public class JavaGraphicDrawer extends GraphicsDrawer {

	private Graphics _graphics;
	private Font _j_font;
	private int _font_real_size = 16;
	private Image image;
	private int m_x;
	private int m_y;
	private GC gc;
	private static Color[][][] gcolor= null;
	
	public static Color convert(HtmlColor color) {
		if (gcolor == null) {
			gcolor = new Color[256][][];
			for (int i = 0; i < 256; i++) {
				gcolor[i] = new Color[256][];
				for (int j = 0; j < 256; j++) {
					gcolor[i][j] = new Color[256];
				}
			}
		}
		if (color == null || color.isNull())
			return null;
		int r = color.getRed();
		int g = color.getGreen();
		int b = color.getBlue();
		if (gcolor[r][g][b] == null) {
			gcolor[r][g][b] = new Color(Display.getCurrent(), color.getRed(), color.getGreen(), color.getBlue());
		}
		return gcolor[r][g][b];
	}

	public JavaGraphicDrawer(int width, int height) {
		this(width, height, 120);
	}

	public JavaGraphicDrawer(int width, int height, int font_size) {
		super(width, height);
		_font = new FontProperty(HtmlColor.BLACK, "Arial", font_size, FontProperty.WEIGHT_BOLD);
		_j_font = new Font(Display.getDefault(), _font.getFamily(), 
				(int) (_font_real_size * _font.getSize()), _font.getWeight());
		setBound(width, height);
	}

	public void setBound(int width, int height) {
		super.setWidth(width);
		super.setHeight(height);
		if (image != null)
			image.dispose();
		if (gc != null)
			gc.dispose();
		image = new Image(Display.getCurrent(), width, height);
		gc = new GC(image);
	}

	public void setFont(FontProperty font) {
		_font = font;
		_j_font = new Font(Display.getDefault(), _font.getFamily(), 
				(int) (_font_real_size * _font.getSize()), _font.getWeight());
	}

	public void setFontSize(int size) {
		_font.setSize(size);
		_j_font = new Font(Display.getDefault(), _font.getFamily(), 
				(int) (_font_real_size * _font.getSize()), _font.getWeight());
	}

	public void setFontRealSize(int size) {
		_font_real_size = size;
	}

	public JavaGraphicDrawer() {
		this(100, 100, 12);
	}

	public void setGraphics(Graphics g) {
		_graphics = g;
		_graphics.setFont(_j_font);
		_graphics.setLineWidthFloat(2.0f);
	}

	public Graphics getGraphics() {
		return _graphics;
	}

	public void drawLine(double x1, double y1, double x2, double y2) {
		if (_graphics == null)
			return;
		if (getLine().getColor() != null && !getLine().getColor().isNull()) {
			gc.setForeground(convert(getLine().getColor()));
			gc.setLineWidth((int) getLine().getWidth());
			gc.drawLine((int) (x1 * getWidth()), (int) (y1 * getHeight()), (int) (x2 * getWidth()), (int) (y2 * getHeight()));
		}
	}

	public void drawPolygon(double[] x, double[] y) {
		if (_graphics == null)
			return;
		int nmax = (x.length > y.length) ? y.length : x.length;
		int[] points = new int[nmax * 2];
		for (int n = 0; n < nmax; n++) {
			points[n * 2] = (int) (x[n] * getWidth());
			points[n * 2 + 1] = (int) (y[n] * getHeight());
		}
		if (getFill().getColor() != null && !getFill().getColor().isNull()) {
			gc.setBackground(convert(getFill().getColor()));
			gc.fillPolygon(points);
		}
		if (getLine().getColor() != null && !getLine().getColor().isNull()) {
			gc.setForeground(convert(getLine().getColor()));
			gc.setLineWidth((int) getLine().getWidth());
			gc.drawPolygon(points);
		}
	}

	public void drawPolyline(double[] x, double[] y) {
		if (_graphics == null)
			return;
		int nmax = (x.length > y.length) ? y.length : x.length;
		int[] points = new int[nmax * 2];
		for (int n = 0; n < nmax; n++) {
			points[n * 2] = m_x+(int) (x[n] * getWidth());
			points[n * 2 + 1] = m_y+(int) (y[n] * getHeight());
		}
		if (getFill().getColor() != null && !getFill().getColor().isNull()) {
			gc.setBackground(convert(getFill().getColor()));
			gc.fillPolygon(points);
		}
		if (getLine().getColor() != null && !getLine().getColor().isNull()) {
			gc.setForeground(convert(getLine().getColor()));
			gc.setLineWidth((int) getLine().getWidth());
			gc.drawPolygon(points);
		}
	}

	public void drawMarker(double x, double y) {
		if (_graphics == null)
			return;
		int X = (int) (x * getWidth());
		int Y = (int) (y * getHeight());
		int R = (int) (getMarkerSize() * getWidth() / 50);
		if (getLine().getColor() != null && !getLine().getColor().isNull()) {
			gc.setForeground(convert(getLine().getColor()));
			gc.fillOval(X - R / 2, Y - R / 2, R, R);
		}
	}

	public void drawMarkers(double[] x, double[] y) {
		if (_graphics == null)
			return;
		int nmax = (x.length > y.length) ? y.length : x.length;
		for (int n = 0; n < nmax; n++) {
			drawMarker(m_x+x[n], m_y+y[n]);
		}
	}

	public void drawRect(double x, double y, double width, double height) {
		if (_graphics == null)
			return;
		int X = (int) (x * getWidth());
		int Y = (int) (y * getHeight());
		int W = (int) (width * getWidth());
		int H = (int) (height * getHeight());
		if (W == 0 && width > 0)
			W = 1;
		if (H == 0 && height > 0)
			H = 1;
		if (getFill().getColor() != null && !getFill().getColor().isNull()) {
			gc.setBackground(convert(getFill().getColor()));
			gc.fillRectangle(X, Y, W, H);
		}
		if (getLine().getColor() != null && !getLine().getColor().isNull()) {
			gc.setForeground(convert(getLine().getColor()));
			gc.setLineWidth((int) getLine().getWidth());
			gc.drawRectangle(X, Y, W, H);
		}
	}

	public void drawString(String str, double x, double y, String align) {
		drawString(str, x, y, align, 0);
	}

	public void drawString(String text, double x, double y, String align, double angle) {
		if (_graphics == null)
			return;
		if (getFont().getColor() != null && !getFont().getColor().isNull()) {
			int X = (int) (x * getWidth());
			int Y = (int) (y * getHeight());
			float Angle = (float) (angle);// * Math.PI/180.);
			Color titleColor = convert(getFont().getColor());//gc.getForeground();
			gc.setForeground(titleColor);
			gc.setFont(_j_font);
			Dimension ts = FigureUtilities.getTextExtents(text, gc.getFont());
			Transform told = new Transform(Display.getCurrent());
			gc.getTransform(told);
			Transform t = new Transform(Display.getCurrent());
			t.translate(X, Y);
			t.rotate(Angle);
			gc.setTransform(t);
			if (align.matches(".*right.*")) X = -ts.width;
			else if (align.matches(".*center.*")) X = -(int) (ts.width * 0.5);
			else X = 0;
			if (align.matches(".*top.*")) Y = 0;
			else if (align.matches(".*middle.*")) Y = -(int) (ts.height * 0.5);
			else Y = -(int) (ts.height);
			gc.drawText(text, X, Y, true);
			t.dispose();
			gc.setTransform(told);
		}
	}

	public void update() {
		super.update();
		if (_graphics != null && image != null && !image.isDisposed() && !gc.isDisposed() /* && gc != null*/) {
			try {
				_graphics.drawImage(image, m_x, m_y);
				//gc.dispose();
			} catch (Exception e) {

			}
		}
		if (gc != null) {
			gc.dispose();
			gc = new GC(image);
		}
	}

	public void drawCircle(double x, double y, double r_x, double r_y) {
		if (_graphics == null)
			return;
		if (getFill().getColor() != null && !getFill().getColor().isNull()) {
			gc.setBackground(convert(getLine().getColor()));
			gc.fillOval((int) ((x - r_x) * getWidth()), (int) ((y - r_y) * getHeight()), (int) (r_x * getWidth() * 2),
					(int) (r_y * getHeight() * 2));
		}
		if (getLine().getColor() != null && !getLine().getColor().isNull()) {
			gc.setForeground(convert(getLine().getColor()));
			gc.setLineWidth((int)getLine().getWidth());
			gc.drawOval((int) ((x - r_x) * getWidth()), (int) ((y - r_y) * getHeight()), (int) (r_x * getWidth() * 2),
					(int) (r_y * getHeight() * 2));
		}
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
	public void drawCircle(double x, double y, double r_x, double r_y, double angle0, double angle1) {
		if (_graphics == null)
			return;
		if (getFill().getColor() != null && !getFill().getColor().isNull()) {
			gc.setBackground(convert(getLine().getColor()));
			gc.fillArc((int) ((x - r_x) * getWidth()), (int) ((y - r_y) * getHeight()), (int) (r_x * getWidth() * 2),
					(int) (r_y * getHeight() * 2), (int) angle0, (int) angle1);
		}
		if (getLine().getColor() != null && !getLine().getColor().isNull()) {
			gc.setForeground(convert(getLine().getColor()));
			gc.setLineWidth((int)getLine().getWidth());
			gc.drawArc((int) ((x - r_x) * getWidth()), (int) ((y - r_y) * getHeight()), (int) (r_x * getWidth() * 2),
					(int) (r_y * getHeight() * 2), (int) angle0, (int) angle1);
		}
	}

	@Override
	public void drawRect(double x, double y, double width, double height, double addition) {
		if (_graphics == null)
			return;
		int X = (int) (x * getWidth());
		int Y = (int) (y * getHeight());
		int W = (int) (width * getWidth()+addition);
		int H = (int) (height * getHeight()+addition);
		if (W == 0 && width > 0)
			W = 1;
		if (H == 0 && height > 0)
			H = 1;
		if (getFill().getColor() != null && !getFill().getColor().isNull()) {
			gc.setBackground(convert(getFill().getColor()));
			gc.fillRectangle(X, Y, W, H);
		}
		if (getLine().getColor() != null && !getLine().getColor().isNull()) {
			gc.setForeground(convert(getLine().getColor()));
			gc.setLineWidth((int) getLine().getWidth());
			gc.drawRectangle(X, Y, W, H);
		}
	}

	public int getX() {
		return m_x;
	}

	public void setX(int x) {
		this.m_x = x;
	}

	public int getY() {
		return m_y;
	}

	public void setY(int y) {
		this.m_y = y;
	}

}
