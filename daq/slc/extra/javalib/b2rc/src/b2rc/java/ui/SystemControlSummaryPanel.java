package b2rc.java.ui;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;

import javax.swing.JPanel;

import b2daq.ui.Updatable;
import b2rc.core.RCMaster;

public class SystemControlSummaryPanel extends JPanel implements Updatable {

	private static final long serialVersionUID = 1L;

	private SystemConfigurationPanel _sys_config_panel;
	private SystemStatusPanel _sys_status_panel;
	private RCNodeGroupViewPanel _node_state_panel;
	private RCMaster _master;
	private ControlMainPanel _main_panel;

	public SystemControlSummaryPanel(ControlMainPanel main_panel, RCMaster master) {
		_main_panel = main_panel;
		_master = master;
		_sys_status_panel = new SystemStatusPanel(_master);
		_node_state_panel = new RCNodeGroupViewPanel(_master);

		GridBagLayout layout = new GridBagLayout();
		GridBagConstraints gbc = new GridBagConstraints();
		setLayout(layout);
		gbc.gridx = 0;
		gbc.gridy = 0;
		gbc.weightx = 0.4d;
		gbc.weighty = 1.0d;
		gbc.fill = GridBagConstraints.BOTH;
		gbc.insets = new Insets(5, 5, 5, 5);
		_sys_config_panel = new SystemConfigurationPanel(_main_panel, _master);
		layout.setConstraints(_sys_config_panel, gbc);
		add(_sys_config_panel);

		gbc.gridx = 1;
		gbc.gridy = 0;
		gbc.weightx = 0.8d;
		gbc.weighty = 1.0d;
		gbc.fill = GridBagConstraints.BOTH;
		gbc.insets = new Insets(5, 5, 5, 5);
		layout.setConstraints(_sys_status_panel, gbc);
		add(_sys_status_panel);

		_node_state_panel.init();
		gbc.gridx = 2;
		gbc.gridy = 0;
		gbc.weightx = 0.8d;
		gbc.weighty = 1.0d;
		gbc.fill = GridBagConstraints.BOTH;
		gbc.insets = new Insets(5, 5, 5, 5);
		layout.setConstraints(_node_state_panel, gbc);
		add(_node_state_panel);
	}

	public void update() {
		_sys_config_panel.update();
		_sys_status_panel.update();
		_node_state_panel.update();
	}

	public SystemConfigurationPanel getConfigurationPanel() {
		return _sys_config_panel;
	}
	public SystemStatusPanel getStatusPanel() {
		return _sys_status_panel;
	}
}
