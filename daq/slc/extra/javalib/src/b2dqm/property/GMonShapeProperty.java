package b2dqm.property;

import b2daq.graphics.FontProperty;
import b2daq.graphics.HtmlColor;
import b2dqm.core.HistoPackage;
import b2dqm.core.MonObject;
import b2dqm.graphics.Canvas;
import b2dqm.graphics.GHisto;
import b2dqm.graphics.GMonLabel;
import b2dqm.graphics.GMonShape;

public class GMonShapeProperty extends GProperty {

	public final static String ELEMENT_LABEL = "mon-shape";

	protected String _ref_channel = "";
	
	private GMonShape _shape = null;
	
	public GMonShapeProperty(Canvas canvas, HistoPackage pack, String name) {
		super(canvas);
		for ( MonObject histo : pack.getMonObjects() ) {
			if ( name.matches(histo.getName()) ) {
				_shape = (GMonShape)canvas.addHisto(histo);
				break;
			}
		}
	}

	@Override
	public String getType () {
		return ELEMENT_LABEL;
	}
	
	public void put(String key, String value) {
		if ( _shape == null ) return;
		if ( key.matches("ref") ) {
			_ref_channel = value;
		} else if ( key.matches("bounds") ) {
			GMonLabel label = (GMonLabel)_shape;
			String [] str_v = value.split(",");
			double x = Double.parseDouble(str_v[0]);
			double y = Double.parseDouble(str_v[1]);
			double w = Double.parseDouble(str_v[2]);
			double h = Double.parseDouble(str_v[3]);
			label.setBounds(x, y, w, h);
		} else if ( key.matches("align") ) {
			try {
				GMonLabel label = (GMonLabel)_shape;
				label.getText().setAligment(value);
			} catch(Exception e) {}
		} else if ( key.matches("line.color") ) {
			_shape.setLineColor(new HtmlColor(value));
		} else if ( key.matches("line.width") ) {
			_shape.setLineWidth((int)Double.parseDouble(value));
		} else if ( key.matches("fill.color") ) {
			_shape.setFillColor(new HtmlColor(value));
		} else if ( key.matches("font.size") ) {
			try {
				GMonLabel label = (GMonLabel)_shape;
				label.getText().setFontSize(Double.parseDouble(value));
			} catch(Exception e) {}
		} else if ( key.matches("font.weight") ) {
			GMonLabel label = (GMonLabel)_shape;
			if ( value.matches("normal") ) {
				label.getText().setFontWeight(FontProperty.WEIGHT_NORMAL);
			} else if ( value.matches("bold") ) {
				label.getText().setFontWeight(FontProperty.WEIGHT_BOLD);
			}
		} else if ( key.matches("font.color") ) {
			if (value.matches("none")) _shape.setFontColor(HtmlColor.NULL);
			else _shape.setFontColor(new HtmlColor(value));
		} else if ( key.matches("text.align") ) {
			GMonLabel label = (GMonLabel)_shape;
			label.setTextAlign(value);
		}
	}
	
	public GHisto set(Canvas canvas, HistoPackage pack) {
		return null;
	}

}
