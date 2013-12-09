package b2rc.java.ui;

import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;

import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.border.EtchedBorder;
import javax.swing.border.TitledBorder;

import b2daq.core.Time;
import b2daq.ui.Updatable;
import b2rc.core.DataObject;
import b2rc.core.RCMaster;
import b2rc.core.RCNode;
import b2rc.core.RCState;

public class SystemStatusPanel extends JPanel implements Updatable {

	private static final long serialVersionUID = 1L;
	
	private RCNodeStateLabel _daq_state_label;
	private RCMaster _master;
	private JLabel _text_exp_no;
	private JLabel _text_run_no;
	private JLabel _text_start_time;
	private JLabel _text_end_time;

	private JLabel _text_trigger_mode;

	private JLabel _text_dummy_rate;

	private JLabel _text_trigger_limit;
	
	public SystemStatusPanel(RCMaster master) {
		_master = master;
		_daq_state_label = new RCNodeStateLabel(_master.getNode());
		setBorder(new TitledBorder(new EtchedBorder(), "System status",
				TitledBorder.LEFT, TitledBorder.TOP));
		GridBagLayout layout = new GridBagLayout();
		GridBagConstraints gbc = new GridBagConstraints();
		setLayout(layout);
		
		int rows = 0;
		setGrid(gbc, 0, rows, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		JLabel label_operation_mode = new JLabel("DAQ status:");
		label_operation_mode.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(label_operation_mode, gbc);
		add(label_operation_mode);

		setGrid(gbc, 1, rows++, 0.2d, 0.1d, GridBagConstraints.NONE,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		layout.setConstraints(_daq_state_label, gbc);
		add(_daq_state_label);

		setGrid(gbc, 0, rows, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		JLabel label_exp_no = new JLabel("Experiment #:");
		label_exp_no.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(label_exp_no, gbc);
		add(label_exp_no);

		setGrid(gbc, 1, rows++, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		_text_exp_no = new JLabel("--");
		_text_exp_no.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(_text_exp_no, gbc);
		add(_text_exp_no);

		setGrid(gbc, 0, rows, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		JLabel label_run_no = new JLabel("Run #:");
		label_run_no.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(label_run_no, gbc);
		add(label_run_no);

		setGrid(gbc, 1, rows++, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		_text_run_no = new JLabel("--");
		layout.setConstraints(_text_run_no, gbc);
		add(_text_run_no);

		setGrid(gbc, 0, rows, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		JLabel label_trigger_mode = new JLabel("Trigger mode:");
		label_trigger_mode.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(label_trigger_mode, gbc);
		add(label_trigger_mode);

		setGrid(gbc, 1, rows++, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		_text_trigger_mode = new JLabel("----");
		_text_trigger_mode.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(_text_trigger_mode, gbc);
		add(_text_trigger_mode);

		setGrid(gbc, 0, rows, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		JLabel label_dummy_rate = new JLabel("Dummy rate:");
		label_dummy_rate.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(label_dummy_rate, gbc);
		add(label_dummy_rate);

		setGrid(gbc, 1, rows++, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		_text_dummy_rate = new JLabel("----");
		_text_dummy_rate.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(_text_dummy_rate, gbc);
		add(_text_dummy_rate);

		setGrid(gbc, 0, rows, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		JLabel label_trigger_limit = new JLabel("Trigger limit:");
		label_trigger_limit.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(label_trigger_limit, gbc);
		add(label_trigger_limit);

		setGrid(gbc, 1, rows++, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		_text_trigger_limit = new JLabel("----");
		_text_trigger_limit.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(_text_trigger_limit, gbc);
		add(_text_trigger_limit);

		setGrid(gbc, 0, rows, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		JLabel label_start_time = new JLabel("Start time:");
		label_start_time.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(label_start_time, gbc);
		add(label_start_time);

		setGrid(gbc, 1, rows++, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		_text_start_time = new JLabel("2013/09/11 10:30");
		_text_start_time.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(_text_start_time, gbc);
		add(_text_start_time);

		setGrid(gbc, 0, rows, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		JLabel label_end_time = new JLabel("End time:");
		label_end_time.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(label_end_time, gbc);
		add(label_end_time);

		setGrid(gbc, 1, rows++, 0.2d, 0.1d, GridBagConstraints.BOTH,
				GridBagConstraints.LINE_START, new Insets(0, 5, 0, 5));
		_text_end_time = new JLabel("2013/09/11 10:30");
		_text_end_time.setFont(new Font("Sans", Font.PLAIN, 13));
		layout.setConstraints(_text_end_time, gbc);
		add(_text_end_time);

	}

	public void update() {
		_daq_state_label.update(_master.getNode());
		setExpNumber(_master.getStatus().getExpNumber());
		setRunNumber(_master.getStatus().getColdNumber());
		setStartTime(_master.getStatus().getStartTime());
		setEndTime(_master.getStatus().getEndTime());
		/*
		setTriggerType(_master.getTriggerMode());
		setDummyRate(_master.getDummyRate());
		setTriggerLimit(_master.getTriggerLimit());
		*/
		for ( RCNode node : _master.getNSMNodes() ) {
			DataObject data = node.getData();
			if ( data != null && data.getObject("ftsw") != null ) {
				DataObject ftsw = data.getObject("ftsw");
				setTriggerType(ftsw.getInt("trigger_mode"));
				setDummyRate(ftsw.getInt("dummy_rate"));
				setTriggerLimit(ftsw.getInt("trigger_limit"));
			}
		}
	}

	public void setTriggerType(int trigger_mode) {
		String [] type = new String [10]; 
		type[0] = "NONE";
		type[1] = "IN";
		type[2] = "TLU";
		type[3] = "--";
		type[4] = "PULSE";
		type[5] = "REVO";
		type[6] = "RANDOM";
		type[7] = "POSSION";
		type[8] = "ONCE";
		type[9] = "STOP";
		_text_trigger_mode.setText(type[trigger_mode]);
	}

	public void setDummyRate(int dummy_rate) {
		if ( dummy_rate < 0 ) {
			_text_dummy_rate.setText("no dummy trigger");
		} else if ( dummy_rate < 10000 ) {
			_text_dummy_rate.setText(""+(dummy_rate/1000.0)+"Hz");
		} else {
			_text_dummy_rate.setText(""+(dummy_rate/1000000.0)+"kHz");
		}
	}

	public void setTriggerLimit(int trigger_limit) {
		_text_trigger_limit.setText(""+trigger_limit);		
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

	public void setExpNumber(int no) {
		_text_exp_no.setText(""+no);
	}
	public void setRunNumber(int no) {
		_text_run_no.setText(""+no);
	}
	public void setStartTime(int time) {
		if ( time <= 0 ) {
			_text_start_time.setText("<html><span style='color:black;font-weight:bold;'>No data taking</span></html>");
		} else {
			_text_start_time.setText(new Time(time).toDateString());
		}
	}
	public void setEndTime(int time) {
		if ( !_master.getNode().getState().equals(RCState.RUNNING_S)) {
			_text_end_time.setText(new Time(time).toDateString());
		} else {
			if ( time <= 0 ) {
				_text_end_time.setText("<html><span style='color:black;font-weight:bold;'>No data taking</span></html>");
			} else {
				_text_end_time.setText("<html><span style='color:blue;font-weight:bold;'>Data taking is on going</span></html>");
			}
		}
	}
}
