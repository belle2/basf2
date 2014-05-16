package b2daq.dqm.ui;

import java.awt.Dimension;
import java.awt.Font;
import java.text.SimpleDateFormat;
import java.util.Date;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JPanel;

import b2daq.ui.Updatable;

public class DQMStatusPanel extends JPanel implements Updatable {

	private static final long serialVersionUID = 1L;
	private JLabel _runno_label;
	private DQMStateLabel _state_label;
	private JLabel _date_label;
	private SimpleDateFormat _data_format = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");  
	
	public DQMStatusPanel() {
		JPanel panel = new JPanel();
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		add(Box.createRigidArea(new Dimension(10,10)));
		add(panel);
		add(Box.createRigidArea(new Dimension(10,10)));
		panel.setLayout(new BoxLayout(panel, BoxLayout.X_AXIS));
		_state_label = new DQMStateLabel();
		_runno_label = new JLabel("run # : 0000.000000");
		_runno_label.setFont(new Font("Sans", Font.PLAIN, 15));
		JLabel label = new JLabel("DQM state : ");
		label.setFont(new Font("Sans", Font.PLAIN, 15));
		_date_label = new JLabel("updated at : ----/--/-- --:--:--");
		_date_label.setFont(new Font("Sans", Font.PLAIN, 15));
		panel.add(Box.createRigidArea(new Dimension(10,10)));
		panel.add(label);
		panel.add(Box.createRigidArea(new Dimension(10,10)));
		panel.add(_state_label);
		panel.add(Box.createRigidArea(new Dimension(20,10)));
		panel.add(_runno_label);
		panel.add(Box.createRigidArea(new Dimension(20,10)));
		panel.add(_date_label);
		panel.add(Box.createGlue());
	}
	
	@Override
	public void update() {
		_state_label.update();
	}
	
	public void update(int expno, int runno, int stateno) {
		_state_label.setState(stateno);
		_runno_label.setText("run # : "+String.format("%04d", expno)+
				"." + String.format("%06d", runno));
		_date_label.setText("updated at : "+ _data_format.format(new Date()));
		update();
	}
	
}
