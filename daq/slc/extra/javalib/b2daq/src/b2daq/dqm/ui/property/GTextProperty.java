package b2daq.dqm.ui.property;

import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.graphics.GObject;
import b2daq.graphics.GText;
import javafx.scene.paint.Color;
import javafx.scene.text.FontWeight;

public class GTextProperty extends GShapeProperty {

    public final static String ELEMENT_LABEL = "text";

    protected GText _text = new GText();

    public GTextProperty(HistogramCanvas canvas) {
        super(canvas);
        canvas.addShape(_text);
        _shape = _text;
    }

    @Override
    public String getType() {
        return ELEMENT_LABEL;
    }

    @Override
    public void put(String key, String value) {
        super.put(key, value);
        if (key.matches("x")) {
            _text.setX(Double.parseDouble(value));
        } else if (key.matches("y")) {
            _text.setY(Double.parseDouble(value));
        } else if (key.matches("value")) {
            value = value.replaceAll("\t", "").replaceAll("\n", "");
            _text.setText(value);
        } else if (key.matches("align")) {
            _text.setAlignment(value);
        } else if (key.matches("font-weight")) {
            if (value.matches("bold")) {
                _text.setFontWeight(FontWeight.BOLD);
            }
            if (value.matches("normal")) {
                _text.setFontWeight(FontWeight.NORMAL);
            }
        } else if (key.matches("font-color")) {
            _text.setFontColor(Color.web(value));
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
