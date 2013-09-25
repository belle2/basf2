package b2dqm.graphics;

import b2daq.graphics.GCanvas;
import b2daq.graphics.HtmlColor;
import b2dqm.core.Range;

public class GHorizontalRange extends GHisto {

	private double _y0;
	private double _y1;

	public GHorizontalRange() {
		super();
		getLine().setColor(HtmlColor.CYAN);
	}
	public GHorizontalRange(Canvas canvas, Range range) {
		super(canvas, range);
		getLine().setColor(HtmlColor.CYAN);
	}
	
	public void draw(GCanvas canvas) {
		update();
		canvas.store();
		canvas.setLine(getLine());
		canvas.setFont(getFont());
		canvas.setFill(getFill());
		canvas.drawLine(0,_y0, 1, _y0);
		canvas.drawLine(0,_y1, 1, _y1);
		canvas.restore();
	}
	
	public void update() {
    	double ymin = _canvas.getAxisY().get().getMin();
    	double yscale = _canvas.getAxisY().getScale();
    	_y0 = _canvas.getPad().getY() + ( _histo.getData().get(0) - ymin ) * yscale;
    	_y1 = _canvas.getPad().getY() + ( _histo.getData().get(1) - ymin ) * yscale;
	}

}
