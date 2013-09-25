package b2dqm.java.ui;

import java.awt.Font;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;

import b2dqm.java.Belle2DQMBrowser;


public class DQMMenuBar extends JMenuBar {

	private static final long serialVersionUID = 1L;
	private DQMMainFrame _frame;
	private JMenuItem _side_bar;
	private JMenuItem _show_main_panel;

	public DQMMenuBar(DQMMainFrame frame) {
		_frame = frame;
		
		JMenu file_menu = new JMenu("File");
	    JMenu show_menu = new JMenu("Show");
	    JMenu config_menu = new JMenu("Config");
	    JMenu help_menu = new JMenu("Help");
        file_menu.setFont(new Font("Sans", Font.PLAIN, 13));
        show_menu.setFont(new Font("Sans", Font.PLAIN, 13));
        config_menu.setFont(new Font("Sans", Font.PLAIN, 13));
        help_menu.setFont(new Font("Sans", Font.PLAIN, 13));

	    add(file_menu);
	    add(show_menu);
	    add(config_menu);
	    add(help_menu);

	    JMenuItem menuitem1 = new JMenuItem("New");
	    JMenuItem menuitem2 = new JMenuItem("Open");
	    JMenuItem menuitem3 = new JMenuItem("Close");
        menuitem1.setFont(new Font("Sans", Font.PLAIN, 12));
        menuitem2.setFont(new Font("Sans", Font.PLAIN, 12));
        menuitem3.setFont(new Font("Sans", Font.PLAIN, 12));

	    file_menu.add(menuitem1);
	    file_menu.add(menuitem2);
	    file_menu.add(menuitem3);

	    _side_bar = new JMenuItem("Close side panel");
       _side_bar.setFont(new Font("Sans", Font.PLAIN, 12));
	    _side_bar.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				if ( _frame.openSidePanel() ) {
					_side_bar.setText("Close side panel"); 
				} else {
					_side_bar.setText("Open side panel"); 
				}
			}
	    });
	    show_menu.add(_side_bar);
	    _show_main_panel = new JMenuItem("Separate monitor panels");
        _show_main_panel.setFont(new Font("Sans", Font.PLAIN, 12));
	    _show_main_panel.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				if ( _frame.separatePanels() ) {
					_show_main_panel.setText("Collect monitor panels"); 
				} else {
					_show_main_panel.setText("Separate monitor panels"); 
				}
			}
	    });
	    show_menu.add(_show_main_panel);
	    JMenuItem version_item = new JMenuItem("Version");
	    version_item.setFont(new Font("Sans", Font.PLAIN, 12));
	    version_item.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent arg0) {
				JOptionPane.showMessageDialog(_frame,
						"Belle-II DQM browser version " + Belle2DQMBrowser.VERSION, "version", JOptionPane.INFORMATION_MESSAGE);
			}
	    });
	    help_menu.add(version_item);
	}
	
}
