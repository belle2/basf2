package b2rc.java.ui;

import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.util.ArrayList;

import javax.swing.JPanel;
import javax.swing.border.EtchedBorder;
import javax.swing.border.TitledBorder;

import b2daq.core.Log;
import b2daq.java.ui.DnDTabbedPane;
import b2daq.java.ui.LogViewPanel;
import b2daq.ui.Updatable;
import b2rc.core.RCNodeSystem;

public class EditorMainPanel extends JPanel implements Updatable {

	private static final long serialVersionUID = 1L;
	private RCNodeSystem _system;
	private ArrayList<ParameterTablePanel> _panel_v = new ArrayList<ParameterTablePanel>();
	private VersionControlPanel _version_panel;
	private LogViewPanel _log_panel = new LogViewPanel();
	private DnDTabbedPane _tab_panel = new DnDTabbedPane();
	
	public EditorMainPanel(RCNodeSystem system) {
		GridBagLayout layout = new GridBagLayout();
		GridBagConstraints gbc = new GridBagConstraints();
		setLayout(layout);

		setGrid(gbc, 0, 0, 0.2d, 0.6d, GridBagConstraints.BOTH,
				GridBagConstraints.WEST, new Insets(5, 5, 5, 5));
		layout.setConstraints(_tab_panel, gbc);
		add(_tab_panel);

		setGrid(gbc, 0, 1, 0.2d, 0.5d, GridBagConstraints.BOTH,
				GridBagConstraints.WEST, new Insets(5, 5, 5, 5));
		JPanel panel = new JPanel();
		panel.setBorder(new TitledBorder(new EtchedBorder(), "Editor Logs",
				TitledBorder.LEFT, TitledBorder.TOP));
		panel.setPreferredSize(new Dimension(1000, 130));
		layout.setConstraints(panel, gbc);
		add(panel);
		
		GridBagLayout layout2 = new GridBagLayout();
		GridBagConstraints gbc2 = new GridBagConstraints();
		panel.setLayout(layout2);
		setGrid(gbc2, 0, 0, 1, 1, GridBagConstraints.BOTH,
				GridBagConstraints.WEST, new Insets(0, 0, 0, 0));
		layout2.setConstraints(_log_panel, gbc2);
		panel.add(_log_panel);
		
		_system = system;
		_version_panel = new VersionControlPanel(this, _system, _panel_v);
		_tab_panel.addTab("Version Control", _version_panel);
		COPPERParameterTablePanel copper_panel = new COPPERParameterTablePanel(this, _version_panel, _system);
		TTDParameterTablePanel ttd_panel = new TTDParameterTablePanel(this, _version_panel, _system);
		HSLBParameterTablePanel hslb_panel = new HSLBParameterTablePanel(this, _version_panel, _system, copper_panel);
		FTSWParameterTablePanel ftsw_panel = new FTSWParameterTablePanel(this, _version_panel, _system, ttd_panel);
		addPanel(copper_panel);
		addPanel(ttd_panel);
		addPanel(new ROParameterTablePanel(this, _version_panel, _system));
		addPanel(hslb_panel);
		addPanel(ftsw_panel);
		for (String label : _system.getModuleLists().keySet()) {
			addPanel(new FEEModuleParameterTablePanel(this, _version_panel, system, label));
		}
	}
	
	public void init() {
		for ( ParameterTablePanel p_panel : _panel_v ) {
			p_panel.init();
		}
		_version_panel.init();
	}
	
	public void update() {
		for ( ParameterTablePanel panel : _panel_v ) {
			panel.update();
		}
	}

	private void addPanel(ParameterTablePanel panel) {
		_tab_panel.addTab(panel.getAlias(), panel);
		_panel_v.add(panel);
	}
	
	private void setGrid(GridBagConstraints gbc, int gridwidth, int gridheight,
			int gridx, int gridy, double weightx, double weighty, int fill,
			int anchor, Insets insets) {
		gbc.gridwidth = gridwidth;
		gbc.gridheight = gridheight;
		gbc.gridx = gridx;
		gbc.gridy = gridy;
		gbc.weightx = weightx;
		gbc.weighty = weighty;
		gbc.fill = fill;
		// gbc.anchor = GridBagConstraints.LINE_START;
		gbc.anchor = anchor;
		gbc.insets = insets;
	}

	private void setGrid(GridBagConstraints gbc, int gridx, int gridy,
			double weightx, double weighty, int fill, int anchor, Insets insets) {
		setGrid(gbc, 1, 1, gridx, gridy, weightx, weighty, fill, anchor, insets);
	}

	public void addLog(Log log) {
		_log_panel.add(log);
	}


}
