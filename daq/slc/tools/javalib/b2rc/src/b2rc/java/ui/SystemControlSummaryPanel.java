package b2rc.java.ui;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;

import javax.swing.JPanel;

import b2daq.ui.Updatable;
import b2rc.core.RCNodeSystem;

public class SystemControlSummaryPanel extends JPanel implements Updatable {

	private static final long serialVersionUID = 1L;

	private SystemConfigurationPanel _sys_config_panel;
	private SystemStatusPanel _sys_status_panel;
	private RCNodeSystem _system;
	private ControlMainPanel _main_panel;
	private EditorMainPanel _editor_panel;

	public SystemControlSummaryPanel(ControlMainPanel main_panel, EditorMainPanel editor_panel, RCNodeSystem system) {
		_main_panel = main_panel;
		_system = system;
		_editor_panel = editor_panel;
		_sys_status_panel = new SystemStatusPanel(_system);
		GridBagLayout layout = new GridBagLayout();
		GridBagConstraints gbc = new GridBagConstraints();
		setLayout(layout);
		gbc.gridx = 0;
		gbc.gridy = 0;
		gbc.weightx = 0.4d;
		gbc.weighty = 1.0d;
		gbc.fill = GridBagConstraints.BOTH;
		gbc.insets = new Insets(5, 5, 5, 5);
		_sys_config_panel = new SystemConfigurationPanel(_main_panel, _editor_panel, _system);
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
	}

	public void update() {
		_sys_config_panel.update();
		_sys_status_panel.update();
	}

	public SystemConfigurationPanel getConfigurationPanel() {
		return _sys_config_panel;
	}
	public SystemStatusPanel getStatusPanel() {
		return _sys_status_panel;
	}
}
