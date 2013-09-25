package b2rc.java.ui;

import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;

import javax.swing.JLabel;
import javax.swing.JPanel;

import b2daq.ui.Updatable;
import b2rc.core.RCNode;

public class RCNodeViewPanel extends JPanel implements Updatable {

	private static final long serialVersionUID = 1L;
	private RCNode _node;
	private RCNodeStateLabel _state;
	
	public RCNodeViewPanel(RCNode node) {
		_node = node;
		_state = new RCNodeStateLabel(node);
	}

	public void init() {
		GridBagLayout layout = new GridBagLayout();
		GridBagConstraints gbc = new GridBagConstraints();
		setLayout(layout);

		setGrid(gbc, 0, 0, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		JLabel label_nodename = new JLabel(_node.getName());
		label_nodename.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(label_nodename, gbc);
		add(label_nodename);

		setGrid(gbc, 1, 0, 0.2d, 0.1d, GridBagConstraints.NONE,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		layout.setConstraints(_state, gbc);
		add(_state);
	}
	
	public void update() {
		_state.update(_node);
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
