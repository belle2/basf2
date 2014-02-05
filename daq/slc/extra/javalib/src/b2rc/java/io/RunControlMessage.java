package b2rc.java.io;

import b2daq.core.Reader;
import b2daq.core.SerializableObject;
import b2daq.core.Writer;
import b2daq.core.RCCommand;

public class RunControlMessage implements SerializableObject {

	private RCCommand _command = new RCCommand();
	private int[] _pars = new int [0];
	private String _data = "";

	public RunControlMessage() {

	}

	public RunControlMessage(RCCommand cmd) {
		_command.copy(cmd);
	}

	public RunControlMessage(RCCommand cmd, int[] pars) {
		_command.copy(cmd);
		_pars = pars;
	}

	public RunControlMessage(RCCommand cmd, int[] pars, String data) {
		_command.copy(cmd);
		_pars = pars;
		_data = data;
	}

	public RunControlMessage(RCCommand cmd, int value) {
		_command.copy(cmd);
		_pars = new int [1];
		_pars[0] = value;
		_data = "";
	}

	public RunControlMessage(RCCommand cmd, int flag, int value) {
		_command.copy(cmd);
		_pars = new int [2];
		_pars[0] = flag;
		_pars[1] = value;
		_data = "";
	}

	public RunControlMessage(RCCommand cmd, int flag, String value) {
		_command.copy(cmd);
		_pars = new int [1];
		_pars[0] = flag;
		_data = value;
	}

	public RunControlMessage(RCCommand cmd, String data) {
		_command.copy(cmd);
		_data = data;
	}

	public int getNParams() {
		return _pars.length;
	}

	public int[] getParams() {
		return _pars;
	}

	public int getParam(int i) {
		return _pars[i];
	}

	public String getData() {
		return _data;
	}

	public void setNParams(int npar) {
		_pars = new int[npar];
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

	public void readObject(Reader reader) throws Exception {
		int npar = reader.readInt();
		setNParams(npar);
		for (int i = 0; i < _pars.length; i++) {
			_pars[i] = reader.readInt();
		}
		_data = reader.readString();
	}

	public void writeObject(Writer writer) throws Exception {
		writer.writeInt(getNParams());
		for (int i = 0; i < getNParams(); i++) {
			writer.writeInt(_pars[i]);
		}
		if (_data != null) writer.writeString(_data);
		else writer.writeString("");
	}

	public RCCommand getCommand() {
		return _command;
	}

}
