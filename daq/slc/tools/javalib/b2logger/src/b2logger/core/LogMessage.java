package b2logger.core;

import b2daq.core.Reader;
import b2daq.core.Time;

public class LogMessage {

	private String _host_name;
	private String _node_name;
	private String _message;
	private SystemLogLevel _level;
	private Time _time;
	private int _id;
	
	public LogMessage() {
		this("", "", SystemLogLevel.DEBUG, "NO Message");
	}
	
	public LogMessage(String host, String node_name, SystemLogLevel level, String msg) {
		_host_name = host;
		_node_name = node_name;
		_level = level;
		_time = new Time();
		setMessage(msg);
	}

	public LogMessage(LogMessage msg) {
		this(msg.getHostName(), msg.getNodeName(), msg.getLogLevel(), msg.getMessage());
	}

	public LogMessage(String host, SystemLogLevel level) {
		this(host, "", level, "");
	}

	public LogMessage(String host) {
		this(host, "", SystemLogLevel.UNDEFINED, "");
	}

	public void setHostName(String name) {
		_host_name = name;
	}

	public String getHostName() {
		return _host_name;
	}

	public void setNodeName(String name) {
		_node_name = name;
	}

	public String getNodeName() {
		return _node_name;
	}

	public void setMessage(String msg) {
		_message = msg;
	}

	public String getMessage() {
		return _message;
	}

	public SystemLogLevel getLogLevel() {
		return _level;
	}
	
	public void setLogLevel(SystemLogLevel level) {
		_level = level;
	}

	public void readObject(Reader reader) throws Exception {
		int npar = reader.readInt();
		int [] pars = new int [npar];
		for ( int i = 0; i < npar; i++ ) {
			pars[i] = reader.readInt();
		}
		_level = SystemLogLevel.Get(pars[0]);
		_time.set(((long)pars[1]) * 1000, 0);
		String [] str_v = reader.readString().split(";");
		if ( str_v.length < 3 ) return ;
		setHostName(str_v[0]);
		setNodeName(str_v[1]);
		String message = str_v[2];
		for ( int n = 3; n < str_v.length; n++ ) {
			message += ";" + str_v[n];
		}
		setMessage(message);
	}

	public String toString() {
		return "Log ID : "+ _id + ", time:" + _time.toString() + "\n"
			+ "LogLevel : " + _level.toString() +"\n" 
			+ "From : " + _host_name +"\n"
			+ "Message : " + getMessage();
	}
	
	public String toHTML() {
		return "<center> Logger " + _level.toString() +"</center>"
			+ "From : " + _host_name +"<br />"
			+ "Time:" + _time.toDateString()+ "<br />" 
			+ "Message : <br /> "
			+ getMessage();
	}
	
	public String toGaibu() {
		return _host_name + " " + _level.getLevel() + " " + 
			_time.getSecond() + "." + "000000 " + getMessage();
	}
	public Time getTime() { return _time; }
}
