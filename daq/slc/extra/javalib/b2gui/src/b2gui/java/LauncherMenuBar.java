package b2gui.java;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;

public class LauncherMenuBar extends JMenuBar {

	private static final long serialVersionUID = 1L;

	public LauncherMenuBar() {
	    JMenu config_menu = new JMenu("Config");
	    JMenu show_menu = new JMenu("Show");
	    JMenu help_menu = new JMenu("Help");

	    add(config_menu);
	    add(show_menu);
	    add(help_menu);

	    JMenuItem log_item = new JMenuItem("Log");

	    log_item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				if ( LauncherFrame.get().getLog().isVisible() ) {
					LauncherFrame.get().getLog().setVisible(false);
					LauncherFrame.get().setSize(220, LauncherFrame.get().getHeight());
				} else {
					LauncherFrame.get().getLog().setVisible(true);
					LauncherFrame.get().setSize(680, LauncherFrame.get().getHeight());
				}
			}
	    });
	    show_menu.add(log_item);

	    JMenuItem open_item = new JMenuItem("Open");

	    open_item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				LauncherLoginPane.showPane();
			}
	    });

	    config_menu.add(open_item);

	    JMenuItem version_item = new JMenuItem("Version");
	    version_item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				JOptionPane.showMessageDialog(LauncherFrame.get(),
						"DCGUILauncher version " + Belle2GUILauncherJavaGUI.VERSION, "version", JOptionPane.INFORMATION_MESSAGE);
			}
	    });
	    help_menu.add(version_item);

	}
	
}
