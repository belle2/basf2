package b2dqm.io;

import b2daq.graphics.HtmlColor;
import b2daq.java.io.ConfigFile;
import b2dqm.core.Histo;
import b2dqm.graphics.GHisto;

public class GHistoProperty {

	private ConfigFile _config;

	public GHistoProperty(ConfigFile config) {
		_config = config;
	}

	public void put(GHisto histo) {
		put(histo, histo.getName());
	}
	
	public void put(GHisto histo, String name) {
		Histo h = histo.get();
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
		if ( _config.hasKey("line.color") ) {
			histo.setLineColor(new HtmlColor(_config.getString("line.color")));
		}
		if ( _config.hasKey("line.width") ) {
			histo.setLineWidth(_config.getInt("line.width"));
		}
		if ( _config.hasKey("fill.color") ) {
			histo.setFillColor(new HtmlColor(_config.getString("fill.color")));
		}
		if ( _config.hasKey("font.color") ) {
			histo.setFontColor(new HtmlColor(_config.getString("font.color")));
		}
		if ( _config.hasKey("font.size") ) {
			histo.setFontSize(_config.getFloat("font.size"));
		}
		if ( _config.hasKey("font.family") ) {
			histo.setFontFamily(_config.getString("font.family"));
		}
		if ( _config.hasKey("draw.option") ) {
			histo.setDrawOption(_config.getString("draw.option"));
		} 
		_config.cd();
	}
	
}
