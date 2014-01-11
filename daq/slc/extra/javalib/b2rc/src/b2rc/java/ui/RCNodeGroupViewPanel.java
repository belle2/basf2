package b2rc.java.ui;

import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.util.ArrayList;

import javax.swing.JPanel;
import javax.swing.border.EtchedBorder;
import javax.swing.border.TitledBorder;

import b2daq.ui.Updatable;
import b2rc.core.RCMaster;

public class RCNodeGroupViewPanel extends JPanel implements Updatable {

	private static final long serialVersionUID = 1L;
	private ArrayList<RCNodeViewPanel> _panel_v = new ArrayList<RCNodeViewPanel>();
	private RCMaster _master;

	public RCNodeGroupViewPanel(RCMaster master) {
		_master = master;
	}

	public ArrayList<RCNodeViewPanel> getpanels() {
		return _panel_v;
	}

	public void setGroup(RCMaster master) {
		_master = master;
	}

	public void init() {
		setBorder(new TitledBorder(new EtchedBorder(), "Node status",
				TitledBorder.LEFT, TitledBorder.TOP));
		GridBagLayout layout = new GridBagLayout();
		GridBagConstraints gbc = new GridBagConstraints();
		setLayout(layout);
		int i = 0;
		final int cols = 1; 
		final int rows = 6; 
		for (int icols = 0; icols < cols; icols++) {
			for (int irows = 0; irows < rows; irows++) {
				if (i >= _master.getNSMNodes().size()) return;
				RCNodeViewPanel panel = new RCNodeViewPanel(_master.getNSMNodes().get(i));
				panel.init();
				setGrid(gbc, icols, irows, 0.1d, 0.1d, GridBagConstraints.BOTH,
						GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
				layout.setConstraints(panel, gbc);
				add(panel);
				_panel_v.add(panel);
				i++;
			}
		}
	}

	public void update() {
		for (int i = 0; i < _panel_v.size(); i++) {
			_panel_v.get(i).update();
		}
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

}
