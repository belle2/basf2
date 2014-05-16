package b2daq.dqm.core;

import java.io.IOException;

import b2daq.core.Reader;
import b2daq.core.Writer;
import b2daq.graphics.HtmlColor;

public class MonLabel extends MonShape {

	private CharArray _array = new CharArray();
	private HtmlColor _font_color = new HtmlColor(HtmlColor.BLACK);
	
	public MonLabel(String name, int max_size) {
		super(name);
		_array.resize(max_size);
	}

	public MonLabel() {
		super();
		_array.resize(0);
	}

	@Override
	public String getDataType() {
		return "MLB";
	}
	
	public void writeConfig(Writer writer) throws IOException {
		super.writeConfig(writer);
		writer.writeInt(_array.length());
	}

	public void writeContents(Writer writer) throws IOException {
		super.writeContents(writer);
		writer.writeShort((short)_font_color.getRed());
		writer.writeShort((short)_font_color.getGreen());
		writer.writeShort((short)_font_color.getBlue());
		_array.writeObject(writer);
	}

	public void readConfig(Reader reader) throws IOException {
		super.readConfig(reader);
		_array.resize(reader.readInt());
	}

	public void readContents(Reader reader) throws IOException {
		super.readContents(reader);
		short red = reader.readShort();
		short green = reader.readShort();
		short blue = reader.readShort();
		_font_color.setRGB(red, green, blue);
		_array.readObject(reader);
	}

	public void setMaxSize(int max_size) {
		_array.resize(max_size);
	}

	public int getMaxSize() {
		return _array.length();
	}

	public void setText(String text) {
		int n = 0;
		for ( ; n < text.length() && n < _array.length() - 1; n++ ) {
			_array.set(n, text.charAt(n));
		}
		_array.set(n, '\n');
	}

	public String getText() {
		StringBuffer ss = new StringBuffer();
		for ( int n = 0; n < _array.length(); n++ ) {
			char c = _array.at(n);
			if ( c == '\n' || c == '\0' ) break;
			ss.append(c);
		}
		return ss.toString();
	}

	public void setFontColor(HtmlColor font_color) {
		_font_color = font_color;
	}

	public HtmlColor getFontColor() {
		return _font_color;
	}
	
	protected String getXML() {
		String str = "text=\"" + getText() + 
		"\" color=\"" + _font_color.toString() + 
		"\" >\n";
		return str;
	}

}
