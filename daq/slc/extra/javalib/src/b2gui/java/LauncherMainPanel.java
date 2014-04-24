package b2gui.java;

import java.awt.Dimension;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JPanel;

public class LauncherMainPanel extends JPanel {

	private static final long serialVersionUID = 1L;

	private LauncherPanel [] _panel_v = {
			new LauncherPanel("Run controller", "belle-rpc1", 60000, "b2rc.java.Belle2RunController", 
					b2rc.java.Belle2RunController.VERSION),
			new LauncherPanel("DQM browser", "belle-rpc2", 50100, "b2dqm.java.Belle2DQMBrowser", 
					b2dqm.java.Belle2DQMBrowser.VERSION)/*,
			new LauncherPanel("Log viewer", "belle-rpc2", 50200, "b2logger.java.Belle2LogViewer", 
					b2logger.java.Belle2LogViewer.VERSION)*/
	};
	
	public LauncherMainPanel() {
		setPreferredSize(new Dimension(200, 40 * _panel_v.length + 30));
		setMaximumSize(new Dimension(200, 40 * _panel_v.length + 30));
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		add(Box.createRigidArea(new Dimension(10, 10)));
		for ( LauncherPanel panel : _panel_v ) {
			add(panel);
			add(Box.createRigidArea(new Dimension(10, 5)));
		}
		add(Box.createRigidArea(new Dimension(10, 10)));
	}
	
	public LauncherPanel [] getPanels() {
		return _panel_v;
	}
	
	public void setEnableButtons(boolean enabled) {
		for ( LauncherPanel panel : _panel_v ) {
			panel.setEnabled(enabled);
		}
	}

}
