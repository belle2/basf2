package b2daq.core;

import b2daq.graphics.HtmlColor;

public class LogLevel {
	
	static final public LogLevel UNDEFINED = new LogLevel(-1, "UNDEFINED", HtmlColor.BLACK, HtmlColor.NULL);
	static final public LogLevel DEBUG = new LogLevel(0, "DEBUG", HtmlColor.GRAY, HtmlColor.NULL);
	static final public LogLevel INFO = new LogLevel(1, "INFO", new HtmlColor(0, 153, 51), HtmlColor.NULL);
	static final public LogLevel NOTICE = new LogLevel(2, "NOTICE", HtmlColor.BLUE, HtmlColor.NULL);
	static final public LogLevel WARNING = new LogLevel(3, "WARNING", HtmlColor.ORANGE, HtmlColor.NULL);
	static final public LogLevel ERROR = new LogLevel(4, "ERROR", HtmlColor.MAGENTA, HtmlColor.NULL);
	static final public LogLevel EXCEPTION = new LogLevel(5, "EXCEPTION", HtmlColor.RED, HtmlColor.NULL);
	static final public LogLevel FATAL = new LogLevel(6, "FATAL", HtmlColor.WHITE, HtmlColor.RED);
	
	static public LogLevel [] LIST = {
		DEBUG, INFO, NOTICE, WARNING, ERROR, EXCEPTION, FATAL
	};
	
	static public LogLevel Get(int n) {
		if ( n < 0 || n > LIST.length ) return UNDEFINED;
		else return LIST[n];
	} 
	
	protected int _level = -1;
	protected String _label = "UNDEFINED";
	private HtmlColor _font_color = HtmlColor.BLACK;
	private HtmlColor _fill_color = HtmlColor.NULL;
		
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
