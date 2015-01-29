package b2daq.dqm.ui.property;

import b2daq.dqm.core.Graph1;
import b2daq.dqm.core.Graph1D;
import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.graphics.GObject;

public class GGraph1Property extends GShapeProperty {

    public final static String ELEMENT_LABEL = "graph";

    protected Graph1 _graph = null;

    public GGraph1Property(HistogramCanvas canvas) {
        super(canvas);
        _graph = new Graph1D();
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
        if (key.matches("draw")) {
            String[] spoint_v = value.split(":");
            _graph = new Graph1D("", "", spoint_v.length, 0, 0, 0, 0);
            for (int n = 0; n < spoint_v.length; n++) {
                String[] spoint = spoint_v[n].split(",");
                if (spoint.length > 1) {
                    _graph.setPointX(n, Double.parseDouble(spoint[0]));
                    _graph.setPointY(n, Double.parseDouble(spoint[1]));
                }
            }
        }
    }

    @Override
    public void put(GProperty pro) {
    }

    @Override
    public GObject set(HistogramCanvas canvas, HistoPackage pack) {
        return super.set(canvas, pack);
    }

}
