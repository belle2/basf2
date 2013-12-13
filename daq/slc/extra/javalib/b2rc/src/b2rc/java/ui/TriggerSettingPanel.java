package b2rc.java.ui;

import java.awt.Component;
import java.awt.Dimension;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.UIManager;

import b2daq.core.Log;
import b2daq.core.LogLevel;
import b2daq.java.ui.ComboBoxPanel;
import b2daq.java.ui.TextBoxPanel;
import b2rc.core.RCCommand;
import b2rc.core.RCNode;
import b2rc.java.io.RCServerCommunicator;
import b2rc.java.io.RunControlMessage;

public class TriggerSettingPanel extends JPanel {
	
	private static final long serialVersionUID = 1L;

	private ComboBoxPanel _trigger_type;
	private TextBoxPanel _dummy_rate;
	private TextBoxPanel _trigger_limit;
	
	public TriggerSettingPanel(RCNode node) {
		super();
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
		_trigger_type = new ComboBoxPanel("Trigger type : ", type);
		_trigger_type.setFieldSize(80, 30);
		_dummy_rate = new TextBoxPanel("Dummy rate [Hz]: ", ""+(-1));
		_trigger_limit = new TextBoxPanel("Trigger limit : ", ""+0);
		_trigger_type.setLabelSize(120, 30);
		_dummy_rate.setLabelSize(120, 30);
		_trigger_limit.setLabelSize(120, 30);
		_trigger_type.setFieldSize(80, 30);
		_dummy_rate.setFieldSize(80, 30);
		_trigger_limit.setFieldSize(80, 30);
		setSize(300, 300);
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		JPanel title_panel = new JPanel();
		JLabel title_label = new JLabel(node.getName() + " Trigger setting");
		title_label.setHorizontalAlignment(JLabel.CENTER);
		title_panel.setLayout(new BoxLayout(title_panel, BoxLayout.X_AXIS));
		title_panel.add(title_label);
		add(title_panel);
		add(Box.createRigidArea(new Dimension(0,20)));
		add(_trigger_type);
		add(Box.createRigidArea(new Dimension(0,5)));
		add(_dummy_rate);
		add(Box.createRigidArea(new Dimension(0,5)));
		add(_trigger_limit);
		add(Box.createRigidArea(new Dimension(0,20)));
	}
	
	public static boolean showLoginDialog(Component window, RCNode node) {
		try {
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		} catch (Exception e) {}

		TriggerSettingPanel panel = new TriggerSettingPanel(node);
		Object[] message = { panel };
		String[] options = { "Submit", "Cancel" };

		int result = JOptionPane.showOptionDialog(window,
				message,
				node.getName() + " Trigger setting", 
				JOptionPane.DEFAULT_OPTION,
				JOptionPane.PLAIN_MESSAGE,
				null,
				options,
				options[0]);
		switch(result) {
		   case 0:
			   try {
				   int dummy_rate = -1;
				   try {
					   dummy_rate = (int)(1000*Double.parseDouble(panel._dummy_rate.getText()));
				   } catch (Exception e) {
					   dummy_rate = -1;
				   }
				   if (dummy_rate < 0) dummy_rate = -1;
				   int trigger_limit = 0;
				   try {
					   trigger_limit = Integer.parseInt(panel._trigger_limit.getText());
				   } catch (Exception e) {
					   trigger_limit = 0;
				   }
				   int [] value_v = new int[5];
					value_v[0] = panel._trigger_type.getSelected();
					value_v[1] = dummy_rate;
					value_v[2] = trigger_limit;
					value_v[3] = 0;
					value_v[4] = 0;
					String data = node.getName();
					if (node.getData().getObject("ftsw")!= null) {
						node.getData().getObject("ftsw").setInt("trigger_type", value_v[0]);
						node.getData().getObject("ftsw").setInt("dummy_rate", value_v[1]);
						node.getData().getObject("ftsw").setInt("trigger_limit", value_v[2]);
					}
					try{
						RCServerCommunicator.get().sendMessage(new RunControlMessage(RCCommand.TRGIFT, value_v, data));
						RCServerCommunicator.get().getControlPanel().addLog(new Log("Sent new trigger setting to "
											+ node.getName() + "<br/>"
											+ "Trigger type : <span style='color:blue;font-weight:bold;'>"
											+ panel._trigger_type.getText()+"</span><br/>"
											+ "Dummy rate : <span style='color:blue;font-weight:bold;'>"
											+ ((dummy_rate<0)?"none":(dummy_rate>1000000)?(dummy_rate/1000000.0+" kHz"):(dummy_rate/1000.0+" Hz"))+"</span><br/>" 
											+ "Trigger limit : <span style='color:blue;font-weight:bold;'>"
											+ trigger_limit+"</span>", LogLevel.INFO));

					} catch (Exception e) {
						e.printStackTrace();
					}
			   } catch (Exception e) {
					e.printStackTrace();
				   return false;
			   }
			   return true;
		   default:
			   return false;
		}
	}
	
}
