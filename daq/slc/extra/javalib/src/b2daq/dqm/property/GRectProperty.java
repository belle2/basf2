package b2daq.dqm.property;

import b2daq.graphics.GRect;
import b2daq.io.ConfigFile;

public class GRectProperty extends GShapeProperty {

	public GRectProperty(ConfigFile config) {
		super(config);
	}

	public void put(GRect rect) {
		put(rect, rect.getName());
	}
	
	public void put(GRect rect, String name) {
		super.put(rect, name);
		_config.cd(name);
		if ( _config.hasKey("x") ) {
			rect.setX(_config.getFloat("x"));
		}
		if ( _config.hasKey("y") ) {
			rect.setY(_config.getFloat("y"));
		}
		if ( _config.hasKey("width") ) {
			rect.setWidth(_config.getFloat("width"));
		}
		if ( _config.hasKey("height") ) {
			rect.setHeight(_config.getFloat("height"));
		}
		_config.cd();
	}
	
}
