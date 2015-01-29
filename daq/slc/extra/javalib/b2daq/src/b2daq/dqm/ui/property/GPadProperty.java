package b2daq.dqm.ui.property;

import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.dqm.graphics.Pad;

public class GPadProperty extends GRectProperty {

    public final static String ELEMENT_LABEL = "pad";

    private Pad _pad = null;

    public GPadProperty(HistogramCanvas canvas) {
        super(canvas);
        _pad = canvas.getPad();
        _rect = _pad;
        _shape = _pad;
    }

    @Override
    public String getType() {
        return ELEMENT_LABEL;
    }

}
