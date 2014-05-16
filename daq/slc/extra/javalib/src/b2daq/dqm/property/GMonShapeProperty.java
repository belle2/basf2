package b2daq.dqm.property;

import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.graphics.Canvas;
import b2daq.dqm.graphics.GHisto;
import b2daq.dqm.graphics.GMonLabel;
import b2daq.dqm.graphics.GMonShape;
import b2daq.io.ConfigFile;

public class GMonShapeProperty extends GShapeProperty {

	public GMonShapeProperty(ConfigFile config) {
		super(config);
	}

	public void put(GMonShape shape) {
		put(shape, shape.getName());
	}
	
	public void put(GMonShape shape, String name) {
		if ( shape == null ) return;
		_config.cd(name);
		if ( _config.hasKey("bounds") ) {
			GMonLabel label = (GMonLabel)shape;
			String [] str_v = _config.getString("bounds").split(",");
			double x = Double.parseDouble(str_v[0]);
			double y = Double.parseDouble(str_v[1]);
			double w = Double.parseDouble(str_v[2]);
			double h = Double.parseDouble(str_v[3]);
			label.setBounds(x, y, w, h);
		} else if ( _config.hasKey("align") ) {
			try {
				String value = _config.getString("align");
				GMonLabel label = (GMonLabel)shape;
				label.getText().setAligment(value);
			} catch(Exception e) {}
		} else if ( _config.hasKey("text.align") ) {
			GMonLabel label = (GMonLabel)shape;
			label.setTextAlign(_config.getString("text.align"));
		}
		_config.cd();
		super.put(shape, name);
	}
	
	public GHisto set(Canvas canvas, HistoPackage pack) {
		return null;
	}

}
