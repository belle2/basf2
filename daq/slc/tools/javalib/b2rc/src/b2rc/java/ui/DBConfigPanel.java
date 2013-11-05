package b2rc.java.ui;

import java.awt.Component;
import java.awt.Dimension;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.UIManager;

import b2daq.java.ui.PasswordBoxPanel;
import b2daq.java.ui.TextBoxPanel;
import b2rc.db.RCDBManager;

public class DBConfigPanel extends JPanel {
	
	private static final long serialVersionUID = 1L;

	private TextBoxPanel _host_name_panel;
	private TextBoxPanel _database_panel;
	private TextBoxPanel _account_panel;
	private PasswordBoxPanel _password_panel;
	private TextBoxPanel _host_port_panel;
	
	public DBConfigPanel(String hostname, String database, 
			String username, String password, int port) {
		super();
		_host_name_panel = new TextBoxPanel("Host name : ", hostname);
		_database_panel = new TextBoxPanel("Database : ", database);
		_account_panel = new TextBoxPanel("Account : ", username);
		_password_panel = new PasswordBoxPanel("Password :", password);
		_host_port_panel = new TextBoxPanel("Port : ", ""+port);
		setSize(300, 300);
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		JPanel title_panel = new JPanel();
		JLabel title_label = new JLabel("Database config");
		title_label.setHorizontalAlignment(JLabel.CENTER);
		title_panel.setLayout(new BoxLayout(title_panel, BoxLayout.X_AXIS));
		title_panel.add(title_label);
		add(title_panel);
		add(Box.createRigidArea(new Dimension(0,20)));
		add(_host_name_panel);
		add(Box.createRigidArea(new Dimension(0,5)));
		add(_database_panel);
		add(Box.createRigidArea(new Dimension(0,5)));
		add(_account_panel);
		add(Box.createRigidArea(new Dimension(0,5)));
		add(_password_panel);
		add(Box.createRigidArea(new Dimension(0,5)));
		add(_host_port_panel);
		add(Box.createRigidArea(new Dimension(0,20)));
	}
	
	public static boolean showLoginDialog(Component window, String hostname, String database, 
			String username, String password, int port) {
		try {
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		} catch (Exception e) {}

		DBConfigPanel login = new DBConfigPanel(hostname, database, username, password, port);
		Object[] message = { login };
		String[] options = { "Start" };

		int result = JOptionPane.showOptionDialog(window,
				message,
				"Database configuration", 
				JOptionPane.DEFAULT_OPTION,
				JOptionPane.PLAIN_MESSAGE,
				null,
				options,
				options[0]);
		switch(result) {
		   case 0:
			   try {
				   int port_in = port;
				   try {
					   port_in = Integer.parseInt(login._host_port_panel.getText());
				   } catch (Exception e) {
					   port_in = port;
				   }
				   RCDBManager.open(login._host_name_panel.getText(), 
						   login._database_panel.getText(), 
						   login._account_panel.getText(), 
						   login._password_panel.getText(), 
						   port_in);
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
