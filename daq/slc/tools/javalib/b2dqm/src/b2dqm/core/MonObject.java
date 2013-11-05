package b2dqm.core;

import java.util.Date;

import b2daq.core.Reader;
import b2daq.core.SerializableObject;
import b2daq.core.Writer;


public abstract class MonObject implements SerializableObject {

	protected boolean _updated = false;
	protected long _update_time = 0;
	protected char _position_id = 0;
	protected char _tab_id = 0;
	protected String _name = "";

	public void setUpdated(boolean updated) { _updated = updated; }
	public void setUpdateTime(long update_time) { _update_time = update_time; }
	public void setUpdateTime() { _update_time = new Date().getTime(); }
	public void setPositionId(char position_id) { _position_id = position_id; }
	public void setTabId(char tab_id) { _tab_id = tab_id; }
	public void setName(String name) { _name = name; }

	public boolean isUpdated() { return _updated; }
	public long getUpdateTime() { return _update_time; }
	public int getPositionId() { return _position_id; }
	public int getTabId() { return _tab_id; }
	public String getName() { return _name;}
	
	abstract public String getDataType();

	public void reset() {
		_updated = false;
	}
	
	public void writeObject(Writer writer) throws Exception {
		writeConfig(writer);
		writeContents(writer);
	}

	public void writeConfig(Writer writer) throws Exception {
		writer.writeChar(_position_id);
		writer.writeChar(_tab_id);
		writer.writeString(_name);
	}

	abstract public void writeContents(Writer writer) throws Exception;
	
	public void readObject(Reader reader) throws Exception {
		readConfig(reader);
		readContents(reader);
	}

	public void readConfig(Reader reader) throws Exception {
		_position_id = reader.readChar();
		_tab_id = reader.readChar();
		_name = reader.readString();
	}

	abstract public void readContents(Reader reader) throws Exception;
	
	public String toXML() {
		String str = "<mon-object type=\"" + getDataType() + "\" name=\"" + getName() + 
		"\" update_time=\"" + getUpdateTime() + "\" " + getXML() + "</mon-object>\n";
		return str;
	}

	abstract protected String getXML();
	
}
