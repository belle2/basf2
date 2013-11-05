package b2dqm.property;

import b2daq.graphics.GObject;
import b2dqm.core.Graph1D;
import b2dqm.core.Histo;
import b2dqm.core.HistoPackage;
import b2dqm.graphics.Canvas;
import b2dqm.graphics.GGraph1;

public class GGraph1Property extends GShapeProperty {

	public final static String ELEMENT_LABEL = "graph";
	
	protected GGraph1 _graph = null;
	
	public GGraph1Property(Canvas canvas) {
		super(canvas);
		_graph = new GGraph1(canvas, new Graph1D());
		canvas.addCShape(_graph);
		_shape = _graph;
	}

	@Override
	public String getType() {
		return ELEMENT_LABEL;
	}

	@Override
	public void put(String key, String value) {
		super.put(key, value);
		if ( key.matches("draw") ) {
			String[] spoint_v = value.split(":");
			_graph.set(new Graph1D("","",spoint_v.length,0,0,0,0));
			for (int n = 0; n < spoint_v.length; n++ ) {
				String [] spoint = spoint_v[n].split(",");
				if ( spoint.length > 1 ) {
					((Histo)_graph.get()).setPointX(n, Double.parseDouble(spoint[0]));
					((Histo)_graph.get()).setPointY(n, Double.parseDouble(spoint[1]));
				}
			}
		}
	}
	
	@Override
	public void put(GProperty pro) {}

	@Override
	public GObject set(Canvas canvas, HistoPackage pack) {
		return super.set(canvas, pack);
	}

}
