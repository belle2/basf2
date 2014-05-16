package b2daq.dqm.core;

import java.io.IOException;

import b2daq.core.Reader;
import b2daq.core.Writer;

public class MonLine extends MonShape {

	private float _x1, _y1, _x2, _y2;
	
	public MonLine(String name) {
		super(name);
	}

	public MonLine() {
		super();
	}

	@Override
	public String getDataType() {
		return "MLN";
	}
	
	public void writeConfig(Writer writer) throws IOException {
		super.writeConfig(writer);
	}

	public void writeContents(Writer writer) throws IOException {
		super.writeContents(writer);
		writer.writeFloat(_x1);
		writer.writeFloat(_y1);
		writer.writeFloat(_x2);
		writer.writeFloat(_y2);
	}

	public void writeHistory(Writer writer) throws IOException {
		
	}
	
	public void readConfig(Reader reader) throws IOException {
		super.readConfig(reader);
	}

	public void readContents(Reader reader) throws IOException {
		super.readContents(reader);
		_x1 = reader.readFloat();
		_y1 = reader.readFloat();
		_x2 = reader.readFloat();
		_y2 = reader.readFloat();
	}

	public void readHistory(Reader reader) throws IOException {
		
	}

	public void setX2(float x) {
		_x2 = x;
	}

	public float getX2() {
		return _x2;
	}

	public void setX1(float x) {
		_x1 = x;
	}

	public float getX1() {
		return _x1;
	}

	public void setY2(float y) {
		_y2 = y;
	}

	public float getY2() {
		return _y2;
	}

	public void setY1(float y) {
		_y1 = y;
	}

	public float getY1() {
		return _y1;
	}

	protected String getXML() {
		String str = "x1=\"" + _x1 + 
		"\" y1=\"" + _y1 + 
		"\" x2=\"" + _x2 + 
		"\" y2=\"" + _y2 +
		"\" >\n";
		return str;
	}

}
