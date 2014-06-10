package b2daq.dqm.core;

import b2daq.core.Reader;
import b2daq.core.Serializable;
import b2daq.core.Writer;
import b2daq.graphics.GShape;
import java.io.IOException;
import java.util.Date;

public abstract class MonObject extends GShape implements Serializable {

    protected boolean _updated = false;
    protected long _update_time = 0;
    protected char _position_id = 0;
    protected char _tab_id = 0;

    public void setUpdated(boolean updated) {
        _updated = updated;
    }

    public void setUpdateTime(long update_time) {
        _update_time = update_time;
    }

    public void setUpdateTime() {
        _update_time = new Date().getTime();
    }

    public void setPositionId(char position_id) {
        _position_id = position_id;
    }

    public void setTabId(char tab_id) {
        _tab_id = tab_id;
    }

    public boolean isUpdated() {
        return _updated;
    }

    public long getUpdateTime() {
        return _update_time;
    }

    public int getPositionId() {
        return _position_id;
    }

    public int getTabId() {
        return _tab_id;
    }

    abstract public String getDataType();

    public void reset() {
        _updated = false;
    }

    @Override
    public void writeObject(Writer writer) throws IOException {
        writeConfig(writer);
        writeContents(writer);
    }

    public void writeConfig(Writer writer) throws IOException {
        writer.writeChar(_position_id);
        writer.writeChar(_tab_id);
        writer.writeString(getName());
    }

    abstract public void writeContents(Writer writer) throws IOException;

    @Override
    public void readObject(Reader reader) throws IOException {
        readConfig(reader);
        readContents(reader);
    }

    public void readConfig(Reader reader) throws IOException {
        _position_id = reader.readChar();
        _tab_id = reader.readChar();
        setName(reader.readString());
    }

    abstract public void readContents(Reader reader) throws IOException;

    public String toXML() {
        String str = "<mon-object type=\"" + getDataType() + "\" name=\"" + getName()
                + "\" update_time=\"" + getUpdateTime() + "\" " + getXML() + "</mon-object>\n";
        return str;
    }

    abstract protected String getXML();

}
