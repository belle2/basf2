package b2daq.java.ui;

import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;

import javax.swing.JPanel;

import b2daq.core.Log;
import b2daq.core.LogLevel;
import b2daq.core.Time;


public class LogViewPanel extends JPanel {

	private static final long serialVersionUID = 1L;
	private LogPanel _log = new LogPanel();

	public LogViewPanel() {
		setLayout(null);
		addComponentListener(new ComponentAdapter() {
			public void componentResized(ComponentEvent e) {
				int width = getWidth();
				int height = getHeight();
				setSize(width, height);
				_log.setBounds(6, 6, width - 14, height - 14);
			}
    	});
		add(_log);
	}
	
	public void setPriorityThreshold(LogLevel level) {
		_log.setPriorityThreshold(level);
	}
	
	public LogLevel getPriorityThreshold() {
		return _log.getPriorityThreshold();
	}
	
	synchronized public void add(Log log) {
		_log.add(log);
	}
	
	synchronized public void addWithoutDate(Log log) {
		log.setDate(new Time().toTimeString());
		_log.add(log);
	}
	
	public void update() {
		_log.update();
	}
	
	public String toString() {
		return _log.toString();
	}
}
