package b2daq.core;

public class Log {

	private LogLevel _level = new LogLevel();
	private String _text = "";
	private String _date = new Time().toDateString();
	
	public Log() {}

	public Log(String text) {
		_text = text;
		_level = LogLevel.INFO;
	}

	public Log(String text, LogLevel priority) {
		_text = text;
		_level = priority;
	}

	public Log(String text, LogLevel priority, Time date) {
		_text = text;
		_level = priority;
		_date = date.toDateString();
	}

	public Log(String text, LogLevel priority, long time) {
		_text = text;
		_level = priority;
		_date = new Time(time, 0).toDateString();
	}

	public Log(String text, LogLevel priority, String date) {
		_text = text;
		_level = priority;
		_date = date;
	}

	public void setLevel(LogLevel priority) {
		_level = priority;
	}

	public LogLevel getLevel() {
		return _level;
	}

	public void setText(String text) {
		_text = text;
	}

	public String getText() {
		return _text;
	}

	public void setDate(Time date) {
		_date = date.toDateString();
	}

	public void setDate(String date) {
		_date = date;
	}

	public String getDate() {
		return _date;
	}

	public String toString() {
		return "[" + _level.toString() + "]" 
				+ "[" + _date + "] "
				+ _text.toString() +"\n";
	}

	public String toXML() {
		String text = _text.replace("&", "&amp;").replace("'", "&apos;")
		.replace("\"", "&quot; ").replace(">", "&gt;").replace("<", "&lt;");
		return "<log leve;='" + _level.getLabel() + "' "
			+ "date='" + _date + "' >" + text + "</log>";
	}
	
	public String toHTML() {
		return "<span class='" + _level.getLabel() + "'>"
			+ "[" + _level.getLabel() + "][" + _date + "] "+ _text + "</span>";
	}

}
