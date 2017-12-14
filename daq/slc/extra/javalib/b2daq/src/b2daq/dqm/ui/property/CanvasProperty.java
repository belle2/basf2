package b2daq.dqm.ui.property;

import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.graphics.FillProperty;
import java.util.ArrayList;
import javafx.scene.paint.Color;

public class CanvasProperty extends GRectProperty {

    public final static String ELEMENT_LABEL = "canvas";

    private HistogramCanvas m_canvas = null;
    private ArrayList<GProperty> _pro_v = new ArrayList<GProperty>();

    public CanvasProperty(HistogramCanvas canvas) {
        super(canvas);
        m_canvas = canvas;
    }

    @Override
    public String getType() {
        return ELEMENT_LABEL;
    }

    @Override
    public void put(String key, String value) {
        if (m_canvas == null) {
            return;
        }
        if (key.matches("fill-color")) {
            FillProperty fill = m_canvas.getFill();
            if (value.matches("none")) {
                fill.setColor(Color.TRANSPARENT);
            } else {
                fill.setColor(Color.web(value));
            }
        } else {
            super.put(key, value);
        }
        if (key.matches("name")) {
            m_canvas.setName(value);
        }
        if (key.matches("title")) {
            m_canvas.setTitle(value);
        }
        if (key.matches("title-offset")) {
            double offset = Double.parseDouble(value);
            m_canvas.setTitleOffset(offset);
        }
        if (key.matches("title-position")) {
            m_canvas.setTitlePosition(value);
        }
    }

    @Override
    public void put(GProperty pro) {
        _pro_v.add(pro);
    }

    public void setup(HistoPackage pack) {
        for (GProperty pro: _pro_v) {
            pro.set(m_canvas, pack);
        }
        m_canvas.resetPadding();
    }

}
