package b2dqm.graphics;

import java.util.ArrayList;

import b2daq.graphics.GCanvas;
import b2daq.graphics.GRect;
import b2daq.graphics.HtmlColor;
import b2dqm.core.Histo;


public class GLegend extends GRect {

	private String _title = "";
	private ArrayList<GHisto> _histo_v = new ArrayList<GHisto>();
	
	public GLegend() {
		_x = 0.68; _y = 0.02;
		_width = 0.26; _height = 0.08;
		setFillColor(HtmlColor.WHITE);
		setLineColor(HtmlColor.BLACK);
		_font.setSize(0.9);
	}
	
	public void setBounds(double x, double y, double width, double height){
		_x = x; _y = y;
		_width = width; _height = height;
	}

	public void add(GHisto h) {
		_histo_v.add(h);
		_height = _histo_v.size()*0.03+0.02;
	}
	
	public void setTitle(String title) {
		_title = title;
	}
	
	public String getTitle() {
		return _title;
	}
	
	public void draw(GCanvas canvas) {
		canvas.store();
		canvas.setFont(getFont());
		canvas.setFill(getFill());
		canvas.setLine(getLine());
		canvas.drawRect(_x, _y, _width, _height);
		int n = 0;
		for ( GHisto gh : _histo_v ) {
			canvas.setFill(gh.getFill());
			canvas.setLine(gh.getLine());
			if ( gh.getDrawOption().matches(".*L.*") ) {
				canvas.drawLine(_x + 0.02, _y + 0.01 * (2.5+n*3), _x + 0.06, _y + 0.01 * (2.5 + n*3));
				if ( gh.getFill() != null && gh.getFill().getColor() != null
						&& !gh.getFill().getColor().isNull() ) {
						canvas.drawRect(_x + 0.02, _y + 0.01 * (1.5+n*3), 0.055, 0.02);
				}
			}
			if ( gh.getDrawOption().matches(".*P.*") ) {
				canvas.drawMarker(_x + 0.03, _y + 0.01 * (2.5+n*3));
			}
			Histo h = (Histo)gh.get();
			canvas.drawString(h.getTitle(), _x + 0.08 , _y + 0.01 * (2.5 + n*3), "left middle");
			n++;
		}
		canvas.restore();
	}
	
	public ArrayList<GHisto> getHistos() { return _histo_v; }
	
}
