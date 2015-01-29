package b2daq.dqm.ui.property;

import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.graphics.GObject;
import b2daq.graphics.GShape;
import javafx.scene.paint.Color;
import javafx.scene.text.FontWeight;

public abstract class GShapeProperty extends GProperty {

    protected GShape _shape = null;

    public GShapeProperty(HistogramCanvas canvas) {
        super(canvas);
    }

    @Override
    public void put(String key, String value) {
        if (key.matches("name")) {
            if (value.length() > 0) {
                _shape.setName(value);
            }
        } else if (key.matches("line-color")) {
            if (value.matches("none")) {
                _shape.setLineColor(Color.TRANSPARENT);
            } else {
                _shape.setLineColor(Color.web(value));
            }
        } else if (key.matches("line-width")) {
            _shape.setLineWidth(Integer.parseInt(value));
        } else if (key.matches("fill-color")) {
            if (value.matches("none")) {
                _shape.setFillColor(Color.TRANSPARENT);
            } else {
                _shape.setFillColor(Color.web(value));
            }
        } else if (key.matches("font-color")) {
            if (value.matches("none")) {
                _shape.setFontColor(Color.TRANSPARENT);
            } else {
                _shape.setFontColor(Color.web(value));
            }
        } else if (key.matches("font-size")) {
            _shape.setFontSize(Double.parseDouble(value));
        } else if (key.matches("font-family")) {
            _shape.setFontFamily(value);
        } else if (key.matches("font-weight")) {
            if (value.matches("normal")) {
                _shape.setFontWeight(FontWeight.NORMAL);
            } else if (value.matches("bold")) {
                _shape.setFontWeight(FontWeight.BOLD);
            }
        } else if (key.matches("visible")) {
            if (value.matches("false")) {
                _shape.setVisible(false);
            } else if (value.matches("true")) {
                _shape.setVisible(false);
            }
        }
    }

    public GObject set(HistogramCanvas canvas, HistoPackage pack) {
        return _shape;
    }

}
