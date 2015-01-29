package b2daq.dqm.ui.property;

import b2daq.dqm.core.HistoPackage;
import b2daq.dqm.graphics.HistogramCanvas;
import b2daq.graphics.GObject;

public abstract class GProperty {

    static public String parseUnicode(String code) {
        String str = code.replace("&lt;", "<");
        str = str.replace("&gt;", ">");
        str = str.replace("&quot;", "\"");
        str = str.replace("&apos;", "'");
        str = str.replace("&amp;", "&");
        return str;
    }

    public GProperty(HistogramCanvas canvas) {
    }

    abstract public String getType();

    abstract public GObject set(HistogramCanvas canvas, HistoPackage pack);

    abstract public void put(String key, String value);

    public void put(GProperty pro) {
    }

}
