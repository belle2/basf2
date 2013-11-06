package b2logger.java.ui;

import java.awt.GraphicsEnvironment;

import javax.swing.JToolTip;
import javax.swing.JWindow;

import b2logger.core.LogMessage;

public class LogPopup extends Thread {

	protected static final int WIDTH = (int) (GraphicsEnvironment
			.getLocalGraphicsEnvironment().getMaximumWindowBounds().width);
	protected static final int HEIGHT = (int) (GraphicsEnvironment
			.getLocalGraphicsEnvironment().getMaximumWindowBounds().height);
	
	protected String _html = "<html>NO MESSAGE</html>";
	protected JWindow _window = new JWindow();
	protected JToolTip _tip = new JToolTip();
	protected LogMessage _message = null;
	
	public LogPopup(LogMessage message, int font_size) {
		this();
		_message = message;
		_html = "<html><div style=\"font-size:"+font_size+";background:white;color:" + 
		message.getLogLevel().getFontColor() +
		";background:" + message.getLogLevel().getFillColor() + "\">" + 
		message.toHTML() +
		"</div></html>";
        _tip.setTipText(_html);
        _window.setAlwaysOnTop(true);
	}

	public LogPopup() {}

	public JWindow getWindow() { return _window; }

}
