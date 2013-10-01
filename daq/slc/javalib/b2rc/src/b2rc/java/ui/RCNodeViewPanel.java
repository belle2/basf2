package b2rc.java.ui;

import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.util.ArrayList;

import javax.swing.JLabel;
import javax.swing.JMenu;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.SwingUtilities;

import b2daq.ui.Updatable;
import b2rc.core.FTSW;
import b2rc.core.RCCommand;
import b2rc.core.RCNode;
import b2rc.core.RCState;
import b2rc.core.TTDNode;
import b2rc.java.io.RCServerCommunicator;
import b2rc.java.io.RunControlMessage;

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

		addMouseListener(new MouseListener() {
			@Override
			public void mouseClicked(MouseEvent e) {
				if (SwingUtilities.isRightMouseButton(e)) {
					JPopupMenu popup_menu = new JPopupMenu();
					ArrayList<RCCommand> command_v = new ArrayList<RCCommand>();
					if (_node.getState().equals(RCState.INITIAL_S)) {
						command_v.add(RCCommand.BOOT);
					} else if (_node.getState().equals(RCState.CONFIGURED_S)) {
						command_v.add(RCCommand.BOOT);
						command_v.add(RCCommand.LOAD);
					} else if (_node.getState().equals(RCState.READY_S)) {
						command_v.add(RCCommand.BOOT);
						command_v.add(RCCommand.LOAD);
						command_v.add(RCCommand.START);
						if ( _node instanceof TTDNode ) {
							command_v.add(RCCommand.TRGIFT);
						}
					} else if (_node.getState().equals(RCState.RUNNING_S)) {
						command_v.add(RCCommand.STOP);
						command_v.add(RCCommand.PAUSE);
						command_v.add(RCCommand.ABORT);
					} else if (_node.getState().equals(RCState.PAUSED_S)) {
						command_v.add(RCCommand.STOP);
						command_v.add(RCCommand.RESUME);
						command_v.add(RCCommand.ABORT);
					}
					JMenu menu = new JMenu("Command");
					popup_menu.add(menu);
					for (RCCommand command : command_v) {
						if ( !command.equal(RCCommand.TRGIFT) ) {
							JMenuItem item = new JMenuItem(command.getAlias());
							item.addActionListener(new CommandMenuItemListener(command));
							menu.add(item);
						} else {
							JMenu submenu = new JMenu(command.getAlias());
							JMenuItem item = new JMenuItem("NONE");
							item.addActionListener(new TrgFTCommandMenuItemListener(FTSW.TRIG_NORMAL));
							submenu.add(item);
							item = new JMenuItem("IN");
							item.addActionListener(new TrgFTCommandMenuItemListener(FTSW.TRIG_IN));
							submenu.add(item);
							item = new JMenuItem("PULSE");
							item.addActionListener(new TrgFTCommandMenuItemListener(FTSW.TRIG_PULSE));
							submenu.add(item);
							item = new JMenuItem("REVO");
							item.addActionListener(new TrgFTCommandMenuItemListener(FTSW.TRIG_REVO));
							submenu.add(item);
							item = new JMenuItem("RANDOM");
							item.addActionListener(new TrgFTCommandMenuItemListener(FTSW.TRIG_RANDOM));
							submenu.add(item);
							item = new JMenuItem("POSSION");
							item.addActionListener(new TrgFTCommandMenuItemListener(FTSW.TRIG_POSSION));
							submenu.add(item);
							item = new JMenuItem("ONCE");
							item.addActionListener(new TrgFTCommandMenuItemListener(FTSW.TRIG_ONCE));
							submenu.add(item);
							item = new JMenuItem("STOP");
							item.addActionListener(new TrgFTCommandMenuItemListener(FTSW.TRIG_STOP));
							submenu.add(item);
							menu.add(submenu);
						}
					}
					popup_menu.show(e.getComponent(), e.getX(), e.getY());
				}
			}

			@Override
			public void mousePressed(MouseEvent e) {
			}

			@Override
			public void mouseReleased(MouseEvent e) {
			}

			@Override
			public void mouseEntered(MouseEvent e) {
			}

			@Override
			public void mouseExited(MouseEvent e) {
			}

		});
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

	private class CommandMenuItemListener implements ActionListener {
		private RCCommand _command;

		public CommandMenuItemListener(RCCommand command) {
			_command = command;
		}

		public void actionPerformed(ActionEvent arg0) {
			int [] value_v = new int[1];
			value_v[0] = _node.getIndex();
			try {
				RCServerCommunicator.get().sendMessage(new RunControlMessage(_command, value_v));
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

	private class TrgFTCommandMenuItemListener implements ActionListener {
		private RCCommand _command = RCCommand.TRGIFT;
		private int _param = 0;
		
		public TrgFTCommandMenuItemListener(int param) {
			_param = param;
		}

		public void actionPerformed(ActionEvent arg0) {
			int [] value_v = new int[7];
			value_v[0] = _node.getIndex();
			value_v[1] = _param;
			value_v[2] = 0;
			value_v[3] = 0;
			value_v[4] = 0;
			value_v[5] = 0;
			value_v[6] = 0;
			try {
				RCServerCommunicator.get().sendMessage(new RunControlMessage(_command, value_v));
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}

}
