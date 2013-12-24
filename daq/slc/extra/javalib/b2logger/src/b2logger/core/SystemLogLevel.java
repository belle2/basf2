package b2logger.core;

import b2daq.core.LogLevel;
import b2daq.graphics.HtmlColor;

public class SystemLogLevel extends LogLevel {
	
	static final public SystemLogLevel UNDEFINED = new SystemLogLevel(0, "UNDEFINED", HtmlColor.BLACK, HtmlColor.NULL);
	static final public SystemLogLevel DEBUG = new SystemLogLevel(1, "DEBUG", HtmlColor.GRAY, HtmlColor.NULL);
	static final public SystemLogLevel INFO = new SystemLogLevel(2, "INFO", new HtmlColor(0, 204, 102), HtmlColor.NULL);
	static final public SystemLogLevel NOTICE = new SystemLogLevel(3, "NOTICE", HtmlColor.BLUE, HtmlColor.NULL);
	static final public SystemLogLevel WARNING = new SystemLogLevel(4, "WARNING", HtmlColor.ORANGE, HtmlColor.NULL);
	static final public SystemLogLevel ERROR = new SystemLogLevel(5, "ERROR", HtmlColor.MAGENTA, HtmlColor.NULL);
	static final public SystemLogLevel FATAL = new SystemLogLevel(6, "FATAL", HtmlColor.RED, HtmlColor.NULL);
	
	static public SystemLogLevel [] LogLevelList = {
		UNDEFINED, DEBUG, INFO, NOTICE, WARNING, ERROR, FATAL
	};
	
	static public SystemLogLevel Get(int n) {
		n = n % 10;
		if ( n <= 0 || n > LogLevelList.length ) return UNDEFINED;
		else return LogLevelList[n];
	} 
	
	public SystemLogLevel() {
		super(-1, "UNDEFINED");
	}
	
	private SystemLogLevel(int level, String label, HtmlColor font_color, HtmlColor fill_color) {
		super(level, label, font_color, fill_color);
	}

}
