package b2daq.dqm.ui.property;

import b2daq.dqm.graphics.GColorAxis;
import b2daq.dqm.graphics.HistogramCanvas;
import javafx.scene.paint.Color;

public class GColorAxisProperty extends GAxisProperty {

    public final static String ELEMENT_LABEL = "color-axis";

    public GColorAxisProperty(HistogramCanvas canvas) {
        super(canvas);
        if (canvas.getColorAxis() == null) {
            canvas.setColorAxis(new GColorAxis(10, 0, 1, "", false, false, false));
        }
        _axis = canvas.getColorAxis();
        _shape = _axis;
    }

    @Override
    public String getType() {
        return ELEMENT_LABEL;
    }

    @Override
    public void put(String key, String value) {
        if (_axis == null) {
            return;
        }
        super.put(key, value);
        if (key.matches("color-pattern")) {
            String[] scolor_v = value.replaceAll(" ", "").split(",");
            if (scolor_v.length > 0) {
                Color[] color_v = new Color[scolor_v.length];
                for (int ncolor = 0; ncolor < scolor_v.length; ncolor++) {
                    color_v[ncolor] = Color.web(scolor_v[ncolor]);
                }
                ((GColorAxis) _axis).setColorPattern(color_v);
            }
        }
    }

}
