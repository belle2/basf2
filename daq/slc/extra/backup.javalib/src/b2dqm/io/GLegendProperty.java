package b2dqm.io;

import b2daq.java.io.ConfigFile;
import b2dqm.graphics.GLegend;

public class GLegendProperty extends GRectProperty {

	private String [] _name_v;
	
	public GLegendProperty(ConfigFile config) {
		super(config);
	}

	public void put(GLegend legend, String name) {
		if (legend == null) return;
		_config.cd(name);
		if ( _config.hasKey("objects") ) {
			_name_v = _config.getString("objects").split(",");
			for ( int n = 0; n< _name_v.length; n++ ) {
				_name_v[n] = _name_v[n].replaceAll(" ", "").replaceAll("\t", "");
			}
		}
		_config.cd();
		super.put(legend, name);
	}
	
	public String [] getNames() {
		return _name_v;
	}
	
}
