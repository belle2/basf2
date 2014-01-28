package b2dqm.graphics;

import java.util.ArrayList;
import java.util.Date;

import b2daq.core.NumberString;
import b2daq.graphics.FontProperty;
import b2daq.graphics.GCanvas;
import b2daq.graphics.GLine;
import b2daq.graphics.GShape;
import b2daq.graphics.HtmlColor;
import b2dqm.core.Axis;


public abstract class GAxis extends GShape {

	public final static int HAND_R = 1;
	public final static int HAND_L = -1;

	public final static int TIME_SCALE_NON = -1;
	public final static int TIME_SCALE_SEC = 0;
	public final static int TIME_SCALE_MIN = 1;
	public final static int TIME_SCALE_HOUR = 2;

	public final static int POSITION_CENTER = 0;
	public final static int POSITION_LEFT = 1;
	public final static int POSITION_RIGHT = 2;

	public final static int LABEL_POSITION_CENTER = 0;
	public final static int LABEL_POSITION_LEFT = 1;
	public final static int LABEL_POSITION_RIGHT = 2;

	protected Axis _axis;

	protected double _x0, _y0, _length, _padding_x, _padding_y;
	protected int _hand = HAND_R;
	protected boolean _auto_label = true;
	protected int _time_scale = TIME_SCALE_NON;
	private boolean _log_scale = false;
	private long _update_time = (new Date()).getTime();

	private int _title_position = POSITION_CENTER;

	protected ArrayList<GLine> _tick_line_v = new ArrayList<GLine>();
	private double _title_offset = 0.01;
	protected double _tick_length = 0.01;

	protected ArrayList<GValuedText> _label_v = new ArrayList<GValuedText>();
	private FontProperty _label_font = new FontProperty();
	protected double _label_offset = 0.01;
	protected double _label_angle = 0;

	protected boolean _use_border_line = false;
	protected GLine _border_line0 = new GLine(0, 0, 0, 0, HtmlColor.BLACK);
	protected GLine _border_line1 = new GLine(0, 0, 0, 0, HtmlColor.BLACK);
	protected String _label_align;
	private String _time_format = "";
	private int _ndivisions = 10;

	public GAxis() {
		super();
		_axis = new Axis();
		setLineColor(HtmlColor.BLACK);
	}

	public GAxis(Axis axis) {
		_axis = axis;
		setLineColor(HtmlColor.BLACK);
	}

	public GAxis(int nbin, double min, double max, String label,
			boolean fixMin, boolean fixMax, boolean log) {
		_axis = new Axis(nbin, min, max, label);
		_axis.fixMin(fixMin);
		_axis.fixMax(fixMax);
		setLineColor(HtmlColor.BLACK);
		setLogScale(log);
	}

	public Axis get() {
		return _axis;
	}

	public void set(Axis axis) {
		_axis = axis;
	}

	public void set(double x0, double y0, double length, double padding_x,
			double padding_y) {
		_x0 = x0;
		_y0 = y0;
		_length = length;
		_padding_x = padding_x;
		_padding_y = padding_y;
	}

	public void setX(double x0) {
		_x0 = x0;
	}

	public void setY(double y0) {
		_y0 = y0;
	}

	public void setLength(double length) {
		_length = length;
	}

	public void setPaddingX(double padding_x) {
		_padding_x = padding_x;
	}

	public void setPaddingY(double padding_y) {
		_padding_y = padding_y;
	}

	public double getX() {
		return _x0;
	}

	public double getY() {
		return _y0;
	}

	public double getLength() {
		return _length;
	}

	public double getPaddingX() {
		return _padding_x;
	}

	public double getPaddingY() {
		return _padding_y;
	}

	public void addLabel(String label, double value) {
		_auto_label = false;
		_label_v.add(new GValuedText(label, value, "top"));
	}

	public void setLabels(ArrayList<GValuedText> label_v) {
		_auto_label = false;
		_label_v = label_v;
	}

	public void setLabels(GValuedText[] label_a) {
		ArrayList<GValuedText> label_v = new ArrayList<GValuedText>();
		for (int n = 0; n < label_a.length; n++) {
			label_v.add(label_a[n]);
		}
		setLabels(label_v);
	}

	public void setLineColor(HtmlColor color) {
		super.setLineColor(color);
		_border_line0.setLineColor(color);
		_border_line1.setLineColor(color);
	}

	public double getScale() {
		if (isLogScale())
			return _length / (Math.log(_axis.getMax()) - Math.log(_axis.getMin()));
		else
			return _length / (_axis.getMax() - _axis.getMin());
	}

	public void setTickLength(double tick) {
		_tick_length = tick;
	}

	public double getTickLength() {
		return _tick_length;
	}

	public void setHand(int hand) {
		_hand = hand;
	}

	public void mousePressed(double x1, double y1, double x2, double y2) {
		_focused = true;
		_border_line0.set(x1, y1, x2, y2);
		_use_border_line = true;
	}

	public void mouseReleased() {
		if (_focused == true) {
			_border_line0.set(0, 0, 0, 0);
			_border_line1.set(0, 0, 0, 0);
		}
		_focused = false;
		_use_border_line = false;
	}

	public void mouseDragged(double x1, double y1, double x2, double y2) {
		if (_focused == true) {
			_border_line1.set(x1, y1, x2, y2);
		}
	}

	public void setLabelsFont(FontProperty ticks_font) {
		_label_font = ticks_font;
	}

	public FontProperty getLabelsFont() {
		return _label_font;
	}

	public void setTimeScale(int scale) {
		_time_scale = scale;
		switch( _time_scale ) {
		case TIME_SCALE_SEC:
			_ndivisions = 6;
			break;
		case TIME_SCALE_MIN:
			_ndivisions = 6;
			break;
		case TIME_SCALE_HOUR:
			_ndivisions = 6;
			break;
		}
	}

	public int getTimeScale() {
		return _time_scale;
	}

	public void setTitleOffset(double title_offset) {
		_title_offset = title_offset;
	}

	public double getTitleOffset() {
		return _title_offset;
	}

	public void setTitlePosition(int title_position) {
		_title_position = title_position;
	}

	public int getTitlePosition() {
		return _title_position;
	}

	public void setTitlePosition(String title_position) {
		if (title_position.matches("center")) {
			_title_position = POSITION_CENTER;
		} else if (title_position.matches("left")) {
			_title_position = POSITION_LEFT;
		} else if (title_position.matches("right")) {
			_title_position = POSITION_RIGHT;
		}
	}

	public String getTitlePositionString() {
		if (_title_position == POSITION_CENTER) {
			return "center";
		} else if (_title_position == POSITION_LEFT) {
			return "left";
		} else if (_title_position == POSITION_RIGHT) {
			return "right";
		} else {
			return "";
		}
	}

	public boolean isLogScale() {
		return (_axis.getMin() > 0) && _log_scale;
	}

	public void setLogScale(boolean enable) {
		_log_scale = enable;
	}

	protected void setTicks(GCanvas canvas, String align) {
		_tick_line_v.clear();
		_tick_line_v = new ArrayList<GLine>();
		if (_auto_label) {
			_label_v.clear();
			_label_v = new ArrayList<GValuedText>();
		}
		if ( _time_scale == GAxis.TIME_SCALE_NON ) {
			double dx = Math.pow(10d, Math.floor(Math.log10(_axis.getMax() - _axis.getMin())) - 1 ) * 2;// / _ndivisions;
			if (_axis.getMax() < dx * 25) dx /= 2;
			long n_min = (int) Math.floor(_axis.getMin() / dx), n_max = (int) Math.floor(_axis.getMax() / dx);
			int nR = 0;
			long n5_min = n_min - n_min % _ndivisions, n5_max = n_max - n_max % _ndivisions;
			while (nR < _ndivisions) {
				if (NumberString.toString(n5_min * dx, nR).compareTo(
						NumberString.toString(n5_max * dx, nR)) != 0) {
					break;
				}
				nR++;
			}
			if (NumberString.toString((n5_max - _ndivisions) * dx, nR).compareTo(
					NumberString.toString(n5_max * dx, nR)) == 0) {
				nR++;
			}
			double x = 0;
			if ( isLogScale() ) {
				int logMax = (int)Math.floor( Math.log10(_axis.getMax()) );
				int logMin = (int)Math.floor( Math.log10(_axis.getMin()) );
				for ( int nlog = logMin; nlog <= logMax; nlog++ ) {
					double ddx = Math.pow(10, nlog); 
					x = ddx;
					if (_auto_label) {
						String s = NumberString.toString(x, nR);
						_label_v.add(new GValuedText(s, x, align));
					}
					_tick_line_v.add(new GLine(x, 0, x, 2));
					for ( int n = 1; n < 10; n++) {
						x = n * ddx; 
						_tick_line_v.add(new GLine(x, 0, x, 1));
					}					
				}
			} else {
				for (long n = n_min; n <= n_max; n++) {
					x = n * dx;
					if (x >= _axis.getMin() && x <= _axis.getMax()) {
						if (n % _ndivisions == 0) {
							if (_auto_label) {
								String s = "";
								s = NumberString.toString(x, nR);
								_label_v.add(new GValuedText(s, x, align));
							}
							_tick_line_v.add(new GLine(x, 0, x, 2));
						} else {
							_tick_line_v.add(new GLine(x, 0, x, 1));
						}
					}
				}
			}
		} else {
			long dx = 5;
			long time_max = (int) _axis.getMax();
			int range = (int) _axis.getMax() - (int) _axis.getMin();
			String format = "HH:mm:ss";
			long time_base = _update_time - time_max * 1000;
			if ( range >= 60 * 60 * 24 * 6 ) {
				format = "M/d";
				dx = 3600 * 24;
			} else if ( range > 60 * 60 * 6 ) {
				format = "M/d HH:mm";
				dx = 3600 * 2;
			} else if ( range >  60 * 5 ) {
				format = "HH:mm";
				dx = 300;
			}
			if ( _time_format.length() > 0 ) format = _time_format;
			long x = 0;
			int n = 0;
			while ( x <= time_max ) {
				if (n % _ndivisions == 0) {
					if (_auto_label) {
						String s = "";
						s = canvas.getTime((long) ( time_base + x * 1000), format);
						_label_v.add(new GValuedText(s, x, "left top"));
					}
					_tick_line_v.add(new GLine(x, 0, x, 2));
				} else {
					_tick_line_v.add(new GLine(x, 0, x, 1));
				}
				n++;
				x += dx;
			}
		}
	}

	public double eval(double v) {
		if ( isLogScale() ) {
			if ( v <= 0 ) return 0;
			return (Math.log(v) - Math.log(_axis.getMin())) * getScale();
		} else {
			return (v - _axis.getMin()) * getScale();
		}
	}

	public void setUpdateTime(long update_time) {
		_update_time = update_time;
	}

	public double getUpdateTime() {
		return _update_time;
	}

	public void setLabelAngle(double label_angle) {
		_label_angle = label_angle;
	}

	public double getLabelAngle() {
		return _label_angle;
	}

	public void setLabelsFontSize(double size) {
		_label_font.setSize(size);
	}

	public void setLabelAlignment(String align) {
		_label_align = align;
	}

	public String getLabelAlignment() {
		return _label_align;
	}

	public void setLabelOffset(double label_offset) {
		_label_offset = label_offset;
	}

	public void setTimeFormat(String time_format) {
		_time_format = time_format;
	}

	public int getNdivisions() {
		return _ndivisions;
	}

	public void setNdivisions(int ndiv) {
		if ( ndiv > 0 ) _ndivisions = ndiv;
	}

}
