package b2daq.dqm.core;

import java.io.IOException;

import b2daq.core.Reader;
import b2daq.core.Writer;

public class MonRect extends MonShape {

	private float _x, _y, _width, _height;
	
	public MonRect(String name) {
		super(name);
	}

	public MonRect() {
		super();
	}

	@Override
	public String getDataType() {
		return "MRT";
	}
	
	public void writeConfig(Writer writer) throws IOException {
		super.writeConfig(writer);
	}

	public void writeContents(Writer writer) throws IOException {
		super.writeContents(writer);
		writer.writeFloat(_x);
		writer.writeFloat(_y);
		writer.writeFloat(_width);
		writer.writeFloat(_height);
	}

	public void writeHistory(Writer writer) throws IOException {
		
	}
	
	public void readConfig(Reader reader) throws IOException {
		super.readConfig(reader);
	}

	public void readContents(Reader reader) throws IOException {
		super.readContents(reader);
		_x = reader.readFloat();
		_y = reader.readFloat();
		_width = reader.readFloat();
		_height = reader.readFloat();
	}

	public void readHistory(Reader reader) throws IOException {
		
	}

	public void setX(float x) {
		_x = x;
	}

	public float getX() {
		return _x;
	}

	public void setWidth(float width) {
		_width = width;
	}

	public float getWidth() {
		return _width;
	}

	public void setY(float y) {
		_y = y;
	}

	public float getY() {
		return _y;
	}

	public void setHeight(float height) {
		_height = height;
	}

	public float getHeight() {
		return _height;
	}

	protected String getXML() {
		String str = "x=\"" + _x + 
		"\" y=\"" + _y + 
		"\" width=\"" + _width + 
		"\" height=\"" + _height + 
		"\" >\n";
		return str;
	}

}
