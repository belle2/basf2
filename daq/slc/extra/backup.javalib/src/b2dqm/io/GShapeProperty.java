package b2dqm.io;

import b2daq.graphics.FontProperty;
import b2daq.graphics.GShape;
import b2daq.graphics.HtmlColor;
import b2daq.java.io.ConfigFile;

public abstract class GShapeProperty extends GProperty {

	public GShapeProperty(ConfigFile config) {
		super(config);
	}

	public void put(GShape shape) {
		put(shape, shape.getName());
	}
	
	public void put(GShape shape, String name) {
		_config.cd(name);
		if ( _config.hasKey("line.color") ) {
			String value = _config.getString("line.color");
			if (value.matches("none")) shape.setLineColor(HtmlColor.NULL);
			else shape.setLineColor(new HtmlColor(value));
		}
		if ( _config.hasKey("line.width") ) {
			shape.setLineWidth(_config.getInt("line.width"));
		}
		if ( _config.hasKey("fill.color") ) {
			String value = _config.getString("fill.color");
			if (value.matches("none")) shape.setFillColor(HtmlColor.NULL);
			else shape.setFillColor(new HtmlColor(value));
		}
		if ( _config.hasKey("font.color") ) {
			String value = _config.getString("font.color");
			if (value.matches("none")) shape.setFontColor(HtmlColor.NULL);
			else shape.setFontColor(new HtmlColor(value));
		}
		if ( _config.hasKey("font.size") ) {
			shape.setFontSize(_config.getFloat("font.size"));
		}
		if ( _config.hasKey("font.family") ) {
			String value = _config.getString("font.family");
			shape.setFontFamily(value);
		}
		if ( _config.hasKey("font.weight") ) {
			String value = _config.getString("font.weight");
			if ( value.matches("normal") ) {
				shape.setFontWeight(FontProperty.WEIGHT_NORMAL);
			} else if ( value.matches("bold") ) {
				shape.setFontWeight(FontProperty.WEIGHT_BOLD);
			}
		}
		if ( _config.hasKey("visible") ) {
			String value = _config.getString("visible");
			if ( value.matches("false")) {
				shape.setVisible(false);
			} else if ( value.matches("true")) {
				shape.setVisible(false);
			}
		}
		_config.cd();
	}

}
