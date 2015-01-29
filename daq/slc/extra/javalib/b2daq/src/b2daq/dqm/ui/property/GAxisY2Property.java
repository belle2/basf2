package b2daq.dqm.ui.property;

import b2daq.dqm.graphics.GAxisY;
import b2daq.dqm.graphics.HistogramCanvas;

public class GAxisY2Property extends GAxisProperty {

    public final static String ELEMENT_LABEL = "y2-axis";

    public GAxisY2Property(HistogramCanvas canvas) {
        super(canvas);
        _axis = new GAxisY();
        canvas.setAxisY2(_axis);
        _shape = _axis;
    }

    @Override
    public String getType() {
        return ELEMENT_LABEL;
    }

}
