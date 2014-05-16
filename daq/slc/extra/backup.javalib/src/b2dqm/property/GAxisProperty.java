package b2dqm.property;

import java.util.ArrayList;

import b2daq.graphics.HtmlColor;
import b2dqm.graphics.Canvas;
import b2dqm.graphics.GAxis;
import b2dqm.graphics.GValuedText;


public class GAxisProperty extends GShapeProperty {

	public final static String ELEMENT_LABEL = "axis";
	
	protected GAxis _axis;

	public GAxisProperty(Canvas canvas) {
		super(canvas);
	}

	@Override
	public String getType() {
		return ELEMENT_LABEL;
	}

	@Override
	public void put(String key, String value) {
		if (_axis == null) return;
		super.put(key, value);
		if ( key.matches("logscale") ) 
			_axis.setLogScale(value.matches("on"));
		if ( key.matches("timeformat") ) 
			_axis.setTimeFormat(value);
		if ( key.matches("name") ) 
			_axis.get().setName(value);
		if ( key.matches("title") ) 
			_axis.get().setTitle(value);
		if ( key.matches("minimum") ) { 
			_axis.get().setMin(Double.parseDouble(value));
			_axis.get().fixMin(true);
		}
		if ( key.matches("maximum") ) { 
			_axis.get().setMax(Double.parseDouble(value));
			_axis.get().fixMax(true);
		}
		if ( key.matches("position") ) { 
			if ( value.matches("center") ) {
				_axis.setTitlePosition(GAxis.POSITION_CENTER);
			} else if ( value.matches("left") ) {
				_axis.setTitlePosition(GAxis.POSITION_LEFT);
			} else if ( value.matches("right") ) {
				_axis.setTitlePosition(GAxis.POSITION_RIGHT);
			}
		}
		if ( key.matches("title.offset") ) {
			_axis.setTitleOffset(Double.parseDouble(value));
		}
		if ( key.matches("label.offset") ) {
			_axis.setLabelOffset(Double.parseDouble(value));
		}
		if ( key.matches("label.font-size") ) {
			_axis.setLabelsFontSize(Double.parseDouble(value));
		}
		if ( key.matches("label.font-color") ) {
			_axis.getLabelsFont().setColor(new HtmlColor(value));
		}
		if ( key.matches("label.font-weight") ) {
			_axis.getLabelsFont().setFamily(value);
		}
		if ( key.matches("label.font-family") ) {
			_axis.getLabelsFont().setFamily(value);
		}
		if ( key.matches("label.angle") ) {
			_axis.setLabelAngle(Double.parseDouble(value));
		}
		if ( key.matches("label.align") ) {
			_axis.setLabelAlignment(value);
		}
		if ( key.matches("label") ) {
			String[] label_pro_v = value.split(":");
			String[] label_v = label_pro_v[0].split(",");
			String[] value_v = label_pro_v[1].split(",");
			ArrayList<GValuedText> glabel_v = new ArrayList<GValuedText>();
			for (int nlabel = 0; nlabel < label_v.length&& nlabel < value_v.length; nlabel++ ) {
				glabel_v.add(new GValuedText(label_v[nlabel], Double.parseDouble(value_v[nlabel]),
						_axis.getLabelAlignment()));
			}
			_axis.setLabels(glabel_v);
		}
		if ( key.matches("tick.length") ) {
			_axis.setTickLength(Double.parseDouble(value));
		}
		if ( key.matches("tick.width") ) {
			_axis.setLineWidth(Integer.parseInt(value));
		}
		if ( key.matches("tick.color") ) {
			_axis.setLineColor(new HtmlColor(value));
		}
		if ( key.matches("tick.ndivisions") ) {
			_axis.setNdivisions(Integer.parseInt(value));
		}
		if ( key.matches("x") ) {
			_axis.setX(Double.parseDouble(value));
		} else if ( key.matches("y") ) {
			_axis.setY(Double.parseDouble(value));
		} else if ( key.matches("length") ) {
			_axis.setLength(Double.parseDouble(value));
		} else if ( key.matches("padding.x") ) {
			_axis.setPaddingX(Double.parseDouble(value));
		} else if ( key.matches("padding.y") ) {
			_axis.setPaddingY(Double.parseDouble(value));
		}
	}
	
}
