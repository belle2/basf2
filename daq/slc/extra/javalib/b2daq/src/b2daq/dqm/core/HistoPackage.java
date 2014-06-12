package b2daq.dqm.core;

import java.io.IOException;
import java.util.ArrayList;

import b2daq.core.Reader;
import b2daq.core.Serializable;
import b2daq.core.Writer;

public class HistoPackage implements Serializable {

    final public static byte ObjectFlag = (byte) 0xDC;

    private ArrayList<MonObject> _histo_v = new ArrayList<MonObject>();
    private String _name;
    private int _update_id;
    private long _update_time;

    public HistoPackage() {
        this("XXX");
    }

    public HistoPackage(String name) {
        _name = name;
        _update_id = 0;
    }

    public void copy(HistoPackage pack) {
        _histo_v.clear();
        _histo_v.addAll(pack._histo_v);
        _name = pack._name;
        _update_id = pack._update_id;
    }

    public MonObject getHisto(int index) {
        return _histo_v.get(index);
    }

    public MonObject getHisto(String name) {
        for (int n = 0; n < _histo_v.size(); n++) {
            if (name.matches(_histo_v.get(n).getName())) {
                return _histo_v.get(n);
            }
        }
        return null;
    }

    public ArrayList<MonObject> getMonObjects() {
        return _histo_v;
    }

    public void setHisto(int index, Histo h) {
        _histo_v.set(index, h);
    }

    public Histo addHisto(Histo h) {
        _histo_v.add(h);
        return h;
    }

    public MonObject addMonObject(MonObject obj) {
        _histo_v.add(obj);
        return obj;
    }

    public String getName() {
        return _name;
    }

    public int getUpdateId() {
        return _update_id;
    }

    public void setName(String name) {
        _name = name;
    }

    public void setUpdateTimeAll() {
        for (MonObject obj : _histo_v) {
            obj.setUpdateTime();
        }
    }

    public void setUpdateId(int id) {
        _update_id = id;
    }

    public String toString() {
        String script = "{\n" + "  'name':'" + getName() + "',\n"
                + "  'updateid':" + getUpdateId() + ",\n" + "  'timestamp':'"
                + ",\n" + "  'tabs':\n" + "    [\n";
        script += "\n" + "    ],\n" + "  'nhistos':" + _histo_v.size() + ",\n"
                + "  'histos':\n" + "    [\n";
        for (int n = 0; n < _histo_v.size(); n++) {
            script += "" + getHisto(n).toString();
            if (n != _histo_v.size() - 1) {
                script += ",\n";
            }
        }
        script += "\n" + "    ]\n" + "}\n";
        return script;
    }

    public String toXML() {
        String script = "<histo-package" + "  name=\"" + getName()
                + "\" updateid=\"" + getUpdateId()
                + "\" >\n";
        for (int n = 0; n < _histo_v.size(); n++) {
            script += getHisto(n).toXML();
        }
        script += "</histo-package>\n";
        return script;
    }

    public void reset() {
        _update_id = 0;
        for (int n = 0; n < _histo_v.size(); n++) {
            _histo_v.get(n).reset();
        }
    }

    public void readObject(Reader reader) throws IOException {
        setName(reader.readString());
        int nhistos = reader.readInt();
        _histo_v = new ArrayList<MonObject>(nhistos);
        setUpdateId(reader.readInt());
        int hid;
        for (int n = 0; n < nhistos; n++) {
            hid = reader.readInt();
            if (hid < 0 || hid > nhistos) {
                throw new IOException("hid=" + hid);
            }
            MonObject h = HistoFactory.create(reader.readString());
            if (h != null) {
                h.readObject(reader);
            } else {
                throw new WrongDataTypeException("");
            }
        }
    }

    public void readContents(Reader reader) throws IOException {
        setUpdateId(reader.readInt());
        long time = reader.readLong() * 1000;
        setUpdateTime(time);
        int nhistos = reader.readInt();
        byte c;
        int hid;
        for (int n = 0; n < nhistos; n++) {
            c = (byte) reader.readChar();
            hid = reader.readInt();
            if (c != ObjectFlag) {
                throw new IOException(n + "::" + hid + "=" + c);
            }
            if (hid >= 0 && hid < getMonObjects().size()) {
                getHisto(hid).readContents(reader);
                getHisto(hid).setUpdateTime(time);
                getHisto(hid).setUpdated(true);
            } else {
                throw new IOException("Bad object id: " + hid);
            }
        }
        if (getUpdateId() == 0) {
            reset();
        }
    }

    public void readConfig(Reader reader) throws IOException {
        setName(reader.readString());
        int nhistos = reader.readInt();
        _histo_v = new ArrayList<MonObject>(nhistos);
        int hid;
        for (int n = 0; n < nhistos; n++) {
            hid = reader.readInt();
            if (hid < 0 || hid > nhistos) {
                throw new IOException("hid=" + hid);
            }
            MonObject h = HistoFactory.create(reader.readString());
            if (h != null) {
                h.readConfig(reader);
                addMonObject(h);
            } else {
                throw new WrongDataTypeException("");
            }
        }
    }

    public void writeObject(Writer writer) throws IOException {
        writeConfig(writer);
        for (int n = 0; n < _histo_v.size(); n++) {
            writer.writeChar((char) ObjectFlag);
            getHisto(n).writeObject(writer);
        }
    }

    public void writeContents(Writer writer) throws IOException {
        writer.writeInt(getUpdateId());
        writer.writeLong(getUpdateTime());
        writer.writeInt(_histo_v.size());
        for (int n = 0; n < _histo_v.size(); n++) {
            writer.writeChar((char) ObjectFlag);
            writer.writeInt(n);
            getHisto(n).writeContents(writer);
        }
    }

    public long getUpdateTime() {
        return _update_time;
    }

    public void setUpdateTime(long time) {
        _update_time = time;
    }

    public void writeConfig(Writer writer) throws IOException {
        writer.writeString(getName());
        writer.writeInt(_histo_v.size());
        for (int n = 0; n < _histo_v.size(); n++) {
            writer.writeInt(n);
            getHisto(n).writeConfig(writer);
        }
    }

    public int getNHistos() {
        return _histo_v.size();
    }

}
