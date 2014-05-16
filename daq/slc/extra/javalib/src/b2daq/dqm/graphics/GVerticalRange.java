package b2daq.dqm.graphics;

import b2daq.dqm.core.Range;
import b2daq.graphics.GCanvas;
import b2daq.graphics.HtmlColor;

public class GVerticalRange extends GHisto {

	private double _x0, _x1;

	public GVerticalRange() {
		super();
		getLine().setColor(HtmlColor.CYAN);
	}
	public GVerticalRange(Canvas canvas, Range range) {
		super(canvas, range);
		getLine().setColor(HtmlColor.CYAN);
	}
	
	public void draw(GCanvas canvas) {
		canvas.store();
		canvas.setLine(getLine());
		canvas.setFont(getFont());
		canvas.setFill(getFill());
		canvas.drawLine(_x0, 0, _x0, 1);
		canvas.drawLine(_x1, 0, _x1, 1);
		canvas.restore();
	}
	
	public void update() {
    	double xmin = _canvas.getAxisX().get().getMin();
    	double xscale = _canvas.getAxisX().getScale();
    	_x0 = _canvas.getPad().getX() + ( _histo.getData().get(0) - xmin ) * xscale;
    	_x1 = _canvas.getPad().getX() + ( _histo.getData().get(1) - xmin ) * xscale;
	}

}
