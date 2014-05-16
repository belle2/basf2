package b2daq.dqm.graphics;

import b2daq.dqm.core.Axis;
import b2daq.graphics.GCanvas;
import b2daq.graphics.HtmlColor;

public class GColorAxis extends GAxisY {

	private HtmlColor [] _color_patrern = HtmlColor.ColorGrad;
	
	public GColorAxis(){ super(); }
	public GColorAxis(Axis axis){ super(axis); }
	public GColorAxis(int nbin, double min, double max, String label, 
			boolean fixMin, boolean fixMax, boolean logon){
		super(nbin, min, max, label,fixMin, fixMax, logon);
		setHand(HAND_L);
		getLabelsFont().setSize(0.90);
	}

	public void draw(GCanvas canvas) {
		if ( !isVisible() ) return;
		canvas.store();
		canvas.setFont(getLabelsFont());
		canvas.setLine(null);
		double height = _length / _color_patrern.length;
		double X = _x0 - _padding_x;
		double Y = _y0;
		double px_height = height;
		if ( height < 0.005 ) px_height = 0.005;
		for (int nz = 0; nz < _color_patrern.length; nz++){
          Y -= height;
          canvas.getFill().setColor(_color_patrern[nz]);
          canvas.drawRect(X, Y, _padding_x * 1.05, px_height * 1.05);
		}
       canvas.restore();
		super.draw(canvas);
	}

	public boolean hit(double x, double y) {
		if ( !_visible ) return false;
		if ( x >= _x0 - _padding_x && x <= _x0 + _padding_x &&
			 y <= _y0 && y >= _y0 - _length ){
			return true;
		}
		return false;
	}

	public HtmlColor[] getColorPattern() {
		return _color_patrern;
	}

	public void setColorPattern(HtmlColor[] parttern) {
		_color_patrern = parttern;
	}
	
}
