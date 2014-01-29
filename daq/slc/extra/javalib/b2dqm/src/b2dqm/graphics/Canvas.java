package b2dqm.graphics;

import java.util.ArrayList;

import b2daq.graphics.FontProperty;
import b2daq.graphics.GCanvas;
import b2daq.graphics.GRect;
import b2daq.graphics.GShape;
import b2daq.graphics.GText;
import b2daq.graphics.HtmlColor;
import b2dqm.core.Graph1;
import b2dqm.core.Histo;
import b2dqm.core.Histo1;
import b2dqm.core.Histo2;
import b2dqm.core.MonArrow;
import b2dqm.core.MonCircle;
import b2dqm.core.MonLabel;
import b2dqm.core.MonLine;
import b2dqm.core.MonObject;
import b2dqm.core.MonRect;
import b2dqm.core.PieTable;
import b2dqm.core.Range;
import b2dqm.core.TimedGraph1;

public class Canvas extends GRect {

	private GPad _pad = new GPad(0.16, 0.16, 0.68, 0.68);
	private GAxis _axis_x = new GAxisX();
	private GAxis _axis_y = new GAxisY();
	private GAxis _axis_y_2 = null;
	private GAxis _color_axis = null;
	private ArrayList<GMonObject> _histo_v = new ArrayList<GMonObject>();
	private ArrayList<GShape> _shape_v = new ArrayList<GShape>();
	private ArrayList<GShape> _cshape_v = new ArrayList<GShape>();
	private GLegend _legend = null;
	private double _title_offset = 0.0;
	private GText _title = new GText("", 0.5, _pad.getY()/3 - _title_offset, "center top");
	private String _title_position = "center";
	private boolean _use_pad = false;
	private long _update_time = -1;
	private GRect _stat_rect;
	private GText _stat_name;
	private GText _stat_update;
	private GText _stat_entries;
	private Histo _stat_histo = null;
	
	public Canvas(String name, String title, int width, int height) {
		super(0, 0, 1, 1);
		setName(name);
		setTitle(title);
		setFont(new FontProperty(HtmlColor.BLACK, "Arial", 1.1, FontProperty.WEIGHT_BOLD));
		resetPadding();
		setFillColor(HtmlColor.WHITE);
	}

	public Canvas(String name, String title) {
		this(name, title, 100, 100);
	}

	public Canvas() {
		this("", "", 100, 100);
	}

	public GPad getPad() {
		return _pad;
	}

	public void resetPadding() {
		if ( !_use_pad ) return;
		_axis_x.set(_pad.getX(), _pad.getY() + _pad.getHeight(), 
				_pad.getWidth(), 0, 1 - (_pad.getY() + _pad.getHeight()));
		_axis_y.set(_pad.getX(), _pad.getY() + _pad.getHeight(),
				_pad.getHeight(), _pad.getX(), 0);
		if ( _color_axis != null ) {
			_color_axis.set(_pad.getX()+_pad.getWidth() + (1 - _pad.getX() - _pad.getWidth())*0.28, _pad.getY() + _pad.getHeight(),
					_pad.getHeight(), (1 - _pad.getX() - _pad.getWidth())*0.25, 0);
		}
		if ( _axis_y_2 != null ) {
			_axis_y_2.set(_pad.getX()+_pad.getWidth(), _pad.getY() + _pad.getHeight(),
				_pad.getHeight(), 1 - _pad.getX() - _pad.getWidth(), 0);
		}
	}
	
	public int getNHistos() {
		return _histo_v.size();
	}

	public GHisto getHisto(int index) {
		return (GHisto)_histo_v.get(index);
	}

	public GMonObject getMonObject(int index) {
		return (GMonObject)_histo_v.get(index);
	}

	public ArrayList<GMonObject> getMonObjects() {
		return _histo_v;
	}

	public GLegend getLegend() {
		return _legend;
	}

	public void setMonObject(int index, MonObject h) {
		_histo_v.get(index).set(h);
	}

	public GMonObject addHisto(MonObject obj) {
		if ( !_use_pad ) {
			_use_pad = true;
			try {
				Histo h = (Histo)obj;
				getAxisX().get().copy(h.getAxisX());
				getAxisY().get().copy(h.getAxisY());
				if ( getTitle().length() == 0 ) {
					setTitle(h.getTitle());
				}
				_stat_histo  = h;
				_stat_rect = new GRect(0.71, 0.02, 0.28, 0.12, HtmlColor.WHITE, HtmlColor.BLACK);
				_stat_name = new GText(h.getName(), 0.85, 0.055, "center");
				_stat_name.setFontSize(0.52);
				if (_stat_name.getText().length() > 20) _stat_name.setFontSize(0.47);
				_stat_update = new GText("mean     : ", 0.72, 0.088, "left");
				_stat_update.setFontSize(0.52);
				_stat_entries = new GText("entries : "+h.getEntries(), 0.72, 0.128, "left");
				_stat_entries.setFontSize(0.52);
				addShape(_stat_rect);
				addShape(_stat_name);
				addShape(_stat_update);
				addShape(_stat_entries);
			} catch (Exception e) {}
		}
		GMonObject dh = null;
		String datatype = obj.getDataType();
		if (datatype.matches("H1.")) {
			dh = new GHisto1(this, (Histo1) obj);
		} else if ( datatype.matches("H2.") ) {
			if (_color_axis == null ) {
				_color_axis = new GColorAxis(10, 0, 1, "", false, false, false);
			}
			_color_axis.setHand(GAxis.HAND_L);
			resetPadding();
			dh = new GHisto2(this, (Histo2) obj);
		} else if (datatype.matches("G1.")) {
			dh = new GGraph1(this, (Graph1) obj);
		} else if (datatype.matches("TG.")) {
			TimedGraph1 graph = (TimedGraph1) obj;
			dh = new GTimedGraph1(this, graph);
			_axis_x.setTimeScale(GAxis.TIME_SCALE_SEC);
			_axis_x.get().copy(graph.getAxisX());
			_axis_x.setLabelAngle(45);
			_axis_x.setLabelAlignment("top");
		} else if (datatype.matches("R1V")) {
			dh = new GVerticalRange(this, (Range) obj);
		} else if (datatype.matches("R1H")) {
			dh = new GHorizontalRange(this, (Range) obj);
		} else if (datatype.matches("R1B")) {
			dh = new GBevelRange(this, (Range) obj);
		} else if (datatype.matches("PIE")) {
			dh = new GPieTable(this, (PieTable)obj);
			_use_pad = false;
		} else if (datatype.matches("MLB")) {
			dh = new GMonLabel(this, (MonLabel)obj);
			_use_pad = false;
		} else if (datatype.matches("MRT")) {
			dh = new GMonRect(this, (MonRect)obj);
			_use_pad = false;
		} else if (datatype.matches("MCC")) {
			dh = new GMonCircle(this, (MonCircle)obj);
			_use_pad = false;
		} else if (datatype.matches("MLN")) {
			dh = new GMonLine(this, (MonLine)obj);
			_use_pad = false;
		} else if (datatype.matches("MAW")) {
			dh = new GMonArrow(this, (MonArrow)obj);
			_use_pad = false;
		} else if (datatype.matches("..M")) {
			_use_pad = false;
			if (_color_axis == null ) {
				_color_axis = new GColorAxis(10, 0, 1, "", false, false, false);
			}
			_color_axis.setHand(GAxis.HAND_L);
			_color_axis.getFont().setSize(0.96);
			_color_axis.set(0.76, 0.94, 0.26, 0.03, 0);
		}
		_histo_v.add(dh);
		return dh;
	}

	public void addShape(GShape shape) {
		_shape_v.add(shape);
	}
	
	public void addCShape(GShape shape) {
		_cshape_v.add(shape);
	}
	
	public void setLegend(GLegend legend) {
		_legend = legend;
	}

	public void update() {
		for ( GMonObject histo : _histo_v ) {
			histo.update();
		}
		if ( _stat_histo != null) {
			if (_stat_histo.getDim() == 1) {
				String label = String.format("%1$.3f", _stat_histo.getMean());
				_stat_update.setText("mean     : " + label);
			} else if (_stat_histo.getDim() == 2) {
				Histo2 h = (Histo2)_stat_histo;
				String label = String.format("(%1$.3f, ", h.getMeanX());
				label += String.format("%1$.3f)", h.getMeanY());
				_stat_update.setText("mean     : " + label);
			}
			   _stat_entries.setText("integral : " + ((int)_stat_histo.getEntries()));
		}
	}
	
	public void draw(GCanvas canvas) {
		canvas.store();
		super.draw(canvas);
		update();
		if ( _histo_v.size() > 0 ){
			if ( _use_pad ) {
				try {
					Histo histo = (Histo)(_histo_v.get(0).get());
					canvas.drawRect(_pad.getX(), _pad.getY(), _pad.getWidth(), _pad.getHeight());
					getAxisX().get().copyRange(histo.getAxisX());
					getAxisY().get().copyRange(histo.getAxisY());
					_update_time = histo.getUpdateTime();
					getAxisX().setUpdateTime(_update_time);
					getAxisY().setUpdateTime(_update_time);
					_pad.draw(canvas);
					if (_color_axis != null)
						_color_axis.get().copyRange(histo.getAxisZ());
					for ( GMonObject obj : _histo_v ) obj.draw(canvas);
					for ( GShape shape : _cshape_v ) shape.draw(canvas);
					canvas.setFill(getFill());
					canvas.setLine(null);
					canvas.drawRect(0, 0, _pad.getX(), 1);
					canvas.drawRect(_pad.getX() + _pad.getWidth(), 0, 1 - (_pad.getX() + _pad.getWidth()), 1);
					canvas.drawRect(0, 0, 1, _pad.getY());
					canvas.drawRect(0, _pad.getY() + _pad.getHeight(), _pad.getX() + _pad.getWidth(), 1 - (_pad.getY() + _pad.getHeight()));
					canvas.setFill(null);
					canvas.setLine(_pad.getLine());
					canvas.drawRect(_pad.getX(), _pad.getY(), _pad.getWidth(), _pad.getHeight());
					_title.setFont(getFont());
					_title.draw(canvas);
					_axis_x.draw(canvas);
					_axis_y.draw(canvas);
					if ( _axis_y_2 != null )
						_axis_y_2.draw(canvas);
					if (_color_axis != null)
						_color_axis.draw(canvas);
					if (_legend != null)
						_legend.draw(canvas);
				} catch ( Exception e ) {
					e.printStackTrace();
				}
				for ( GShape shape : _shape_v ) shape.draw(canvas);
			} else {
				try {
					Histo histo = (Histo)(_histo_v.get(0).get());
					if (_color_axis != null) {
						_color_axis.get().copyRange(histo.getAxisY());
						_color_axis.draw(canvas);
					}
				} catch ( Exception e ) {}
				for ( GShape shape : _shape_v ) shape.draw(canvas);
				for ( GMonObject obj : _histo_v ) obj.draw(canvas);
			}
		}
		canvas.restore();
	}

	public GAxis getAxisX() { return _axis_x; }
	public GAxis getAxisY() { return _axis_y; }
	public GAxis getAxisY2() { return _axis_y_2; }

	public String getTitle() {
		return _title.getText();
	}

	public void setTitlePosition (String pos) {
		if (pos.matches("center")) {
			_title_position = pos;
			_title.setX(0.5);
			_title.setAligment("center top");
		} else if (pos.matches("left")) {
			_title_position = pos;
			_title.setX(0.025);
			_title.setAligment("left top");
		} else if (pos.matches("right")) {
			_title_position = pos;
			_title.setX(0.925);
			_title.setAligment("right top");
		}
	}
	
	public String getTitlePosition() {
		return _title_position;
	}

	public void setTitle(String title) {
		_title.setText(title);
		if (title.length() > 20) setTitlePosition("left");
		if (title.length() > 25) _title.setFontSize(1.0);
		if (title.length() > 30) _title.setFontSize(0.9);
		if (title.length() > 35) _title.setFontSize(0.8);
		if (title.length() > 40) _title.setFontSize(0.7);
	}
	
	public void setAxisY2(GAxis axis) {
		_axis_y_2 = axis;
		_axis_y_2.setHand(GAxis.HAND_L);
		resetPadding();
	}
	
	public GAxis getColorAxis() {
		return _color_axis;
	}

	public void setColorAxis(GAxis axis) {
		_color_axis = axis;
	}

	public boolean usePad() {
		return _use_pad;
	}

	public void setUpdateTime(long update_time) {
		_update_time = update_time;
	}

	public long getUpdateTime() {
		return _update_time;
	}

	public void setTitleOffset(double offset) {
		_title_offset = offset;
		_title.setY(_pad.getY()/3 - _title_offset);
	}

	public double getTitleOffset() {
		return _title_offset;
	}

}
