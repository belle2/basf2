package b2dqm.graphics;

import b2daq.core.NumberString;
import b2daq.graphics.GCanvas;
import b2daq.graphics.GText;
import b2dqm.core.Histo;
import b2dqm.core.Histo1;

public class GHisto1 extends GHisto {

	private GText[] _text_v = null;

	public GHisto1() {
		super();
	}
	
	public GHisto1(Canvas canvas, Histo1 histo) {
		super(canvas, histo);
    	_point_x = new double [(_histo.getAxisX().getNbins() + 1) * 2];
    	_point_y = new double [(_histo.getAxisX().getNbins() + 1) * 2];
    	_text_v = new GText [_histo.getAxisX().getNbins()];
    	for ( int n =0 ; n < _text_v.length ; n++ ) {
    		_text_v[n] = new GText();
    	}
    	getFont().setSize(0.9);
	}
	
	public void update() {
		super.update();
    	double x0 = _canvas.getPad().getX();
    	double y0 = _canvas.getPad().getY() + _canvas.getPad().getHeight();
    	double xmin = _canvas.getAxisX().get().getMin();
    	double ymin = _axis_y.get().getMin();
    	if ( _axis_y.isLogScale() ) {
    		ymin = Math.log(ymin);
    	}
    	double xscale = _canvas.getAxisX().getScale();
    	double dbinx = (_histo.getAxisX().getMax() - _histo.getAxisX().getMin())
    		/ _histo.getAxisX().getNbins();
    	
    	double X = x0 + ( _histo.getAxisX().getMin() - xmin ) * xscale;
    	double X2 = 0;
    	if(X < x0) X = x0;
    	_point_x[0] = X; _point_y[0] = y0;
    	double Y;
    	int nin = 0;
		int n_digit = 1;
		if ( _histo.getDataType().matches(".*I")
			 ||_histo.getDataType().matches(".*C") || _histo.getDataType().matches(".*S")) {
			n_digit = 0;			
		}
    	for(int n = 0; n < _histo.getAxisX().getNbins(); n++){
            X = x0 + _canvas.getAxisX().eval( dbinx * n  + _histo.getAxisX().getMin() );
            Y = y0 - _axis_y.eval(_histo.getBinContent(n));
            _point_x[2*nin+1] = X;
            _point_y[2*nin+1] = Y;
            X2 = x0 + _canvas.getAxisX().eval( dbinx * (n + 1)  + _histo.getAxisX().getMin() );
            _point_x[2*nin+2] = X2;
            _point_y[2*nin+2] = Y;
			_text_v[n].setFont(getFont());
			_text_v[n].setPosition((X + X2) / 2, Y);
			_text_v[n].setAligment("center bottom");
			_text_v[n].setText(NumberString.toString(_histo.getBinContent(n), n_digit));
            nin++;
    	}
    	_point_x[2*nin+1] = X2;
    	_point_y[2*nin+1] = y0;
	}
	
	public String getMessage(double x, double y) {
		if (!_canvas.getPad().hit(x, y)) return "";
		for( int n = 1; n <= _histo.getAxisX().getNbins(); n++ ) {
    		if ( x < _point_x[2*n] ) {
        		if ( y  >= _point_y[2*n] && y < _canvas.getPad().getY() + _canvas.getPad().getHeight()) {
        			return "<html>" +
        				"histo = " + get().getName() + "<br />" +
        				"bin = " + n +" <br />"  +
        				"value = " + NumberString.toString(((Histo)get()).getBinContent(n-1), 2) +  
        				"</html>";
        		}
    		}
    	}
		return "";
	}

	public void draw(GCanvas canvas) {
		update();
		canvas.store();
		canvas.setLine(getLine());
		canvas.setFill(getFill());
		canvas.drawPolygon(_point_x, _point_y);
		if ( getDrawOption().matches(".*T.*") ) {
			for ( GText text : _text_v ) text.draw(canvas);
		}
		canvas.restore();
	}

}
