package b2daq.dqm.ui.property;

import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.graphics.GCircle;
import b2daq.graphics.GObject;

public class GCircleProperty extends GShapeProperty {

    public final static String ELEMENT_LABEL = "circle";

    private GCircle _circle = new GCircle();

    public GCircleProperty(HistogramCanvas canvas) {
        super(canvas);
        canvas.addShape(_circle);
        _shape = _circle;
    }

    @Override
    public String getType() {
        return ELEMENT_LABEL;
    }

    @Override
    public void put(String key, String value) {
        super.put(key, value);
        if (key.matches("x")) {
            _circle.setX(Double.parseDouble(value));
        } else if (key.matches("y")) {
            _circle.setY(Double.parseDouble(value));
        } else if (key.matches("rx")) {
            _circle.setRX(Double.parseDouble(value));
        } else if (key.matches("ry")) {
            _circle.setRY(Double.parseDouble(value));
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
