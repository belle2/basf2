package b2dqm.graphics;

import b2daq.graphics.GCanvas;
import b2daq.graphics.HtmlColor;
import b2dqm.core.Range;

public class GBevelRange extends GHisto {

	private double _y0, _y1;

	public GBevelRange() {
		super();
		getLine().setColor(HtmlColor.CYAN);
	}
	public GBevelRange(Canvas canvas, Range range) {
		super(canvas, range);
		getLine().setColor(HtmlColor.CYAN);
	}
	
	public void draw(GCanvas canvas) {
		canvas.store();
		canvas.setLine(getLine());
		canvas.setFont(getFont());
		canvas.setFill(getFill());
		canvas.drawLine(0, _y0, 1, _y1);
		canvas.restore();
	}

	public void update() {
    	double ymin = _canvas.getAxisY().get().getMin();
    	double yscale = _canvas.getAxisY().getScale();
    	_y0 = _canvas.getPad().getY() + ( _histo.getData().get(0) - ymin ) * yscale;
    	_y1 = _canvas.getPad().getY() + ( _histo.getData().get(1) - ymin ) * yscale;
	}

}
