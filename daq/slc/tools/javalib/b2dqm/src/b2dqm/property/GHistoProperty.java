package b2dqm.property;

import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import b2daq.graphics.HtmlColor;
import b2dqm.core.Histo;
import b2dqm.core.Histo1F;
import b2dqm.core.HistoPackage;
import b2dqm.core.MonObject;
import b2dqm.graphics.Canvas;
import b2dqm.graphics.GAxis;
import b2dqm.graphics.GHisto;
import b2dqm.graphics.HyperRef;


public class GHistoProperty extends GProperty {

	public final static String ELEMENT_LABEL = "histo";

	protected String _linked_axis_name = "";
	private String _ref_channel = "";
	
	private GHisto _histo = null;
	
	public GHistoProperty(Canvas canvas, HistoPackage pack, String name) {
		super(canvas);
		for ( MonObject histo : pack.getMonObjects() ) {
			if ( name.matches(histo.getName()) ) {
				try {
					_histo = (GHisto)canvas.addHisto(histo);
				} catch (Exception e) {}
				break;
			}
		}
		if ( _histo == null ) {
			_histo = (GHisto)canvas.addHisto(new Histo1F("__", "", 10, 0, 1));
		}
	}

	@Override
	public String getType () {
		return ELEMENT_LABEL;
	}
	
	public void put(String key, String value) {
		Histo h = _histo.get(); 
		value = parseUnicode(value);
		if ( key.matches("title") ) {
			h.setTitle(value);
		} else if ( key.matches("ref") ) {
			_ref_channel = value;
		} else if ( key.matches("maximum") ) {
			h.fixMaximum(Double.parseDouble(value), true);
		} else if ( key.matches("minimum") ) {
			h.fixMinimum(Double.parseDouble(value), true);
		} else if ( key.matches("line-color") ) {
			_histo.setLineColor(new HtmlColor(value));
		} else if ( key.matches("line-width") ) {
			_histo.setLineWidth((int)Double.parseDouble(value));
		} else if ( key.matches("fill-color") ) {
			_histo.setFillColor(new HtmlColor(value));
		} else if ( key.matches("font-color") ) {
			_histo.setFontColor(new HtmlColor(value));
		} else if ( key.matches("font-size") ) {
			_histo.setFontSize(Double.parseDouble(value));
		} else if ( key.matches("font-family") ) {
			_histo.setFontFamily(value);
		} else if ( key.matches("draw-option") ) {
			_histo.setDrawOption(value);
		} else if ( key.matches("linked-axis") ) {
			_linked_axis_name = value;
		} 
	}
	
	public GHisto set (Canvas canvas, HistoPackage pack) {
		if ( _histo != null ) {
			if ( _linked_axis_name.length() > 0 ) {
				GAxis axis = canvas.getAxisX();
				if ( axis.getName().matches(_linked_axis_name) ) {
					_histo.setAxisY(axis);
					_histo.setAxisAjustable(true);
				}
				axis = canvas.getAxisY();
				if ( axis.getName().matches(_linked_axis_name) ) {
					_histo.setAxisY(axis);
					_histo.setAxisAjustable(true);
				}
				axis = canvas.getAxisY2();
				if ( axis != null && axis.getName().matches(_linked_axis_name) ) {
					_histo.setAxisY(axis);
					_histo.setAxisAjustable(true);
				} 
				axis = canvas.getColorAxis();
				if ( axis != null && axis.getName().matches(_linked_axis_name) ) {
					_histo.setAxisY(axis);
					_histo.setAxisAjustable(true);
				}
			}
		}
		return _histo;
	}

	public void setHyperLink(Map<String, Canvas> map) {
		String [] target_v = _ref_channel.split("\\) *, *\\(");
		String regex;
		Histo h = (Histo)_histo.get();
		if ( h.getDataType().matches("H2.*") ) {
			regex = "\\( *(.+) *, *(\\d+) *, *(\\d+) *\\)";
			if ( target_v.length > 0 ) {
				add1(map, find(regex, target_v[0]+")"));
				for (int i = 1; i < target_v.length-1; i++) {
					add1(map, find(regex, "("+target_v[i]+")"));
				}
			}
		} else if ( h.getDataType().matches("I.*M") ) {
			regex = "\\( *(.+) *, *(\\d+) *\\)";
			if ( target_v.length > 1 ) {
				add2(map, find(regex, target_v[0]+")"));
				for (int i = 1; i < target_v.length-1; i++) {
					add2(map, find(regex, "("+target_v[i]+")"));
				}
				add2(map, find(regex, "("+target_v[target_v.length-1]));
			}
		}
	}
	
	private void add1(Map<String, Canvas> map, String [] value_v) {
		if (value_v == null) return;
		_histo.addChannelRef(new HyperRef(map.get(value_v[0]), 
				Integer.parseInt(value_v[1]),
				Integer.parseInt(value_v[2])));
	}
	
	private void add2(Map<String, Canvas> map, String [] value_v) {
		if (value_v == null) return;
		_histo.addChannelRef(new HyperRef(map.get(value_v[0]), 
				Integer.parseInt(value_v[1]),0));
	}
	
	private static String[] find(String regex, String target) {
		Pattern pattern = Pattern.compile(regex);
		Matcher matcher = pattern.matcher(target);
		if ( matcher.find() ){
			String [] value_v = new String [matcher.groupCount()]; 
			for (int n = 0; n < value_v.length; n++) {
				value_v[n] = matcher.group(n+1);
			}
			return value_v;
		}
		return null;
	} 
}
