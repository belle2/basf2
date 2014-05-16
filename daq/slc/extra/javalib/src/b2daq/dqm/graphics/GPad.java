package b2daq.dqm.graphics;

import b2daq.graphics.GCanvas;
import b2daq.graphics.GRect;
import b2daq.graphics.HtmlColor;

public class GPad extends GRect {

	public GPad() {}

	public GPad(double x, double y, double width, double height) {
		super(x, y, width, height);
		setFillColor(HtmlColor.WHITE);
		setLineColor(HtmlColor.BLACK);
		setLineWidth(1);
	}
	
	public void set(String key, String value) {
		if ( key.matches("title") ) {
		} else if ( key.matches("name") ) {
			_name = value;
		} else if ( key.matches("bounds") ) {
			String[] spos = value.split(",");
			if ( spos.length >= 4) {
				_x = Double.parseDouble(spos[0]);
				_y = Double.parseDouble(spos[1]);
				_height = Double.parseDouble(spos[2]);
				_width = Double.parseDouble(spos[3]);
			}
		} else if ( key.matches("line-color") ) {
			setLineColor(new HtmlColor(value));
		} else if ( key.matches("line-width") ) {
			setLineWidth(Integer.parseInt(value));
		} else if ( key.matches("fill-color") ) {
			setFillColor(new HtmlColor(value));
		} else if ( key.matches("font-color") ) {
			setFontColor(new HtmlColor(value));
		} else if ( key.matches("font-size") ) {
			setFontSize(Integer.parseInt(value));
		} else if ( key.matches("font-family") ) {
			setFontFamily(value);
		} else if ( key.matches("draw-option") ) {
		}
	}
	
	public void draw(GCanvas canvas) {
		canvas.store();
		//canvas.setLine(null);
		canvas.setFill(getFill());
		canvas.setLine(getLine());
		canvas.drawRect(_x, _y, _width, _height);
		canvas.restore();
	}
		
}
