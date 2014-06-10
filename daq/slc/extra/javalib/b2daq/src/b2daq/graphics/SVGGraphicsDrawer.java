package b2daq.graphics;

import java.util.Date;
import javafx.scene.text.FontWeight;


public class SVGGraphicsDrawer extends GraphicsDrawer {

	private StringBuffer _buffer = new StringBuffer();
	
	public SVGGraphicsDrawer(int width, int height) {
		setWidth(width);
		setHeight(height);
	}
	
	@Override
	public void drawCircle(double x, double y, double r_x, double r_y) {
		x *= getWidth();
		y *= getHeight();
		r_x *= getWidth();
		r_y *= getHeight();
		String opt = "";
		if (getFill() != null ) opt += "fill=\"" + getFill().getColor().toString() + "\" ";
		if (getLine() != null ) {
			if (getLine().getColor() != null) opt += "stroke=\"" + getLine().getColor().toString() + "\" ";
			opt += "stroke-width=\"" + getLine().getWidth() + "\" ";
		}
		_buffer.append("<ellipse cx=\"" + x + "\" cy=\"" + y + 
				"\" rx=\"" + r_x + "\" ry=\"" + r_y + "\" " + opt +" />\n");
	}

	@Override
	public void drawCircle(double x, double y, double r_x, double r_y	,
			double angle0, double angle1) {
		x *= getWidth();
		y *= getHeight();
		r_x *= getWidth();
		r_y *= getHeight();
		String opt = "";
		if (getFill() != null ) opt += "fill=\"" + getFill().getColor().toString() + "\" ";
		if (getLine() != null ) {
			if (getLine().getColor() != null) opt += "stroke=\"" + getLine().getColor().toString() + "\" ";
			opt += "stroke-width=\"" + getLine().getWidth() + "\" ";
		}
		_buffer.append("<ellipse cx=\"" + x + "\" cy=\"" + y + 
				"\" rx=\"" + r_x + "\" ry=\"" + r_y + "\" " + opt +" />\n");
	}

	@Override
	public void drawLine(double x_s, double y_s, double x_e, double y_e) {
		x_s *= getWidth();
		y_s *= getHeight();
		x_e *= getWidth();
		y_e *= getHeight();
		String opt = "";
		if (getFill() != null ) opt += "fill=\"" + getFill().getColor().toString() + "\" ";
		if (getLine() != null ) {
			if (getLine().getColor() != null) opt += "stroke=\"" + getLine().getColor().toString() + "\" ";
			opt += "stroke-width=\"" + getLine().getWidth() + "\" ";
		}
		_buffer.append("<line x1=\"" + x_s + "\" y1=\"" + y_s + 
				"\" x2=\"" + x_e + "\" y2=\"" + y_e + "\" "+opt+" />\n");
	}

	@Override
	public void drawMarker(double x, double y) {
		x *= getWidth();
		y *= getHeight();
		double r = getMarkerSize()/2;
		String opt = "";
		if (getLine() != null && getLine().getColor() != null) { 
			opt += "fill=\"" + getLine().getColor().toString() + "\" ";
		}
		_buffer.append("<ellipse cx=\"" + x + "\" cy=\"" + y + 
				"\" rx=\"" + r + "\" ry=\"" + r + "\" " + opt +" />\n");
	}

	@Override
	public void drawPolygon(double[] x, double[] y) {
		drawPolygon(0, -1, x, y);
	}

	@Override
	public void drawPolygon(int begin, int end, double[] x, double[] y) {
		drawPolyline(0, -1, x, y);
	}

	@Override
	public void drawPolyline(double[] x, double[] y) {
		drawPolyline(0, -1, x, y);
	}

	@Override
	public void drawPolyline(int begin, int end, double[] x, double[] y) {
		String opt = "";
		if (getFill() != null ) {
			if ( getFill().getColor() == null || getFill().getColor() == null )
				opt += "fill=\"none\" ";
			else 
				opt += "fill=\"" + getFill().getColor().toString() + "\" ";
		}
		if (getLine() != null && getLine().getColor() != null ) {
			opt += "stroke=\"" + getLine().getColor().toString() + "\" ";
			opt += "stroke-width=\"" + getLine().getWidth() + "\" ";
		}
		String points = "";
		if ( end < begin ) end = (x.length > y.length)? y.length: x.length;
		for (int n = begin; n < end; n++) {
			points += (x[n]*getWidth())+","+(y[n]*getHeight())+" ";
		}
		_buffer.append("<polyline points=\"" + points + "\" " + opt + " />\n");
	}

	@Override
	public void drawRect(double x, double y, double width, double height) {
		x *= getWidth();
		y *= getHeight();
		width *= getWidth();
		height *= getHeight();
		String opt = "";
		if (getFill() != null ) {
			if ( getFill().getColor() == null) {
				opt += "fill=\"none\" ";
			} else { 
				opt += "fill=\"" + getFill().getColor().toString() + "\" ";
			}
		}
		if (getLine() != null ) {
			if ( getLine().getColor() != null ) {
				opt += "stroke=\"" + getLine().getColor().toString() + "\" ";
			}
			opt += "stroke-width=\"" + getLine().getWidth() + "\" ";
		}
		_buffer.append("<rect x=\"" + x + "\" y=\"" + y + 
				"\" width=\"" + width + "\" height=\"" + height + "\" "+opt+" />\n");
	}

	@Override
	public void drawString(String str, double x, double y,String align) {
		drawString(str, x, y,align, 0);
	}

	@Override
	public void drawString(String str, double x, double y, String align,
			double angle) {
		x *= getWidth();
		y *= getHeight();
		String opt = "";
		double fontsize = (getWidth()/27) *getFont().getSize();
		if (align.matches(".*right.*")) opt += "text-anchor=\"end\" "; 
		if (align.matches(".*center.*")) opt += "text-anchor=\"middle\" "; 
		if (align.matches(".*top.*")) y += fontsize*0.8; 
		if (align.matches(".*middle.*")) y += fontsize*0.4; 
		opt += "transform=\"rotate("+angle+"," + x + "," + y + ")\" ";
		opt += "font-size=\""+(int)fontsize+"\" font-family=\""+getFont().getFamily()+"\" ";
		opt += "fill=\""+getFont().getColor()+"\" ";
		if ( getFont().getWeight() == FontWeight.BOLD )
			opt += "stroke=\""+getFont().getColor()+"\" ";
		_buffer.append("<text x=\"" + x + "\" y=\"" + y + "\" "+opt+" >\n");
		_buffer.append(str + "\n");
		_buffer.append("</text>\n");
	}

	@SuppressWarnings("deprecation")
	@Override
	public String getTime(long time, String format) {
		Date date = new Date(time);
		String str = "";
		if (format.matches(".*HH:mm:ss*")) str += date.getHours()+":" + date.getMinutes() +":"+ date.getSeconds(); 
		if (format.matches(".*HH:mm*")) str += date.getHours()+":" + date.getMinutes() +":"+ date.getSeconds(); 
		return str;
	}

	@Override
	public String getTime(String format) {
		return getTime(new Date().getTime(), format);
	}

	public String toString() {
		return "<?xml version=\"1.0\" standalone=\"no\"?>"+  "\n" +
		"<svg width=\""+getWidth()+"\" height=\""+getWidth()+"\" viewBox=\"" +
		0 + " " + 0 + " " + getWidth()+ " " + getHeight() + "\" xmlns=\"http://www.w3.org/2000/svg\">\n" + 
		"<title>\n c_rop.svg\n</title>\n"+
		"<desc>\n"+
		"Creator: Konno\n"+ 
		"CreationDate: "+new Date().toString()+ "\n" +
		"</desc>\n" + 
		"<defs>\n" +
		"</defs>\n" +
		_buffer.toString() + "\n" +
		"</svg>\n";
	}
	
}
