package b2daq.ui;

import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;

import javax.swing.JPanel;

public class MessageViewPanel extends JPanel {

	private static final long serialVersionUID = 1L;
	private MessagePanel _message;

	public MessageViewPanel(int fontsize) {
		_message = new MessagePanel(fontsize);
		setLayout(null);
		addComponentListener(new ComponentAdapter() {
			public void componentResized(ComponentEvent e) {
				int width = getWidth();
				int height = getHeight();
				setSize(width, height);
				_message.setBounds(5, 5, width - 10, height - 10);
			}
    	});
		add(_message);
	}
	
	public void setFontSize(int fontsize) {
		_message.setFontSize(fontsize);
	}
	
	synchronized public void add(String message) {
		_message.add(message);
	}
	
	public void update() {
		_message.update();
	}
	
	public void reset() {
		_message.reset();
	}
	
	public String toString() {
		return _message.toString();
	}
}
