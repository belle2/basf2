package b2daq.dqm.property;

import b2daq.dqm.graphics.Canvas;
import b2daq.dqm.graphics.GHisto;
import b2daq.dqm.graphics.GLegend;
import b2daq.dqm.graphics.GMonObject;
import b2daq.graphics.HtmlColor;
import b2daq.io.ConfigFile;

public class CanvasProperty extends GRectProperty {

	public CanvasProperty(ConfigFile config) {
		super(config);
	}

	public void put(Canvas canvas) {
		put(canvas, canvas.getName());
	}
	
	public void put(Canvas canvas, String name) {
		if (canvas == null) return;
		super.put(canvas, name);
		_config.cd(name);
		if (_config.hasKey("fill.color")) {
			String value = _config.getString("fill.color");
			if (value.matches("none")) canvas.setFillColor(HtmlColor.NULL);
			else canvas.setFillColor(new HtmlColor(value));
		}
		if (_config.hasKey("title")) {
			canvas.setTitle(_config.getString("title"));
		}
		if (_config.hasKey("title.offset")) {
			canvas.setTitleOffset(_config.getFloat("title.offset"));
		}
		if (_config.hasKey("title.position")) {
			canvas.setTitleOffset(_config.getFloat("title.position"));
		}
		if (_config.hasKey("legend")) {
			name = _config.getString("legend");
			GLegend legend = new GLegend();
			_config.cd();
			_config.cd(name);
			String [] name_v = _config.getString("objects").split(",");
			for (String subname : name_v) {
				for (GMonObject obj : canvas.getMonObjects()) {
					try {
						GHisto histo = (GHisto)obj;
						if (subname.matches(histo.get().getName())) {
							legend.add(histo);
						}
					} catch (Exception e) {
					}
				}
			}
			canvas.setLegend(legend);
		}
		_config.cd();
	}
	
}
