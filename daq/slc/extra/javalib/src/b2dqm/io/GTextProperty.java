package b2dqm.io;

import b2daq.graphics.GText;
import b2daq.java.io.ConfigFile;

public class GTextProperty extends GShapeProperty {

	public GTextProperty(ConfigFile config) {
		super(config);
	}

	public void put(GText text, String name) {
		_config.cd(name);
		if ( _config.hasKey("x") ) {
			text.setX(_config.getFloat("x"));
		} 
		if ( _config.hasKey("y") ) {
			text.setY(_config.getFloat("y"));
		}
		if ( _config.hasKey("text") ) {
			String value = _config.getString("text").replaceAll("\t", "").replaceAll("\n", "");
			text.setText(value);
		}
		if ( _config.hasKey("align") ) {
			text.setAligment(_config.getString("align"));
		}
		_config.cd();
		super.put(text, name);
	}

}
