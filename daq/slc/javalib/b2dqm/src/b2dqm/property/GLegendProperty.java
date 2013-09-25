package b2dqm.property;

import b2daq.graphics.GObject;
import b2dqm.core.HistoPackage;
import b2dqm.graphics.Canvas;
import b2dqm.graphics.GHisto;
import b2dqm.graphics.GLegend;
import b2dqm.graphics.GMonObject;

public class GLegendProperty extends GRectProperty {

	public final static String ELEMENT_LABEL = "legend";
	
	private GLegend _legend = null;
	private String [] _name_v = null;
	
	public GLegendProperty(Canvas canvas) {
		super(canvas);
		_legend = new GLegend(); 
		canvas.setLegend(_legend);
		_rect = _legend;
		_shape = _legend;
	}

	@Override
	public void put(String key, String value) {
		if (_legend == null) return;
		super.put(key, value);
		if ( key.matches("list") ) {
			_name_v = value.split(",");
			for ( int n = 0; n< _name_v.length; n++ ) {
				_name_v[n] = _name_v[n].replaceAll(" ", "").replaceAll("\t", "");
			}
		}
	}

	public GObject set(Canvas canvas, HistoPackage pack) {
		if ( _legend != null && _name_v != null) {
			for ( GMonObject obj : canvas.getMonObjects() ) {
				for ( String name : _name_v ) {
					try {
						if ( name.matches(obj.get().getName()) ) {
							try {
								_legend.add((GHisto)obj);
							} catch (Exception e) {}
						} 
					} catch (Exception e) {
						e.printStackTrace();
					}
				}
			}
			
		}
		return _legend;
	}
	
	@Override
	public String getType() {
		return ELEMENT_LABEL;
	}
	
}
