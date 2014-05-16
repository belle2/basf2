package b2daq.logger.ui;

import java.awt.Dimension;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Vector;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JComboBox;
import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

import b2daq.apps.Belle2LogViewer;
import b2daq.logger.core.SystemLogLevel;

public class LogMenuBar extends JMenuBar {

	private static final long serialVersionUID = 1L;
	private LogMainFrame _frame;

	public LogMenuBar(LogMainFrame frame) {
		_frame = frame;
		
	    JMenu config_menu = new JMenu("Config");
	    JMenu command_menu = new JMenu("Command");
	    JMenu help_menu = new JMenu("Help");

	    add(config_menu);
	    add(command_menu);
	    add(help_menu);

	    JMenuItem popup_threshold = new JMenuItem("Popup threshold");
	    popup_threshold.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				LevelSelectPanel panel = new LevelSelectPanel();
				Object[] message = { panel };
				String[] options = { "OK", "Cancel" };
				int result = JOptionPane.showOptionDialog(_frame, message, "Change Popup threshold",
						JOptionPane.DEFAULT_OPTION, JOptionPane.PLAIN_MESSAGE, null,
						options, options[0]);
				switch (result) {
				case 0:
					_frame.setPopupLevel((String)panel.host_combo.getSelectedItem(),
							(SystemLogLevel)panel.level_combo.getSelectedItem());
					break;
				case 1:
				}
			}
	    });
	    config_menu.add(popup_threshold);

	    JMenuItem log_threshold = new JMenuItem("Log threshold");
	    log_threshold.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				LevelSelectPanel panel = new LevelSelectPanel();
				Object[] message = { panel };
				String[] options = { "OK", "Cancel" };
				int result = JOptionPane.showOptionDialog(_frame, message, "Change Log threshold",
						JOptionPane.DEFAULT_OPTION, JOptionPane.PLAIN_MESSAGE, null,
						options, options[0]);
				switch (result) {
				case 0:
					_frame.setLogThreshold((String)panel.host_combo.getSelectedItem(),
							(SystemLogLevel)panel.level_combo.getSelectedItem());
					break;
				case 1:
				}
			}
	    });
	    config_menu.add(log_threshold);

	    JMenuItem clear_warning_error = new JMenuItem("Clear Warnings and Errors");
	    clear_warning_error.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				LogWarningPopup.clearAllPopups();
			}
	    });
	    command_menu.add(clear_warning_error);
	    JMenuItem version_item = new JMenuItem("Version");
	    version_item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				JOptionPane.showMessageDialog(_frame,
						"Belle2LogViewer version " + Belle2LogViewer.VERSION, "version", JOptionPane.INFORMATION_MESSAGE);
			}
	    });
	    help_menu.add(version_item);
	}
	

	private class LevelSelectPanel extends JPanel {

		private static final long serialVersionUID = 1L;
		public JComboBox host_combo;
		public JComboBox level_combo;

		public LevelSelectPanel() {
			setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
			add(Box.createRigidArea(new Dimension(0, 10)));

			Vector<String> host_v = new Vector<String>();
			for (String key : _frame.getPopupLevels().keySet()) {
				host_v.add(key);
			}
			host_combo = new JComboBox(host_v);
			host_combo.setPreferredSize(new Dimension(180, 24));
			JPanel label_panel = new JPanel();
			label_panel.setPreferredSize(new Dimension(180, 20));
			JLabel label = new JLabel("Select process : ");
			label.setHorizontalAlignment(JLabel.LEFT);
			label_panel.add(label);
			add(label_panel);
			add(Box.createRigidArea(new Dimension(0, 5)));
			add(host_combo);
			add(Box.createRigidArea(new Dimension(0, 10)));
			level_combo = new JComboBox(SystemLogLevel.LogLevelList);
			level_combo.setPreferredSize(new Dimension(180, 24));
			label_panel = new JPanel();
			label_panel.setPreferredSize(new Dimension(180, 20));
			label = new JLabel("Select priority : ");
			label.setHorizontalAlignment(JLabel.LEFT);
			label_panel.add(label);
			add(label_panel);
			add(Box.createRigidArea(new Dimension(0, 5)));
			add(level_combo);
			add(Box.createRigidArea(new Dimension(0, 10)));
		}
	}

}
