package org.belle2.daq.base;

import org.belle2.daq.graphics.HtmlColor;

public class LogLevel {
	
	static final public LogLevel UNDEFINED = new LogLevel(0, "UNDEFINED", HtmlColor.BLACK, null);
	static final public LogLevel DEBUG = new LogLevel(1, "DEBUG", HtmlColor.GRAY, null);
	static final public LogLevel INFO = new LogLevel(2, "INFO", new HtmlColor(0, 153, 51), null);
	static final public LogLevel NOTICE = new LogLevel(3, "NOTICE", HtmlColor.BLUE, null);
	static final public LogLevel WARNING = new LogLevel(4, "WARNING", HtmlColor.ORANGE, null);
	static final public LogLevel ERROR = new LogLevel(5, "ERROR", HtmlColor.MAGENTA, null);
	static final public LogLevel FATAL = new LogLevel(6, "FATAL", HtmlColor.RED, null);
	
	static public LogLevel [] LIST = {
		UNDEFINED, DEBUG, INFO, NOTICE, WARNING, ERROR, FATAL
	};
	
	static public LogLevel Get(int n) {
		if ( n < 0 || n > LIST.length ) return UNDEFINED;
		else return LIST[n];
	} 
	
	protected int _level = -1;
	protected String _label = "UNDEFINED";
	private HtmlColor _font_color = HtmlColor.BLACK;
	private HtmlColor _fill_color = null;
		
	public LogLevel() {}
	
	protected LogLevel(int level, String label) {
		_level = level;
		_label = label;
	}

	protected LogLevel(int level, String label, HtmlColor font_color) {
		_level = level;
		_label = label;
		_font_color = font_color;
	}

	protected LogLevel(int level, String label, HtmlColor font_color, HtmlColor fill_color) {
		_level = level;
		_label = label;
		_font_color = font_color;
		_fill_color = fill_color;
	}

	public int getLevel() {
		return _level;
	}

	public String getLabel() {
		return _label;
	}

	public void copy(LogLevel level) {
		_level = level.getLevel();
		_label = level.getLabel();
	}
	
    public boolean hiegher(LogLevel level) {
    	return (_level >= level._level);
    }

    public String toString() {
		return _label;
	}

	public void setFontColor(HtmlColor color) {
		_font_color = color;
	}

	public HtmlColor getFontColor() {
		return _font_color;
	}

	public void setFillColor(HtmlColor color) {
		_fill_color = color;
	}

	public HtmlColor getFillColor() {
		return _fill_color;
	}

}
