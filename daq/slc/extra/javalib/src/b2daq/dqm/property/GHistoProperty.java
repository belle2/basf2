package b2daq.dqm.property;

import b2daq.dqm.core.Histo;
import b2daq.dqm.graphics.GHisto;
import b2daq.io.ConfigFile;

public class GHistoProperty extends GShapeProperty {

	public GHistoProperty(ConfigFile config) {
		super(config);
	}

	public void put(GHisto histo) {
		put(histo, histo.getName());
	}
	
	public void put(GHisto histo, String name) {
		Histo h = histo.get();
		super.put(histo, name);
		_config.cd(name);
		if ( _config.hasKey("title") ) {
			h.setTitle(_config.getString("title"));
		}
		if ( _config.hasKey("maximum") ) {
			h.fixMaximum(_config.getFloat("maximum"), true);
		}
		if ( _config.hasKey("minimum") ) {
			h.fixMinimum(_config.getFloat("minimum"), true);
		}
		if ( _config.hasKey("draw.option") ) {
			histo.setDrawOption(_config.getString("draw.option"));
		} 
		_config.cd();
	}
	
}
