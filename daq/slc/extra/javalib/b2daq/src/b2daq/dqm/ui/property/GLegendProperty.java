package b2daq.dqm.ui.property;

import b2daq.dqm.core.Histo;
import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.dqm.graphics.Legend;
import b2daq.graphics.GObject;

public class GLegendProperty extends GRectProperty {

    public final static String ELEMENT_LABEL = "legend";

    private Legend _legend = null;
    private String[] _name_v = null;

    public GLegendProperty(HistogramCanvas canvas) {
        super(canvas);
        _legend = new Legend();
        canvas.setLegend(_legend);
        _rect = _legend;
        _shape = _legend;
    }

    @Override
    public void put(String key, String value) {
        if (_legend == null) {
            return;
        }
        super.put(key, value);
        if (key.matches("list")) {
            _name_v = value.split(",");
            for (int n = 0; n < _name_v.length; n++) {
                _name_v[n] = _name_v[n].replaceAll(" ", "").replaceAll("\t", "");
            }
        }
    }

    @Override
    public GObject set(HistogramCanvas canvas, HistoPackage pack) {
        if (_legend != null && _name_v != null) {
            for (Histo hist : canvas.getHistograms()) {
                for (String name : _name_v) {
                    try {
                        if (name.matches(hist.getName())) {
                            _legend.add(hist);
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }

        }
        return _legend;
    }

    @Override
    public String getType() {
        return ELEMENT_LABEL;
    }

}
