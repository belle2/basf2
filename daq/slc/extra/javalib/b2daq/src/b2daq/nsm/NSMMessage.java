package b2daq.nsm;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;

import b2daq.core.Reader;
import b2daq.core.Serializable;
import b2daq.core.SizeCounter;
import b2daq.core.Writer;
import b2daq.io.DataReader;
import b2daq.io.DataWriter;
import b2daq.runcontrol.core.RCCommand;
import java.util.logging.Level;
import java.util.logging.Logger;

public class NSMMessage implements Serializable {

    private String _reqname = "";
    private String _nodename = "";
    private int[] _pars = new int[0];
    private byte[] _data = null;

    public NSMMessage() {

    }

    public NSMMessage(NSMCommand cmd) {
        _reqname = cmd.getLabel();
    }

    public NSMMessage(NSMCommand cmd, int[] pars) {
        _reqname = cmd.getLabel();
        _pars = pars;
    }

    public NSMMessage(NSMCommand cmd, int[] pars, String data) {
        _reqname = cmd.getLabel();
        _pars = pars;
        setData(data);
    }

    public NSMMessage(String nodename, NSMCommand cmd, String data) {
        this(cmd, data);
        _nodename = nodename;
    }

    public NSMMessage(String nodename, NSMCommand cmd) {
        this(cmd);
        _nodename = nodename;
    }

    public NSMMessage(String nodename, NSMCommand cmd, int[] pars) {
        this(cmd, pars);
        _nodename = nodename;
    }

    public NSMMessage(String nodename, NSMCommand cmd, int[] pars, String data) {
        this(cmd, pars, data);
        _nodename = nodename;
    }

    public NSMMessage(NSMCommand cmd, String data) {
        _reqname = cmd.getLabel();
        _pars = null;
        setData(data);
    }

    public NSMMessage(NSMNode node, RCCommand command) {
        this(node.getName(), command);
    }

    public NSMMessage(NSMNode node, RCCommand command, int[] pars) {
        this(node.getName(), command, pars);
    }

    public NSMMessage(NSMNode node, RCCommand command, int[] pars, String data) {
        this(node.getName(), command, pars, data);
    }

    public String getNodeName() {
        return _nodename;
    }

    public void setNodeName(String nodename) {
        _nodename = nodename;
    }

    public String getReqName() {
        return _reqname;
    }

    public void setReqName(NSMCommand cmd) {
        _reqname = cmd.getLabel();
    }

    public void setReqName(String name) {
        _reqname = name;
    }

    public int getNParams() {
        return (_pars == null) ? 0 : _pars.length;
    }

    public int[] getParams() {
        return _pars;
    }

    public int getParam(int i) {
        return _pars[i];
    }

    public String getData() {
        return new String(_data).replace("\0", "");
    }

    public void getData(Serializable obj) {
        Reader reader = new DataReader(new ByteArrayInputStream(_data));
        try {
            reader.readObject(obj);
        } catch (IOException ex) {
            Logger.getLogger(NSMMessage.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    public void setNParams(int npar) {
        if (_pars.length != npar) {
            _pars = new int[npar];
        }
    }

    public void setParams(int[] pars) {
        _pars = pars;
    }

    public void setParam(int i, int v) {
        _pars[i] = v;
    }

    public void setData(String data) {
        _data = null;
        if (data == null) {
            return;
        }
        _data = new byte[data.length()+1];
        byte[] d = data.getBytes();
        for (int i = 0; i < data.length(); i++) {
            _data[i] = d[i];
        }
        _data[data.length()] = '\0';
    }

    public void setData(Serializable data) {
        _data = null;
        if (data == null) {
            return;
        }
        SizeCounter counter = new SizeCounter();
        try {
            counter.writeObject(data);
            ByteArrayOutputStream ostream = new ByteArrayOutputStream(counter.count());
            Writer writer = new DataWriter(ostream);
            writer.writeObject(data);
            _data = ostream.toByteArray();
        } catch (Exception e) {
        }
    }

    public void readObject(Reader reader) throws IOException {
        setReqName(reader.readString());
        setNodeName(reader.readString());
        int npar = reader.readInt();
        setNParams(npar);
        for (int i = 0; i < _pars.length; i++) {
            _pars[i] = reader.readInt();
        }
        int length = reader.readInt();
        if (length > 0) {
            _data = new byte[length];
            for (int i = 0; i < length; i++) {
                _data[i] = reader.readByte();
            }
        } else {
            _data = null;
        }
    }

    public void writeObject(Writer writer) throws IOException {
        writer.writeString(getReqName());
        writer.writeString(getNodeName());
        writer.writeInt(getNParams());
        for (int i = 0; i < getNParams(); i++) {
            writer.writeInt(_pars[i]);
        }
        if (_data != null) {
            writer.writeInt(_data.length);
            for (int i = 0; i < _data.length; i++) {
                writer.writeByte(_data[i]);
            }
        } else {
            writer.writeInt(0);
        }
    }

    public void print() {
        System.out.println(_reqname);
        System.out.print("pars[" + getNParams() + "] = {");
        for (int i = 0; i < getNParams(); i++) {
            System.out.print(_pars[i]);
            if (i < getNParams() - 1) {
                System.out.print(", ");
            }
        }
        System.out.println("}");
    }

}
