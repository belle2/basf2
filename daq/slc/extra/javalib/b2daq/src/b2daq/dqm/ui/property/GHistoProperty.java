package b2daq.dqm.ui.property;

import b2daq.dqm.core.Histo;
import b2daq.dqm.core.Histo1F;
import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.core.MonObject;
import b2daq.dqm.graphics.GAxis;
import b2daq.dqm.graphics.HistogramCanvas;
import javafx.scene.paint.Color;

public class GHistoProperty extends GProperty {

    public final static String ELEMENT_LABEL = "histo";

    protected String _linked_axis_name = "";

    private Histo _histo = null;

    public GHistoProperty(HistogramCanvas canvas, HistoPackage pack, String name) {
        super(canvas);
        for (MonObject histo : pack.getMonObjects()) {
            if (name.matches(histo.getName())) {
                try {
                    _histo = (Histo) histo;
                    canvas.addHisto(_histo);
                } catch (Exception e) {
                    _histo = null;

                }
                break;
            }
        }
        if (_histo == null) {
            _histo = new Histo1F("__", "", 10, 0, 1);
            canvas.addHisto(_histo);
        }
    }

    @Override
    public String getType() {
        return ELEMENT_LABEL;
    }

    @Override
    public void put(String key, String value) {
        Histo h = _histo;
        value = parseUnicode(value);
        if (key.matches("title")) {
            h.setTitle(value);
        } else if (key.matches("maximum")) {
            h.fixMaximum(Double.parseDouble(value), true);
        } else if (key.matches("minimum")) {
            h.fixMinimum(Double.parseDouble(value), true);
        } else if (key.matches("line-color")) {
            _histo.setLineColor(Color.web(value));
        } else if (key.matches("line-width")) {
            _histo.setLineWidth((int) Double.parseDouble(value));
        } else if (key.matches("fill-color")) {
            _histo.setFillColor(Color.web(value));
        } else if (key.matches("font-color")) {
            _histo.setFontColor(Color.web(value));
        } else if (key.matches("font-size")) {
            _histo.setFontSize(Double.parseDouble(value));
        } else if (key.matches("font-family")) {
            _histo.setFontFamily(value);
        } else if (key.matches("draw-option")) {
            _histo.setDraw(value);
        } else if (key.matches("linked-axis")) {
            _linked_axis_name = value;
        }
    }

    @Override
    public Histo set(HistogramCanvas canvas, HistoPackage pack) {
        if (_histo != null) {
            if (_linked_axis_name.length() > 0) {
                GAxis axis = canvas.getAxisX();
                if (axis.getName().matches(_linked_axis_name)) {
                    _histo.setAxisY(axis.get());
                }
                axis = canvas.getAxisY();
                if (axis.getName().matches(_linked_axis_name)) {
                    _histo.setAxisY(axis.get());
                }
                axis = canvas.getAxisY2();
                if (axis != null && axis.getName().matches(_linked_axis_name)) {
                    _histo.setAxisY(axis.get());
                }
                axis = canvas.getColorAxis();
                if (axis != null && axis.getName().matches(_linked_axis_name)) {
                    _histo.setAxisY(axis.get());
                }
            }
        }
        return _histo;
    }

}
