package b2daq.dqm.graphics;

import java.util.Iterator;

import b2daq.dqm.core.Axis;
import b2daq.graphics.GCanvas;
import b2daq.graphics.GLine;


public class GAxisY extends GAxis {

	public GAxisY() {
		super();
		_label_align = "middle";
	}
	
	public GAxisY(Axis axis) {
		super(axis);
		_label_align = "middle";
	}
	public GAxisY(int nbin, double min, double max, String label,
			boolean fixMin, boolean fixMax, boolean logon){
		super(nbin, min, max, label,fixMin, fixMax, logon);
		_label_align = "middle";
	}

	public GAxisY(int nbin, double min, double max, String label) {
		super(nbin, min, max, label, false, false, false);
		_label_align = "middle";
	}
	
	public void draw(GCanvas canvas) {
		if ( !isVisible() ) return;
		canvas.setLine(getLine());
		canvas.setFont(getLabelsFont());
		String align = (_hand == HAND_R) ? "right " : "left ";
		setTicks(canvas, align);
		for ( Iterator<GValuedText> it = _label_v.iterator(); it.hasNext(); ) {
			GValuedText label = it.next();
			if ( label != null && label.getValue() >= get().getMin() && label.getValue() <= get().getMax() ) {
				label.setPosition(_x0 - _hand * _label_offset, _y0 - eval(label.getValue()));
				label.setFont(getLabelsFont());
				label.setAngle(getLabelAngle());
				label.setAligment(align + _label_align);
				label.draw(canvas);
			}
		}
		for ( Iterator<GLine> it = _tick_line_v.iterator(); it.hasNext(); ) {
			GLine line = it.next();
			if ( line != null && line.getX1() >= get().getMin() && line.getX1() <= get().getMax() ) {
				double X = _x0;
				double Y = _y0 - eval(line.getX1());
				canvas.drawLine(X, Y, X + _hand * _tick_length * line.getY2(), Y);
			}
		}
		canvas.drawLine(_x0, _y0, _x0, _y0 - _length);
		double title_offset = (_hand == HAND_L)? _hand * _padding_x * 0.45 : _hand * _padding_x * 0.80;
		align = "top";
		canvas.setFont(getFont());
		double x = _x0 - title_offset;
		double y = _y0 - _length * 0.5;
		if ( getTitlePosition() == GAxis.POSITION_CENTER ){
			align += " center";
		} else if ( getTitlePosition() == GAxis.POSITION_LEFT ){
			y = 0.9;
			align += " left";
		} else if ( getTitlePosition() == GAxis.POSITION_RIGHT ){
			y = 0.1;
			align += " right";
		}
		canvas.drawString(get().getTitle(), x, y, align, -90);
		if ( _use_border_line == true ) {
			_border_line0.draw(canvas);
			_border_line1.draw(canvas);
		}
	}

	public boolean hit(double x, double y) {
		if ( _hand == HAND_R && x <= _x0 && x >= _x0 - _padding_x && y <= _y0 && y >= _y0 - _length ){
			return true;
		}
		if ( _hand == HAND_L && x >= _x0 && x <= _x0 + _padding_x && y <= _y0 && y >= _y0 - _length ){
			return true;
		}
		return false;
	}
	
	public void mouseReleased() {
		if ( _focused == true ) {
			_focused = false;
			if ( _border_line1.getY1() == 0 ) {
				get().fixMin(false);
				get().fixMax(false);
			} else {
				double min = (get().getMax() - get().getMin()) * (_y0 - _border_line0.getY1()) / _length + get().getMin();
				double max = (get().getMax() - get().getMin()) * (_y0 - _border_line1.getY1()) / _length + get().getMin();
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
