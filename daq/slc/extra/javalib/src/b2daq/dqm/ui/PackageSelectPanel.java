package b2daq.dqm.ui;

import java.awt.Dimension;
import java.util.ArrayList;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JCheckBox;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;

import b2daq.dqm.core.HistoPackage;


public class PackageSelectPanel {

	static public ArrayList<Boolean> showPane(ArrayList<HistoPackage> package_v,
													boolean configured) {
		ArrayList<JCheckBox> check_box_v = new ArrayList<JCheckBox>();
		ArrayList<Boolean> monitored_v = new ArrayList<Boolean>();
		JPanel panel = new JPanel();
		panel.setSize(240, 30*package_v.size()+50);
		panel.setLayout(new BoxLayout(panel, BoxLayout.Y_AXIS));
		JPanel title_panel = new JPanel();
		JLabel title_label = new JLabel("DQM package selection");
		if (configured) title_label.setText("New Packages are available");
		title_label.setHorizontalAlignment(JLabel.CENTER);
		title_panel.setLayout(new BoxLayout(title_panel, BoxLayout.X_AXIS));
		title_panel.add(title_label);
		panel.add(title_panel);
		if (package_v.size() > 0) {
			for ( HistoPackage pack : package_v ) {
				JCheckBox check_box = new JCheckBox(pack.getName());
				check_box.setSelected(true);
				panel.add(Box.createRigidArea(new Dimension(10, 5)));
				panel.add(check_box);
				check_box_v.add(check_box);
			}
			panel.add(Box.createRigidArea(new Dimension(10, 5)));
			Object[] message = { panel };
			String[] options = { "OK", "Cancel" };
			int result = JOptionPane.showOptionDialog(null, message, "Select DQM Packages",
					JOptionPane.DEFAULT_OPTION, JOptionPane.PLAIN_MESSAGE, null,
					options, options[0]);
			switch (result) {
			case 0:
				for ( int n = 0; n < check_box_v.size(); n++ ) {
					monitored_v.add(check_box_v.get(n).isSelected());
				}
				break;
			default:
				System.exit(0);	
			}
		} else {
			JOptionPane.showMessageDialog(null, "No package is available. Terminating GUI");
			System.exit(0);	
		}
		return monitored_v;
	}
}
