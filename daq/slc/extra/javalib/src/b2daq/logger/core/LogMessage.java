package b2daq.logger.core;

import java.util.Date;

import b2daq.database.ConfigObject;

public class LogMessage extends ConfigObject { 

	public LogMessage() {
		this("", "", SystemLogLevel.DEBUG, "");
	}
	
	public LogMessage(String nodename, String groupname,
				SystemLogLevel priority, String message) {
		setConfig(false);
		addInt("date", (int)new Date().getTime());
		addText("groupname", groupname);
		addText("nodename", nodename);
		addEnumList("priority", "DEBUG,INFO,NOTICE,WARNING,ERROR,FATAL");
		addEnum("priority", "");
		setPriority(priority);
		addText("message", message);
	}
	
	public LogMessage(String nodename, String groupname, SystemLogLevel level) {
		this(nodename, groupname, level, "");
	}

	public void setPriority(SystemLogLevel priority)	{
		setEnum("priority", (int)priority.getLevel());
	}

	public void setGroupName(String name) {
		setText("groupname", name);
	}

	public void setNodeName(String name) {
		setText("nodename", name);
	}

	public void setMessage(String message) {
		setText("message", message);
	}

	public void setDate() {
		setDate(new Date());
	}

	public void setDate(int date) {
		setInt("date", date);
	}

	public void setDate(Date date)	{
		setDate((int)date.getTime());
	}

	public SystemLogLevel getPriority() {
		return SystemLogLevel.LogLevelList[getInt("priority")];
	}
	
	public String getNodeName() {
		return getText("nodename");
	}

	public String getGroupName() {
		return getText("groupname");
	}

	public String getMessage() {
		return getText("message");
	}

	public int getDateInt() {
		return getInt("date");
	}

	public Date getDate() {
		return new Date(getInt("date"));
	}

	public String toString() {
		return "Log ID : "+ getId() + ", time:" + getDate().toString() + "\n"
		+ "LogLevel : " + getPriority().toString() +"\n" 
		+ "Group    : " + getGroupName() +"\n"
		+ "Node     : " + getNodeName()+"\n"
		+ "Message  : " + getMessage();
	}
	
	public String toHTML() {
		return "<center> Logger " + getPriority().toString() +"</center>"
		+ "Group : " + getGroupName() +"<br />"
		+ "Node  : " + getNodeName() +"<br />"
		+ "Date  :" + getDate().toString()+ "<br />" 
		+ "Message : <br /> "
		+ getMessage();
	}
	
}
