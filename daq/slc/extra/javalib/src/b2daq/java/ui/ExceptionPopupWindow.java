package b2daq.java.ui;

import b2daq.graphics.FontProperty;
import b2daq.graphics.HtmlColor;

public class ExceptionPopupWindow extends PopupWindow {

	private static final long serialVersionUID = 1L;

	public ExceptionPopupWindow() {
		this(new NullPointerException("NULL"));
	}
	
	public ExceptionPopupWindow(Exception e) {
		super("Exception", "Caught exception : " + e.getClass().getName() + "\n"
				+ "Message : " + e.getMessage() + "", 0, new FontProperty(HtmlColor.RED, HtmlColor.NULL));
		setVisible(true);
	}

	public ExceptionPopupWindow(String from, Exception e) {
		super("Exception", "From :"+from + "\n "
				+ "Caught exception " + e.getClass().getName() + "\n"
				+ "Message : " + e.getMessage() + "", 0, new FontProperty(HtmlColor.RED, HtmlColor.NULL));
		setVisible(true);
	}
}
