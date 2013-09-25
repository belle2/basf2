package b2daq.graphics;

public class HtmlColor {

	String _html_code = "";
	private int _red, _blue, _green;

	public static HtmlColor NULL = new HtmlColor(-1, -1, -1);
	public static HtmlColor BLACK = new HtmlColor(0, 0, 0);
	public static HtmlColor WHITE = new HtmlColor(255, 255, 255);
	public static HtmlColor BLUE = new HtmlColor(0, 0, 255);
	public static HtmlColor CYAN = new HtmlColor(0, 255, 255);
	public static HtmlColor DARK_GRAY = new HtmlColor(64, 64, 64);
	public static HtmlColor GRAY = new HtmlColor(128, 128, 128);
	public static HtmlColor GREEN = new HtmlColor(0, 255, 0);
	public static HtmlColor LIGHT_GRAY = new HtmlColor(192, 192, 192);
	public static HtmlColor MAGENTA = new HtmlColor(255, 0, 255);
	public static HtmlColor ORANGE = new HtmlColor(255, 102, 0);
	public static HtmlColor GOLD = new HtmlColor(255, 204, 0);
	public static HtmlColor PINK = new HtmlColor(255, 175, 175);
	public static HtmlColor RED = new HtmlColor(255, 0, 0);
	public static HtmlColor YELLOW = new HtmlColor(255, 255, 0);
	public static HtmlColor RUNNING_GREEN = new HtmlColor(0, 204, 102); 

	public final static HtmlColor ColorGrad[] = {
		new HtmlColor(153,0,255), new HtmlColor(102,0,255), new HtmlColor(51,0,255), new HtmlColor(0,0,255),
		new HtmlColor(0,51,255), new HtmlColor(0,102,255), new HtmlColor(0,153,255), new HtmlColor(0,204,255),
		new HtmlColor(0,255,255), new HtmlColor(0,255,204), new HtmlColor(0,255,153), new HtmlColor(0,255,102),
		new HtmlColor(0,255,51), new HtmlColor(0,255,0), new HtmlColor(51,255,0), new HtmlColor(102,255,0),
		new HtmlColor(153,255,00), new HtmlColor(204,255,0), new HtmlColor(255,255,0), new HtmlColor(255,204,0),
		new HtmlColor(255,153,00), new HtmlColor(255,102,0), new HtmlColor(255,51,0), new HtmlColor(255,0,0),
		new HtmlColor(255,0,51)
	};

	public HtmlColor() {
		this(255, 255, 255);
	}

	public HtmlColor(String code) {
		code = code.toUpperCase();
		if (code.charAt(0) == '#') {
			if (code.length() > 6) {
				int[] d = new int[6];
				for (int n = 0; n < 6; n++) {
					if (code.charAt(n + 1) >= 'A')
						d[n] = code.charAt(n + 1) - 'A' + 10;
					else
						d[n] = code.charAt(n + 1) - '0';
				}
				_red = d[0] * 16 + d[1];
				_green = d[2] * 16 + d[3];
				_blue = d[4] * 16 + d[5];
			} else if (code.length() == 4) {
				int[] d = new int[3];
				for (int n = 0; n < 3; n++) {
					if (code.charAt(n + 1) >= 'A')
						d[n] = code.charAt(n + 1) - 'A' + 10;
					else
						d[n] = code.charAt(n + 1) - '0';
				}
				_red = d[0] * 16 + d[0];
				_green = d[1] * 16 + d[1];
				_blue = d[2] * 16 + d[2];
			}
			if ( _red > 255 ) _red = 255;
			if ( _green > 255 ) _green = 255;
			if ( _blue > 255 ) _blue = 255;
			_html_code = code;
		} else if (code.equals("BLACK") || code.equals("black")) {
			copy(HtmlColor.BLACK);
		} else if (code.equals("YELLOW") || code.equals("yellow")) {
			copy(HtmlColor.YELLOW);
		} else if (code.equals("PINK") || code.equals("pink")) {
			copy(HtmlColor.PINK);
		} else if (code.equals("GREEN") || code.equals("green")) {
			copy(HtmlColor.GREEN);
		} else if (code.equals("BLUE") || code.equals("blue")) {
			copy(HtmlColor.BLUE);
		} else if (code.equals("RED") || code.equals("red")) {
			copy(HtmlColor.RED);
		} else if (code.equals("GRAY") || code.equals("gray")) {
			copy(HtmlColor.GRAY);
		} else if (code.equals("ORANGE") || code.equals("orange")) {
			copy(HtmlColor.ORANGE);
		} else if (code.equals("CYAN") || code.equals("cyan")) {
			copy(HtmlColor.CYAN);
		} else if (code.equals("DARK_GRAY") || code.equals("dark_gray")) {
			copy(HtmlColor.DARK_GRAY);
		} else if (code.equals("LIGHT_GRAY") || code.equals("light_gray")) {
			copy(HtmlColor.LIGHT_GRAY);
		} else if (code.equals("MAGENTA") || code.equals("magenta")) {
			copy(HtmlColor.MAGENTA);
		} else if (code.equals("WHITE") || code.equals("white")) {
			copy(HtmlColor.WHITE);
		} else if (code.equals("RUNNING_GREEN") || code.equals("running_green")) {
			copy(HtmlColor.RUNNING_GREEN);
		} else {
			copy(HtmlColor.NULL);
		}
	}

	public HtmlColor(int red, int green, int blue) {
		setRGB(red, green, blue);
	}

	public HtmlColor(HtmlColor color) {
		copy(color);
	}

	public void setRGB(int red, int green, int blue) {
		_red = red;
		if (_red > 255) _red = 255;
		_green = green;
		if (_green > 255) _green = 255;
		_blue = blue;
		if (_blue > 255) _blue = 255;
		_html_code = "#";
		if (red < 16)
			_html_code += "0";
		_html_code += Integer.toHexString(red);
		if (green < 16)
			_html_code += "0";
		_html_code += Integer.toHexString(green);
		if (blue < 16)
			_html_code += "0";
		_html_code += Integer.toHexString(blue);
		_html_code = _html_code.toUpperCase();
	}

	public int getRed() {
		return _red;
	}

	public int getGreen() {
		return _green;
	}

	public int getBlue() {
		return _blue;
	}

	public String toString() {
		if (isNull()) return "none";
		else return _html_code;
	}

	public void copy(HtmlColor c) {
		if (c == null)
			return;
		_html_code = c.toString();
		_red = c._red;
		_green = c._green;
		_blue = c._blue;
	}

	public boolean isNull() {
		if (_red < 0 || _green < 0 || _blue < 0) {
			return true;
		} else {
			return false;
		}
	}
}
