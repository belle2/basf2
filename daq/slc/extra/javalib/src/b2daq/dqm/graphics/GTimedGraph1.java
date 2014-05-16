package b2daq.dqm.graphics;

import b2daq.dqm.core.TimedGraph1;
import b2daq.graphics.GCanvas;

public class GTimedGraph1 extends GHisto {

	public GTimedGraph1(){
		super();
		setDrawOption("AP");
	}
	
	public GTimedGraph1(Canvas canvas, TimedGraph1 graph){
		super(canvas, graph);
		setDrawOption("AP");
		_point_x = new double [graph.getAxisX().getNbins()+2];
		_point_y = new double [graph.getAxisX().getNbins()+2];
	}

	public void update() {
		super.update();
    	double x0 = _canvas.getPad().getX() + _canvas.getPad().getWidth();
    	double y0 = _canvas.getPad().getY() + _canvas.getPad().getHeight();
    	TimedGraph1 g = (TimedGraph1)_histo;
		if ( _point_x.length != g.getAxisX().getNbins() + 2) {
			_point_x = new double [g.getAxisX().getNbins() + 2];
			_point_y = new double [g.getAxisX().getNbins() + 2];
		}
		double min = g.getPoint(0);
		double max = g.getPoint(0);
		for ( int n = 0; n < g.getAxisX().getNbins(); n++ ) {
			if ( max < g.getPoint(n) ) max = g.getPoint(n);
			if ( min > g.getPoint(n) ) min = g.getPoint(n);
		}
		if ( !g.getAxisY().isFixMin() ) g.getAxisY().setMin((max+min)/2.-(max-min)/2.*1.05);
		if ( !g.getAxisY().isFixMax() ) g.getAxisY().setMax((max+min)/2.+(max-min)/2.*1.05);
		if ( min == max && max == 0) {
			if ( !g.getAxisY().isFixMin() ) g.getAxisY().setMin(0);
			if ( !g.getAxisY().isFixMax() ) g.getAxisY().setMax(1);
		}
		long t0 = g.getUpdateTime() / 1000;
		int nt = g.getIter();
		if ( nt < 0 ) nt = g.getAxisX().getNbins() - 1;
		_point_x[0] = x0 - _canvas.getAxisX().eval( t0 - g.getTime(nt) );
		_point_y[0] = 1;
		int nttmp = nt;
		double xmin = 1;
		for ( int n = 0; n < g.getAxisX().getNbins(); n++ ) {
			long t = t0 - g.getTime(nt);
			_point_x[n+1] = x0 - _canvas.getAxisX().eval( t );
			if ( _point_x[n+1] >0 && xmin > _point_x[n+1] ) 
				xmin = _point_x[n+1]; 
			_point_y[n+1] = y0 - _axis_y.eval( g.getPoint(nt) );
			nttmp = nt;
			nt--;
			if ( nt < 0 ) nt = g.getAxisX().getNbins() - 1;
		}
		_point_x[g.getAxisX().getNbins()+1] = x0 - _canvas.getAxisX().eval( t0 - g.getTime(nttmp) );
		_point_y[g.getAxisX().getNbins()+1] = 1;
		for ( int i = 0; i < _point_x.length; i++ ) {
			if ( _point_x[i] < 0 ) {
				_point_x[i] = xmin;
				_point_y[i] = 1;
			}
		}
	}
	
	public void draw(GCanvas canvas) {
		update();
		canvas.store();
		canvas.setLine(getLine());
		canvas.setFill(getFill());
		if (getDrawOption().matches(".*L.*"))canvas.drawPolyline(_point_x, _point_y);
		if (getDrawOption().matches(".*P.*"))canvas.drawMarkers(1, _point_x.length - 1, _point_x, _point_y);
		canvas.restore();
	}

}
