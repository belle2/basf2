package b2rc.java.ui;

import java.awt.Dimension;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JPanel;
import javax.swing.border.EtchedBorder;
import javax.swing.border.TitledBorder;

import b2daq.core.Log;
import b2daq.core.LogLevel;
import b2daq.ui.Updatable;
import b2rc.core.RCCommand;
import b2rc.core.RCMaster;
import b2rc.core.RCNode;
import b2rc.core.RCState;
import b2rc.java.io.RCServerCommunicator;
import b2rc.java.io.RunControlMessage;

public class SystemButtonPanel extends JPanel implements Updatable {

	private static final long serialVersionUID = 1L;

	private RCMaster _master;
	private JButton _button_boot;
	private JButton _button_load;
	private JButton _button_start;
	private JButton _button_recover;
	private RCCommand _boot_command = new RCCommand(RCCommand.BOOT);
	private RCCommand _load_command = new RCCommand(RCCommand.LOAD);
	private RCCommand _start_command = new RCCommand(RCCommand.START);
	private RCCommand _recover_command = new RCCommand(RCCommand.ABORT);

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

	private class CommandButtonListener implements ActionListener {
		private RCCommand _command;
		public CommandButtonListener(RCCommand command) {
			_command = command;
		}
		public void actionPerformed(ActionEvent arg0) {
			RCServerCommunicator com = RCServerCommunicator.get();
			try {
				String data = "ALL";
				if ( _command.equals(RCCommand.BOOT) ||
					 _command.equals(RCCommand.LOAD) ) {
					int [] pars = new int [1];
					pars[0] = _master.getConfig().getConfigNumber();
					com.sendMessage(new RunControlMessage(_command, pars, data));
				} else {
					int [] pars = new int [2];
					pars[0] = -1;
					pars[1] = -1;
					if (_command.equals(RCCommand.START) ) {
						data +=  " " + _master.getStatus().getOperators();
					}
					com.sendMessage(new RunControlMessage(_command, pars, data));
				}
				com.getControlPanel().addLog(new Log("Pushed command: "+ _command.getAlias(), LogLevel.INFO));
			} catch (Exception e) {
				com.getControlPanel().addLog(new Log("Failed to send command", LogLevel.ERROR));
				e.printStackTrace();
			}
		}
	}
	
	public SystemButtonPanel(RCMaster master) {
		_master = master;
		setBorder(new TitledBorder(new EtchedBorder(), "System configuration",
				TitledBorder.LEFT, TitledBorder.TOP));
		GridBagLayout layout = new GridBagLayout();
		GridBagConstraints gbc = new GridBagConstraints();
		setLayout(layout);

		setGrid(gbc, 0, 0, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(5, 5, 5, 5));
		_button_boot = new JButton("");
		_button_boot.setFont(new Font("Sans", Font.PLAIN, 13));
		_button_boot.setPreferredSize(new Dimension(100, 50));
		_button_boot.addActionListener(new CommandButtonListener(_boot_command));
		layout.setConstraints(_button_boot, gbc);
		add(_button_boot);

		setGrid(gbc, 0, 1, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(5, 5, 5, 5));
		_button_load = new JButton("");
		_button_load.setPreferredSize(new Dimension(100, 50));
		_button_load.setFont(new Font("Sans", Font.PLAIN, 13));
		_button_load.addActionListener(new CommandButtonListener(_load_command));
		layout.setConstraints(_button_load, gbc);
		add(_button_load);

		setGrid(gbc, 1, 0, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(5, 5, 5, 5));
		_button_start = new JButton("");
		_button_start.setPreferredSize(new Dimension(100, 50));
		_button_start.setFont(new Font("Sans", Font.PLAIN, 13));
		_button_start.addActionListener(new CommandButtonListener(_start_command));
		layout.setConstraints(_button_start, gbc);
		add(_button_start);

		setGrid(gbc, 1, 1, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(5, 5, 5, 5));
		_button_recover = new JButton("");
		_button_recover.setPreferredSize(new Dimension(100, 50));
		_button_recover.setFont(new Font("Sans", Font.PLAIN, 13));
		_button_recover.addActionListener(new CommandButtonListener(_recover_command));
		layout.setConstraints(_button_recover, gbc);
		add(_button_recover);
		
		update();
	}

	public void update() {
		RCNode node = _master.getNode();
		RCState state = node.getState();
		if ( state.equals(RCState.INITIAL_S) ) {
			_boot_command.copy(RCCommand.BOOT);
			_load_command.copy(RCCommand.LOAD);
			_start_command.copy(RCCommand.START);
		} else if ( state.equals(RCState.CONFIGURED_S) ){
			_boot_command.copy(RCCommand.BOOT);
			_load_command.copy(RCCommand.LOAD);
			_start_command.copy(RCCommand.START);
			_recover_command.copy(RCCommand.ABORT);
		} else if ( state.equals(RCState.READY_S) ){
			_boot_command.copy(RCCommand.BOOT);
			_load_command.copy(RCCommand.LOAD);
			_start_command.copy(RCCommand.START);
			_recover_command.copy(RCCommand.ABORT);
		} else if ( state.equals(RCState.RUNNING_S) ){
			_boot_command.copy(RCCommand.BOOT);
			_load_command.copy(RCCommand.LOAD);
			_start_command.copy(RCCommand.STOP);
			_recover_command.copy(RCCommand.ABORT);
		} else if ( state.equals(RCState.FATAL_ES) ){
			_boot_command.copy(RCCommand.BOOT);
			_load_command.copy(RCCommand.LOAD);
			_start_command.copy(RCCommand.STOP);
			_recover_command.copy(RCCommand.ABORT);
		}
		_button_boot.setText(_boot_command.getAlias());
		_button_load.setText(_load_command.getAlias());
		_button_start.setText(_start_command.getAlias());
		_button_recover.setText(_recover_command.getAlias());
		_button_boot.setEnabled(_master.getConfig().getConfigNumber() >= 0 && 
				_boot_command.available(state)>=RCCommand.ENABLED);
		_button_load.setEnabled(_master.getConfig().getConfigNumber() >= 0 && 
					_load_command.available(state)>=RCCommand.ENABLED);
		_button_start.setEnabled((_master.getConfig().getConfigNumber() >= 0 || 
					_start_command.equals(RCCommand.STOP) ) && 
					_start_command.available(state)>=RCCommand.ENABLED);
		_button_recover.setEnabled(_recover_command.available(state)>=RCCommand.ENABLED);
	}

}
