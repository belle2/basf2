package b2daq.dqm.graphics;

import java.util.ArrayList;

import b2daq.dqm.core.Histo;
import b2daq.dqm.core.MonObject;
import b2daq.graphics.HtmlColor;


public abstract class GHisto extends GMonObject {

	protected Histo _histo = null;
	protected GAxis _axis_y = null;
	protected double[] _point_x;
	protected double[] _point_y;
	protected String _opt = "AL";
	protected ArrayList<HyperRef> _channel_ref_v = new ArrayList<HyperRef>();
	protected HyperRef _selected_channel_ref = null;
	private boolean _axis_ajustable = false;

	public GHisto(){
		super(null);
	}

	public GHisto(Canvas canvas){
		super(canvas);
		_axis_y = canvas.getAxisY();
		setLineColor(HtmlColor.RED);
		setFontSize(0.5);
	}

	public GHisto(Canvas canvas, Histo histo){
		this(canvas);
		_histo = histo; 
	}
	
	public Histo get() {
		return _histo;
	}
	
	public Histo getHisto() {
		return _histo;
	}

	public void set(MonObject histo) {
		_histo = (Histo)histo;
	}
	
	public ArrayList<HyperRef> getChannelRefs() {
		return _channel_ref_v;
	}

	public void addChannelRef(HyperRef ref) {
		_channel_ref_v.add(ref);
	}
	
	public void setAxisY(GAxis axis) {
		_axis_y = axis;
	}
	
	public void update() {
		if ( _axis_ajustable ) {
			_axis_y.get().copyRange(_histo.getAxisY());
		}
	}
	
	public void setDrawOption(String opt) {
		_opt = opt;
	}
	
	public String getDrawOption() {
		return _opt;
	}
	
	public HyperRef getSelectedChannelRef() {
		return _selected_channel_ref;
	}

	public boolean isAxisAjustable() {
		return _axis_ajustable;
	}

	public void setAxisAjustable(boolean axis_ajustable) {
		_axis_ajustable = axis_ajustable;
	}
	
}
