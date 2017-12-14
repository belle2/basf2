package b2daq.dqm.ui.property;

import b2daq.dqm.graphics.HistogramCanvas;

public class GAxisYProperty extends GAxisProperty {

    public final static String ELEMENT_LABEL = "y-axis";

    public GAxisYProperty(HistogramCanvas canvas) {
        super(canvas);
        _axis = canvas.getAxisY();
        _shape = _axis;
    }

    @Override
    public String getType() {
        return ELEMENT_LABEL;
    }

}
