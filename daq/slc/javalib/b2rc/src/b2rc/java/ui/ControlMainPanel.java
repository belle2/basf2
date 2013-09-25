package b2rc.java.ui;

import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;

import javax.swing.BoxLayout;
import javax.swing.JPanel;
import javax.swing.border.EtchedBorder;
import javax.swing.border.TitledBorder;

import b2daq.core.Log;
import b2daq.java.ui.DnDTabbedPane;
import b2daq.java.ui.LogViewPanel;
import b2daq.ui.Updatable;
import b2rc.core.RCNodeSystem;

public class ControlMainPanel extends JPanel implements Updatable {

	private static final long serialVersionUID = 1L;
	private SystemControlSummaryPanel _sys_summary_panel;
	private RCNodeViewSummaryPanel _node_state_panel;
	private SystemButtonPanel _sys_button_panel;
	private LogViewPanel _log_view_panel = new LogViewPanel();
	private RCNodeSystem _system;

	public ControlMainPanel(EditorMainPanel editor_panel, RCNodeSystem system) {
		_system = system;
		_sys_summary_panel = new SystemControlSummaryPanel(this, editor_panel, _system);
		_node_state_panel = new RCNodeViewSummaryPanel(_system.getNodeGroups());
		_sys_button_panel = new SystemButtonPanel(system);
		_node_state_panel.init();
		
		GridBagLayout layout = new GridBagLayout();
		GridBagConstraints gbc = new GridBagConstraints();
		setLayout(layout);

		DnDTabbedPane tab_panel = new DnDTabbedPane();
		tab_panel.addTab("Summary", _sys_summary_panel);
		tab_panel.addTab("Nodes", _node_state_panel);
		
		gbc.gridwidth = 2;
		gbc.gridx = 0;
		gbc.gridy = 0;
		gbc.weightx = 0.2d;
		gbc.weighty = 1.0d;
		gbc.fill = GridBagConstraints.BOTH;
		gbc.insets = new Insets(5, 5, 5, 5);
		layout.setConstraints(tab_panel, gbc);
		add(tab_panel);

		gbc.gridwidth = 1;
		gbc.gridx = 0;
		gbc.gridy = 1;
		gbc.weightx = 0.8d;
		gbc.weighty = 0.6d;
		gbc.fill = GridBagConstraints.BOTH;
		gbc.insets = new Insets(5, 5, 5, 5);
		layout.setConstraints(_sys_button_panel, gbc);
		add(_sys_button_panel);

		gbc.gridx = 1;
		gbc.gridy = 1;
		gbc.weightx = 4.0d;
		gbc.weighty = 0.6d;
		gbc.fill = GridBagConstraints.BOTH;
		gbc.insets = new Insets(5, 5, 5, 5);
		JPanel _log0_panel = new JPanel();
		_log0_panel.setLayout(new BoxLayout(_log0_panel, BoxLayout.X_AXIS));
		JPanel log_panel = new JPanel();
		_log0_panel.add(log_panel);
		log_panel.setLayout(new BoxLayout(log_panel, BoxLayout.Y_AXIS));
		log_panel.setBorder(new TitledBorder(new EtchedBorder(),
				"Operation log", TitledBorder.LEFT, TitledBorder.TOP));
		_log_view_panel.setPreferredSize(new Dimension(Short.MAX_VALUE, Short.MAX_VALUE));
		_log_view_panel.setMaximumSize(new Dimension(Short.MAX_VALUE, Short.MAX_VALUE));
		log_panel.add(_log_view_panel);
		layout.setConstraints(_log0_panel, gbc);
		add(_log0_panel);
	}

	public SystemControlSummaryPanel getControlSummaryPanel() { return _sys_summary_panel; }
	public RCNodeViewSummaryPanel getNodeSummaryPanel() { return _node_state_panel; }
	public SystemButtonPanel getButtonPanel() { return _sys_button_panel; }
	public LogViewPanel getLogPanel() { return _log_view_panel; }

	public void update() {
		_sys_summary_panel.update();
		_node_state_panel.update();
		_sys_button_panel.update();
	}

	public void addLog(Log log) {
		_log_view_panel.add(log);
	}

}
