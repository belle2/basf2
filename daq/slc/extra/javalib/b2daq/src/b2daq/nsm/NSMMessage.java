package b2daq.nsm;

import java.io.IOException;

import b2daq.core.Reader;
import b2daq.core.Serializable;
import b2daq.core.Writer;
import b2daq.runcontrol.core.RCCommand;

public final class NSMMessage implements Serializable {

    private String _reqname = "";
    private String _nodename = "";
    private int[] _pars = new int[0];
    private String _data = "";

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
        return _data.replace("\0", "");
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
        _data = data;
    }

    @Override
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
            StringBuilder buf = new StringBuilder();
            for (int i = 0; i < length; i++) {
                buf.append(reader.readChar());
            }
            _data = buf.toString().replace("\0", "");
        } else {
            _data = "";
        }
    }

    @Override
    public void writeObject(Writer writer) throws IOException {
        writer.writeString(getReqName());
        writer.writeString(getNodeName());
        writer.writeInt(getNParams());
        for (int i = 0; i < getNParams(); i++) {
            writer.writeInt(_pars[i]);
        }
        if (_data != null) {
            writer.writeInt(_data.length()+1);
            for (char c : _data.toCharArray()) {
                writer.writeChar(c);
            }
            writer.writeChar('\0');
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
