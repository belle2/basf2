package b2daq.dqm.property;

import b2daq.io.ConfigFile;

public abstract class GProperty {

	static public String parseUnicode(String code) {
		String str = code.replace("&lt;", "<");
		str = str.replace("&gt;",">");
	    str = str.replace("&quot;", "\"");
	    str = str.replace("&apos;", "'");
	    str = str.replace("&amp;", "&");
		return str;
	}
	
	protected ConfigFile _config;
	
	public GProperty(ConfigFile config) {
		_config = config;
	}

}
