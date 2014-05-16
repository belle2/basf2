package b2daq.ui;

import java.awt.Component;
import java.awt.Dimension;

import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.UIManager;

public class LoginPanel extends JPanel {
	
	private static final long serialVersionUID = 1L;

	private TextBoxPanel _host_name_panel = new TextBoxPanel("Host name : ", "localhost");
	private TextBoxPanel _host_port_panel = new TextBoxPanel("Port : ", "0");
	private TextBoxPanel _account_panel = new TextBoxPanel("Account : ", "");

	public LoginPanel(String host, int port, String label, String title) {
		super();
		setSize(300, 300);
		setLayout(new BoxLayout(this, BoxLayout.Y_AXIS));
		JPanel title_panel = new JPanel();
		JLabel title_label = new JLabel(title);
		title_label.setHorizontalAlignment(JLabel.CENTER);
		title_panel.setLayout(new BoxLayout(title_panel, BoxLayout.X_AXIS));
		title_panel.add(title_label);
		_host_name_panel.setText(host);
		_host_port_panel.setText("" + port);
		add(title_panel);
		add(Box.createRigidArea(new Dimension(0,20)));
		add(_host_name_panel);
		add(Box.createRigidArea(new Dimension(0,5)));
		add(_host_port_panel);
		add(Box.createRigidArea(new Dimension(0,5)));
		add(_account_panel);
		add(Box.createRigidArea(new Dimension(0,20)));
	}
	
	public static boolean showLoginDialog(Component window, String host_name, int port , 
				String title, String comments, JavaEntoryPoint entry_point) {
		try {
			UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
		} catch (Exception e) {}

		LoginPanel login = new LoginPanel(host_name, port, title, comments);
		Object[] message = { login };
		String[] options = { "Login", "Cancel" };

		int result = JOptionPane.showOptionDialog(window,
				message,
				title, 
				JOptionPane.DEFAULT_OPTION,
				JOptionPane.PLAIN_MESSAGE,
				null,
				options,
				options[0]);
		switch(result) {
		   case 0:
			   entry_point.init(login._host_name_panel.getText(), 
					   Integer.parseInt(login._host_port_panel.getText()),
					   login._account_panel.getText());
			   entry_point.start();
			   return true;
		   default:
			   return false;
		}
	}
	
}
