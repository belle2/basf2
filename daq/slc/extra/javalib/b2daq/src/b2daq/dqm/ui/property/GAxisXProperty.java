package b2daq.dqm.ui.property;

import b2daq.dqm.graphics.HistogramCanvas;

public class GAxisXProperty extends GAxisProperty {

    public final static String ELEMENT_LABEL = "x-axis";

    public GAxisXProperty(HistogramCanvas canvas) {
        super(canvas);
        _axis = canvas.getAxisX();
        _shape = _axis;
    }

    @Override
    public String getType() {
        return ELEMENT_LABEL;
    }

}
