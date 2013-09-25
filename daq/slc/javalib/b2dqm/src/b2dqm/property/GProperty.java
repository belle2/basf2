package b2dqm.property;

import b2daq.graphics.GObject;
import b2dqm.core.HistoPackage;
import b2dqm.graphics.Canvas;

public abstract class GProperty {

	static public String parseUnicode(String code) {
		String str = code.replace("&lt;", "<");
		str = str.replace("&gt;",">");
	    str = str.replace("&quot;", "\"");
	    str = str.replace("&apos;", "'");
	    str = str.replace("&amp;", "&");
		return str;
	}
	
	public GProperty(Canvas canvas) {}

	abstract public String getType();

	abstract public GObject set(Canvas canvas, HistoPackage pack);

	abstract public void put(String key, String value);

	public void put(GProperty pro) {}

}
