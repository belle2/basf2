package b2daq.core;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;

import b2daq.java.io.DataReader;
import b2daq.java.io.DataWriter;

public class NSMMessage implements SerializableObject {

	private String _reqname = "";
	private String _nodename = "";
	private int[] _pars = new int [0];
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
	
	public int getNParams() {
		return (_pars == null)?0:_pars.length;
	}

	public int[] getParams() {
		return _pars;
	}

	public int getParam(int i) {
		return _pars[i];
	}

	public String getData() {
		return new String(_data);
	}

	public void getData(SerializableObject obj) {
		Reader reader = new DataReader(new ByteArrayInputStream(_data));
		try {
			reader.readObject(obj);
		} catch (Exception e) {
			
		}
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
		_data = null;
		if (data == null) return; 
		_data = new byte[data.length()];
		byte[] d = data.getBytes();
		for (int i = 0; i < data.length(); i++) 
			_data[i] = d[i];
	}

	public void setData(SerializableObject data) {
		_data = null;
		if (data == null) return; 
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
	
	public void readObject(Reader reader) throws Exception {
		_reqname = reader.readString();
		int npar = reader.readInt();
		setNParams(npar);
		for (int i = 0; i < _pars.length; i++) {
			_pars[i] = reader.readInt();
		}
		int length = reader.readInt();
		_data = new byte[length];
		for (int i = 0; i < length; i++) { 
			_data[i] = reader.readByte();
		}
	}

	public void writeObject(Writer writer) throws Exception {
		writer.writeString(_reqname);
		writer.writeInt(getNParams());
		for (int i = 0; i < getNParams(); i++) {
			writer.writeInt(_pars[i]);
		}
		if (_data != null) {
			writer.writeInt(_data.length);
			for (int i = 0; i < _data.length; i++) 
				writer.writeByte(_data[i]);
		} else {
			writer.writeInt(0);
		}
	}

	public void print() {
		System.out.println(_reqname);
		System.out.print("pars["+getNParams()+"] = {");
		for (int i = 0; i < getNParams(); i++) {
			System.out.print(_pars[i]);
			if (i < getNParams()-1) System.out.print(", ");
		}
		System.out.println("}");
	}
	
}
