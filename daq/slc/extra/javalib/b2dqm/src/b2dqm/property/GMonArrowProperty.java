package b2dqm.property;

import b2daq.graphics.GArrow;
import b2daq.graphics.HtmlColor;
import b2dqm.core.HistoPackage;
import b2dqm.core.MonObject;
import b2dqm.graphics.Canvas;
import b2dqm.graphics.GHisto;
import b2dqm.graphics.GMonArrow;

public class GMonArrowProperty extends GProperty {

	public final static String ELEMENT_LABEL = "mon-arrow";

	private GMonArrow _shape = null;
	
	public GMonArrowProperty(Canvas canvas, HistoPackage pack, String name) {
		super(canvas);
		for ( MonObject histo : pack.getMonObjects() ) {
			if ( name.matches(histo.getName()) ) {
				_shape = (GMonArrow)canvas.addHisto(histo);
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
		try {
			if ( key.matches("position.x") ) {
				_shape.getArrow().setPositionX(Double.parseDouble(value));
			} else if ( key.matches("position.y") ) {
				_shape.getArrow().setPositionY(Double.parseDouble(value));
			} else if ( key.matches("length") ) {
				_shape.getArrow().setLength(Double.parseDouble(value));
			} else if ( key.matches("width") ) {
				_shape.getArrow().setWidth(Double.parseDouble(value));
			} else if ( key.matches("head.width") ) {
				_shape.getArrow().setHeadWidth(Double.parseDouble(value));
			} else if ( key.matches("head.length") ) {
				_shape.getArrow().setHeadLength(Double.parseDouble(value));
			} else if ( key.matches("direction") ) {
				if ( value.matches("up") || value.matches(""+GArrow.DIRECT_UP))
					_shape.getArrow().setDirection(GArrow.DIRECT_UP);
				else if ( value.matches("down") || value.matches(""+GArrow.DIRECT_DOWN))
					_shape.getArrow().setDirection(GArrow.DIRECT_DOWN);
				else if ( value.matches("left") || value.matches(""+GArrow.DIRECT_LEFT))
					_shape.getArrow().setDirection(GArrow.DIRECT_RIGHT);
				else if ( value.matches("right") || value.matches(""+GArrow.DIRECT_RIGHT))
					_shape.getArrow().setDirection(GArrow.DIRECT_LEFT);
			} else if ( key.matches("line.color") ) {
				_shape.getArrow().setLineColor(new HtmlColor(value));
			} else if ( key.matches("line.width") ) {
				_shape.getArrow().setLineWidth((int)Double.parseDouble(value));
			} else if ( key.matches("fill.color") ) {
				_shape.getArrow().setFillColor(new HtmlColor(value));
			}
		} catch (Exception e) {}
	}
	
	public GHisto set(Canvas canvas, HistoPackage pack) {
		return null;
	}

}
