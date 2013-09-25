package b2dqm.graphics;

import b2daq.graphics.GCanvas;
import b2daq.graphics.GLine;
import b2dqm.core.Axis;

public class GAxisX extends GAxis {

	public GAxisX(){ 
		super();
		_label_align = "center";
	}

	public GAxisX(Axis axis){
		super(axis);
		_label_align = "center";
	}

	public GAxisX(int nbin, double min, double max, String label,
			boolean fixMin, boolean fixMax, boolean logon){
		super(nbin, min, max, label, fixMin, fixMax, logon);
		_label_align = "center";
	}
		
	public void draw(GCanvas canvas) {
		if ( !isVisible() ) return;
		canvas.restore();
		canvas.setLine(getLine());
		canvas.setFont(getLabelsFont());
		setTicks(canvas, _label_align + " top");
		for ( GValuedText label : _label_v ) {
			if ( label != null && label.getValue() >= get().getMin() && label.getValue() <= get().getMax() ) {
				label.setPosition(_x0 + eval(label.getValue()), _y0 + _label_offset);
				label.setFont(getLabelsFont());
				label.setAngle(getLabelAngle());
				label.setAligment(_label_align + " top");
				label.draw(canvas);
			}
		}
		for ( GLine line : _tick_line_v ) {
			if ( line != null && line.getX1() >= get().getMin() && line.getX1() <= get().getMax() ) {
				double X = _x0 + eval(line.getX1());
				double Y = _y0;
				canvas.drawLine(X, Y, X, Y - _tick_length * line.getY2());
			}
		}
		canvas.drawLine(_x0, _y0, _x0 + _length, _y0);
		canvas.setFont(getFont());
		double x = _x0 + _length / 2;
		double y = _y0 + _padding_y * 0.5;
		String align = "center top";
		if ( getTitlePosition() == GAxis.POSITION_RIGHT ){
			x = 0.9;
			align = "right top";
		} else if ( getTitlePosition() == GAxis.POSITION_LEFT ){
			x = 0.1;
			align = "left top";
		}
		canvas.drawString(get().getTitle(), x, y, align);
		canvas.restore();
		if ( _use_border_line == true ) {
			_border_line0.draw(canvas);
			_border_line1.draw(canvas);
		}
	}

	public boolean hit(double x, double y) {
		if ( x >= _x0 && x <= _x0 + _length && y >= _y0 && y <= _y0 + _padding_y ){
			return true;
		}
		return false;
	}
	
	public void mouseReleased() {
		if ( _focused == true ) {
			_focused = false;
			if ( _border_line1.getX1() == 0 ) {
				get().fixMin(false);
				get().fixMax(false);
			} else {
				double min = (get().getMax() - get().getMin()) * (_border_line0.getX1() - _x0) / _length + get().getMin();
				double max = (get().getMax() - get().getMin()) * (_border_line1.getX1() - _x0) / _length + get().getMin();
				get().fixMin(false);
				get().fixMax(false);
				if ( min < max ) {
					get().setMin(min);
					get().setMax(max);
				} else if ( min > max ) {
					get().setMin(max);
					get().setMax(min);
				}
				get().fixMin(true);
				get().fixMax(true);
			}
			_border_line0.set(0,0,0,0);
			_border_line1.set(0,0,0,0);
		}
	}
}
