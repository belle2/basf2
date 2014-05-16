package b2daq.dqm.core;

import java.io.IOException;

import b2daq.core.Reader;
import b2daq.core.Writer;

public class MonCircle extends MonShape {

	private float _x, _y, _r_x, _r_y;
	
	public MonCircle(String name) {
		super(name);
	}

	public MonCircle() {
		super();
	}

	@Override
	public String getDataType() {
		return "MCC";
	}
	
	public void writeConfig(Writer writer) throws IOException {
		super.writeConfig(writer);
	}

	public void writeUpdate(Writer writer) throws IOException {
		super.writeConfig(writer);
		writer.writeFloat(_x);
		writer.writeFloat(_y);
		writer.writeFloat(_r_x);
		writer.writeFloat(_r_y);
	}

	public void writeHistory(Writer writer) throws IOException {
		
	}
	
	public void readConfig(Reader reader) throws IOException {
		super.readConfig(reader);
	}

	public void readUpdate(Reader reader) throws IOException {
		super.readConfig(reader);
		_x = reader.readFloat();
		_y = reader.readFloat();
		_r_x = reader.readFloat();
		_r_y = reader.readFloat();
	}

	public void readHistory(Reader reader) throws IOException {
		
	}

	public void setX(float x) {
		_x = x;
	}

	public float getX() {
		return _x;
	}

	public void setRX(float rx) {
		_r_x = rx;
	}

	public float getRX() {
		return _r_x;
	}

	public void setY(float y) {
		_y = y;
	}

	public float getY() {
		return _y;
	}

	public void setRY(float ry) {
		_r_y = ry;
	}

	public float getRY() {
		return _r_y;
	}

	protected String getXML() {
		String str = "x=\"" + _x + 
		"\" y=\"" + _y + 
		"\" r_x=\"" + _r_x + 
		"\" r_y=\"" + _r_y + 
		"\" >\n";
		return str;
	}
	
}
