package b2dqm.core;

import b2daq.core.Reader;
import b2daq.core.Writer;
import b2daq.graphics.HtmlColor;

public abstract class MonShape extends MonObject {

	private HtmlColor _fill_color = new HtmlColor(HtmlColor.NULL);
	private HtmlColor _line_color = new HtmlColor(HtmlColor.BLACK);

	public MonShape() {
		setName("");
	}

	public MonShape(String name) {
		setName(name);
	}

	public void setFillColor(HtmlColor color) {
		_fill_color = color;
	}

	public void setLineColor(HtmlColor color) {
		_line_color = color;
	}

	public HtmlColor getFillColor() {
		return _fill_color;
	}
	
	public HtmlColor getLineColor() {
		return _line_color;
	}
	
	public void writeConfig(Writer writer) throws Exception {
		super.writeConfig(writer);
	}

	public void writeContents(Writer writer) throws Exception {
		writer.writeShort((short)_fill_color.getRed());
		writer.writeShort((short)_fill_color.getGreen());
		writer.writeShort((short)_fill_color.getBlue());
		writer.writeShort((short)_line_color.getRed());
		writer.writeShort((short)_line_color.getGreen());
		writer.writeShort((short)_line_color.getBlue());
	}

	public void readConfig(Reader reader) throws Exception {
		super.readConfig(reader);
	}

	public void readContents(Reader reader) throws Exception {		
		short red = reader.readShort();
		short green = reader.readShort();
		short blue = reader.readShort();
		_fill_color.setRGB(red, green, blue);
		red = reader.readShort();
		green = reader.readShort();
		blue = reader.readShort();
		_line_color.setRGB(red, green, blue);
	}

}
