package b2logger.java.ui;

import java.awt.Dimension;
import java.awt.Toolkit;
import java.util.HashMap;

import javax.swing.BoxLayout;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JTabbedPane;

import b2daq.core.Log;
import b2daq.core.LogLevel;
import b2daq.java.ui.LogViewPanel;
import b2logger.core.LogMessage;
import b2logger.java.Belle2LogViewer;

public class LogMainFrame extends JFrame {

	private static final long serialVersionUID = 1L;

	private HashMap<String, LogViewPanel> _log_view_m = new HashMap<String, LogViewPanel>();
	private HashMap<String, LogLevel> _popup_level_m = new HashMap<String, LogLevel>();
	private JPanel _side_panel = new JPanel();
	private HashMap<String, LogStatePanel> _state_panel_m = new HashMap<String, LogStatePanel>();
	private JTabbedPane _tab_panel = new JTabbedPane(JTabbedPane.TOP);

	public LogMainFrame () {
		setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		setBounds(0, 0, 660, 520);
		setLocationRelativeTo(null);
		setTitle("Double Chooz Gaibu Monitor version " + Belle2LogViewer.VERSION);
		_side_panel.setPreferredSize(new Dimension(180, 520));
		_side_panel.setMaximumSize(new Dimension(180, 520));
		_side_panel.setLayout(new BoxLayout(_side_panel, BoxLayout.Y_AXIS));
		JPanel panel = new JPanel();
		panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
		panel.add(_side_panel);
		panel.add(_tab_panel);
		add(panel);
		setJMenuBar(new LogMenuBar(this));
		setIconImage(Toolkit.getDefaultToolkit().createImage(getClass().getResource("gaibu.png")));
	}
	
	public void update (LogMessage message) {
		LogViewPanel log_view = null;
		String host_name = message.getHostName();
		if ( _log_view_m.containsKey(host_name) ) {
			log_view = _log_view_m.get(host_name);
		}
		if ( log_view == null ) {
			log_view = new LogViewPanel();
			_log_view_m.put(host_name, log_view);
			_popup_level_m.put(host_name, LogLevel.DEBUG);
			_tab_panel.addTab(host_name, log_view);
			LogStatePanel state_panel = new LogStatePanel(message);
			_side_panel.add(state_panel);
			_state_panel_m.put(host_name, state_panel);
		}
		log_view.add(new Log(message.getMessage(), 
				message.getLogLevel(), message.getTime().getSecond()));
		if ( !getLogThreshold(host_name).hiegher(message.getLogLevel()) ){
			_tab_panel.setSelectedComponent(log_view);
		}
		if ( _state_panel_m.containsKey(host_name) ) {
			_state_panel_m.get(host_name).update(message);
		}
	}
	
	public void setLogThreshold(String host_name, LogLevel level) {
		if ( _log_view_m.containsKey(host_name) ) {
			_log_view_m.get(host_name).setPriorityThreshold(level);
		}
	}
	
	public LogLevel getLogThreshold(String host_name) {
		if ( _log_view_m.containsKey(host_name) ) {
			return _log_view_m.get(host_name).getPriorityThreshold();
		}
		return LogLevel.UNDEFINED;
	}

	public void setPopupLevel(String host_name, LogLevel level) {
		if ( _popup_level_m.containsKey(host_name) ) {
			_popup_level_m.remove(host_name);
			_popup_level_m.put(host_name, level);
		}
	}

	public HashMap<String, LogLevel> getPopupLevels() {
		return _popup_level_m;
	}
	
	public LogLevel getPopupLevel(String host_name) {
		if ( _popup_level_m.containsKey(host_name) ) {
			return _popup_level_m.get(host_name);
		} else {
			return LogLevel.UNDEFINED;
		}
	}

}
