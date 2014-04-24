package b2dqm.property;

import java.util.ArrayList;
import java.util.Iterator;

import b2daq.graphics.FillProperty;
import b2daq.graphics.HtmlColor;
import b2dqm.core.HistoPackage;
import b2dqm.graphics.Canvas;

public class CanvasProperty extends GRectProperty {

	public final static String ELEMENT_LABEL = "canvas";

	private Canvas _canvas = null;
	private ArrayList<GProperty> _pro_v = new ArrayList<GProperty>();

	public CanvasProperty(Canvas canvas) {
		super(canvas);
		_canvas = canvas;
	}

	@Override
	public String getType () {
		return ELEMENT_LABEL;
	}
	
	@Override
	public void put(String key, String value) {
		if ( _canvas == null ) return;
		if ( key.matches("fill.color") ) {
			FillProperty fill = _canvas.getFill();
			if (value.matches("none")) fill.setColor(HtmlColor.NULL);
			else fill.setColor(new HtmlColor(value));
		} else {
			super.put(key, value);
		}
		if ( key.matches("name") ) {
			_canvas.setName(value);
		}
		if ( key.matches("title") ) {
			_canvas.setTitle(value);
		}
		if ( key.matches("title.offset") ) {
			double offset = Double.parseDouble(value);
			_canvas.setTitleOffset(offset);
		}
		if ( key.matches("title.position") ) {
			_canvas.setTitlePosition(value);
		}
	}
	
	@Override
	public void put(GProperty pro) {
		_pro_v.add(pro);
	}

	public void setup(HistoPackage pack) {
		Iterator<GProperty> it = _pro_v.iterator();
		while ( it.hasNext() ) it.next().set(_canvas, pack);
		_canvas.resetPadding();
	}

}
