package b2daq.dqm.graphics;

import b2daq.dqm.core.Graph1;
import b2daq.graphics.GCanvas;

public class GGraph1 extends GHisto {

	public GGraph1() {
		super();
	}
	
	public GGraph1(Canvas canvas, Graph1 graph) {
		super(canvas, graph);
    	_point_x = new double [_histo.getAxisX().getNbins()];
    	_point_y = new double [_histo.getAxisX().getNbins()];
	}

	public void update() {
		super.update();
		double x0 = _canvas.getPad().getX();
    	double y0 = _canvas.getPad().getY() + _canvas.getPad().getHeight();;

    	if ( _histo.getAxisX().getNbins() > _point_x.length ) {
    		_point_x = new double [_histo.getAxisX().getNbins()];
    		_point_y = new double [_histo.getAxisX().getNbins()];
    	}
    	for ( int n = 0; n < _point_x.length && n < _point_y.length ; n++ ) {
			_point_x[n] = x0 + _canvas.getAxisX().eval( _histo.getPointX(n) );
			_point_y[n] = y0 - _canvas.getAxisY().eval( _histo.getPointY(n) );
		}
	}
	
	public void draw(GCanvas canvas) {
		update();
		canvas.store();
		canvas.setLine(getLine());
		canvas.setFill(getFill());
		if (getDrawOption().matches(".*P.*")) canvas.drawMarkers(_point_x, _point_y);
		if (getDrawOption().matches(".*L.*")) canvas.drawPolyline(_point_x, _point_y);
		canvas.restore();
	}

}
